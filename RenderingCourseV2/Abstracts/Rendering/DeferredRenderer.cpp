#include "Abstracts/Rendering/DeferredRenderer.h"
#include <d3dcompiler.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>

#pragma comment(lib, "d3dcompiler.lib")

namespace
{
	constexpr int ShadowCascadeCount = 4;
	constexpr std::array<float, ShadowCascadeCount> ShadowCascadeSplitFactors =
	{
		0.08f,
		0.2f,
		0.45f,
		1.0f
	};

	DirectX::XMFLOAT3 NormalizeVector3(const DirectX::XMFLOAT3& InputVector)
	{
		const DirectX::XMVECTOR VectorValue = DirectX::XMLoadFloat3(&InputVector);
		DirectX::XMFLOAT3 OutputVector;
		DirectX::XMStoreFloat3(&OutputVector, DirectX::XMVector3Normalize(VectorValue));
		return OutputVector;
	}

	float GetShadowCascadeSplitFactor(int CascadeIndex, int ShadowCascadeCountSetting)
	{
		if (ShadowCascadeCountSetting <= 1)
		{
			return 1.0f;
		}

		if (ShadowCascadeCountSetting == 2)
		{
			return (CascadeIndex == 0) ? 0.25f : 1.0f;
		}

		if (ShadowCascadeCountSetting == 3)
		{
			if (CascadeIndex == 0)
			{
				return 0.15f;
			}
			if (CascadeIndex == 1)
			{
				return 0.4f;
			}
			return 1.0f;
		}

		return ShadowCascadeSplitFactors[CascadeIndex];
	}

	void CalculateCameraNearFar(
		const DirectX::XMMATRIX& ProjectionMatrix,
		float& OutNearPlaneDistance,
		float& OutFarPlaneDistance)
	{
		const float MatrixElement33 = ProjectionMatrix.r[2].m128_f32[2];
		const float MatrixElement43 = ProjectionMatrix.r[3].m128_f32[2];
		const float SafeElement33 = (std::fabs(MatrixElement33) < 0.0001f) ? 0.0001f : MatrixElement33;
		const float SafeFarDenominator = (std::fabs(MatrixElement33 - 1.0f) < 0.0001f) ? 0.0001f : (MatrixElement33 - 1.0f);
		OutNearPlaneDistance = -MatrixElement43 / SafeElement33;
		OutFarPlaneDistance = -MatrixElement43 / SafeFarDenominator;
		OutNearPlaneDistance = (std::max)(0.01f, OutNearPlaneDistance);
		OutFarPlaneDistance = (std::max)(OutNearPlaneDistance + 1.0f, OutFarPlaneDistance);
	}

	void BuildBaseFrustumCorners(
		const DirectX::XMMATRIX& InverseViewProjectionMatrix,
		std::array<DirectX::XMVECTOR, 8>& OutFrustumCorners)
	{
		const std::array<DirectX::XMFLOAT2, 4> NearPlaneCoordinates =
		{
			DirectX::XMFLOAT2(-1.0f, -1.0f),
			DirectX::XMFLOAT2(-1.0f, 1.0f),
			DirectX::XMFLOAT2(1.0f, 1.0f),
			DirectX::XMFLOAT2(1.0f, -1.0f)
		};

		for (int CornerIndex = 0; CornerIndex < 4; ++CornerIndex)
		{
			const DirectX::XMFLOAT2 ExistingNearPlaneCoordinate = NearPlaneCoordinates[CornerIndex];
			DirectX::XMVECTOR NearPointClip = DirectX::XMVectorSet(
				ExistingNearPlaneCoordinate.x,
				ExistingNearPlaneCoordinate.y,
				0.0f,
				1.0f);
			DirectX::XMVECTOR FarPointClip = DirectX::XMVectorSet(
				ExistingNearPlaneCoordinate.x,
				ExistingNearPlaneCoordinate.y,
				1.0f,
				1.0f);
			DirectX::XMVECTOR NearPointWorld = DirectX::XMVector4Transform(NearPointClip, InverseViewProjectionMatrix);
			DirectX::XMVECTOR FarPointWorld = DirectX::XMVector4Transform(FarPointClip, InverseViewProjectionMatrix);
			NearPointWorld = DirectX::XMVectorScale(NearPointWorld, 1.0f / DirectX::XMVectorGetW(NearPointWorld));
			FarPointWorld = DirectX::XMVectorScale(FarPointWorld, 1.0f / DirectX::XMVectorGetW(FarPointWorld));
			OutFrustumCorners[CornerIndex] = NearPointWorld;
			OutFrustumCorners[CornerIndex + 4] = FarPointWorld;
		}
	}
}

struct ShadowVolumeTransformBufferData
{
	DirectX::XMFLOAT4X4 WorldViewProjectionMatrix;
};

struct DeferredCameraBufferData
{
	DirectX::XMFLOAT4X4 ViewMatrix;
	DirectX::XMFLOAT4X4 InverseViewProjectionMatrix;
	DirectX::XMFLOAT3 CameraWorldPosition;
	float Padding0;
};

struct DeferredLightBufferData
{
	DirectX::XMFLOAT3 DirectionalLightDirection;
	float DirectionalLightIntensity;
	DirectX::XMFLOAT4 DirectionalLightColor;
	float PointLightCountValue;
	float SpotLightCountValue;
	DirectX::XMFLOAT2 LightCountPadding0;
	DeferredPointLightData PointLights[MaximumDeferredPointLightCount];
	DeferredSpotLightData SpotLights[MaximumDeferredSpotLightCount];
	float UseFullBrightnessWithoutLighting;
	float ShadowBias;
	float ShadowStrength;
	float ShadowMapTexelSize;
	DirectX::XMFLOAT4 CascadeSplitDepths;
	float ShadowCascadeCountValue;
	DirectX::XMFLOAT3 ShadowCascadeCountValuePadding;
	DirectX::XMFLOAT4X4 CascadeViewProjectionMatrices[ShadowCascadeCount];
	uint32_t LightingPassMode;
	uint32_t SingleLightIndex;
	uint32_t LightingPassPadding0;
	uint32_t LightingPassPadding1;
	float DeferredDebugBufferViewMode;
	DirectX::XMFLOAT3 DeferredDebugBufferViewModePadding;
};

DeferredRenderer::DeferredRenderer()
	: GBufferAlbedoTexture(nullptr)
	, GBufferNormalTexture(nullptr)
	, GBufferMaterialTexture(nullptr)
	, GBufferDepthTexture(nullptr)
	, GBufferAlbedoRTV(nullptr)
	, GBufferNormalRTV(nullptr)
	, GBufferMaterialRTV(nullptr)
	, GBufferAlbedoSRV(nullptr)
	, GBufferNormalSRV(nullptr)
	, GBufferMaterialSRV(nullptr)
	, GBufferDepthSRV(nullptr)
	, GBufferDepthDSV(nullptr)
	, LightingVertexShader(nullptr)
	, LightingPixelShader(nullptr)
	, LightingVertexShaderByteCode(nullptr)
	, LightingPixelShaderByteCode(nullptr)
	, CameraConstantBuffer(nullptr)
	, LightConstantBuffer(nullptr)
	, GBufferSampler(nullptr)
	, ShadowDepthTextureArray(nullptr)
	, ShadowDepthSRV(nullptr)
	, ShadowComparisonSampler(nullptr)
	, ShadowRasterizerState(nullptr)
	, ShadowVolumeVertexShader(nullptr)
	, ShadowVolumeVertexShaderByteCode(nullptr)
	, ShadowVolumeInputLayout(nullptr)
	, ShadowVolumeTransformConstantBuffer(nullptr)
	, ShadowVolumeDepthStencilDepthFailPass1(nullptr)
	, ShadowVolumeDepthStencilDepthFailPass2(nullptr)
	, ShadowVolumeRasterizerCullFront(nullptr)
	, ShadowVolumeRasterizerCullBack(nullptr)
	, LightingStencilEqualZeroState(nullptr)
	, LightingAdditiveBlendState(nullptr)
	, ShadowVolumeColorWriteDisabledBlendState(nullptr)
	, ShadowCascadeSplitDepths(10.0f, 30.0f, 60.0f, 120.0f)
	, ShadowMapResolution(2048)
	, ShadowCascadeCountSetting(ShadowCascadeCount)
	, ShadowMaximumDistanceSetting(160.0f)
	, CachedWidth(0)
	, CachedHeight(0)
{
	for (int CascadeIndex = 0; CascadeIndex < ShadowCascadeCount; ++CascadeIndex)
	{
		ShadowDepthDSVs[CascadeIndex] = nullptr;
		DirectX::XMStoreFloat4x4(&ShadowCascadeViewMatricesStorage[CascadeIndex], DirectX::XMMatrixIdentity());
		DirectX::XMStoreFloat4x4(&ShadowCascadeProjectionMatricesStorage[CascadeIndex], DirectX::XMMatrixIdentity());
		DirectX::XMStoreFloat4x4(&ShadowCascadeViewProjectionMatricesStorage[CascadeIndex], DirectX::XMMatrixIdentity());
	}
}

DeferredRenderer::~DeferredRenderer()
{
	Shutdown();
}

void DeferredRenderer::Initialize(ID3D11Device* Device)
{
	if (Device == nullptr)
	{
		return;
	}

	CompileShader(Device, "./Shaders/Deferred/DeferredLightingPass.hlsl", "VSMain", "vs_5_0", &LightingVertexShaderByteCode, reinterpret_cast<ID3D11DeviceChild**>(&LightingVertexShader));
	CompileShader(Device, "./Shaders/Deferred/DeferredLightingPass.hlsl", "PSMain", "ps_5_0", &LightingPixelShaderByteCode, reinterpret_cast<ID3D11DeviceChild**>(&LightingPixelShader));

	D3D11_BUFFER_DESC CameraBufferDescription = {};
	CameraBufferDescription.Usage = D3D11_USAGE_DEFAULT;
	CameraBufferDescription.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	CameraBufferDescription.ByteWidth = static_cast<UINT>(sizeof(DeferredCameraBufferData));
	Device->CreateBuffer(&CameraBufferDescription, nullptr, &CameraConstantBuffer);

	D3D11_BUFFER_DESC LightBufferDescription = {};
	LightBufferDescription.Usage = D3D11_USAGE_DEFAULT;
	LightBufferDescription.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	LightBufferDescription.ByteWidth = static_cast<UINT>(sizeof(DeferredLightBufferData));
	Device->CreateBuffer(&LightBufferDescription, nullptr, &LightConstantBuffer);

	D3D11_SAMPLER_DESC SamplerDescription = {};
	SamplerDescription.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	SamplerDescription.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	SamplerDescription.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	SamplerDescription.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	SamplerDescription.MinLOD = 0.0f;
	SamplerDescription.MaxLOD = D3D11_FLOAT32_MAX;
	Device->CreateSamplerState(&SamplerDescription, &GBufferSampler);

	D3D11_TEXTURE2D_DESC ShadowDepthTextureDescription = {};
	ShadowDepthTextureDescription.Width = static_cast<UINT>(ShadowMapResolution);
	ShadowDepthTextureDescription.Height = static_cast<UINT>(ShadowMapResolution);
	ShadowDepthTextureDescription.MipLevels = 1;
	ShadowDepthTextureDescription.ArraySize = ShadowCascadeCount;
	ShadowDepthTextureDescription.Format = DXGI_FORMAT_R32_TYPELESS;
	ShadowDepthTextureDescription.SampleDesc.Count = 1;
	ShadowDepthTextureDescription.Usage = D3D11_USAGE_DEFAULT;
	ShadowDepthTextureDescription.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	Device->CreateTexture2D(&ShadowDepthTextureDescription, nullptr, &ShadowDepthTextureArray);

	for (int CascadeIndex = 0; CascadeIndex < ShadowCascadeCount; ++CascadeIndex)
	{
		D3D11_DEPTH_STENCIL_VIEW_DESC ShadowDepthStencilViewDescription = {};
		ShadowDepthStencilViewDescription.Format = DXGI_FORMAT_D32_FLOAT;
		ShadowDepthStencilViewDescription.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
		ShadowDepthStencilViewDescription.Texture2DArray.MipSlice = 0;
		ShadowDepthStencilViewDescription.Texture2DArray.FirstArraySlice = static_cast<UINT>(CascadeIndex);
		ShadowDepthStencilViewDescription.Texture2DArray.ArraySize = 1;
		Device->CreateDepthStencilView(ShadowDepthTextureArray, &ShadowDepthStencilViewDescription, &ShadowDepthDSVs[CascadeIndex]);
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC ShadowShaderResourceViewDescription = {};
	ShadowShaderResourceViewDescription.Format = DXGI_FORMAT_R32_FLOAT;
	ShadowShaderResourceViewDescription.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
	ShadowShaderResourceViewDescription.Texture2DArray.MostDetailedMip = 0;
	ShadowShaderResourceViewDescription.Texture2DArray.MipLevels = 1;
	ShadowShaderResourceViewDescription.Texture2DArray.FirstArraySlice = 0;
	ShadowShaderResourceViewDescription.Texture2DArray.ArraySize = ShadowCascadeCount;
	Device->CreateShaderResourceView(ShadowDepthTextureArray, &ShadowShaderResourceViewDescription, &ShadowDepthSRV);

	D3D11_SAMPLER_DESC ShadowSamplerDescription = {};
	ShadowSamplerDescription.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
	ShadowSamplerDescription.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	ShadowSamplerDescription.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	ShadowSamplerDescription.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	ShadowSamplerDescription.BorderColor[0] = 1.0f;
	ShadowSamplerDescription.BorderColor[1] = 1.0f;
	ShadowSamplerDescription.BorderColor[2] = 1.0f;
	ShadowSamplerDescription.BorderColor[3] = 1.0f;
	ShadowSamplerDescription.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
	ShadowSamplerDescription.MinLOD = 0.0f;
	ShadowSamplerDescription.MaxLOD = D3D11_FLOAT32_MAX;
	Device->CreateSamplerState(&ShadowSamplerDescription, &ShadowComparisonSampler);

	D3D11_RASTERIZER_DESC ShadowRasterizerDescription = {};
	ShadowRasterizerDescription.FillMode = D3D11_FILL_SOLID;
	ShadowRasterizerDescription.CullMode = D3D11_CULL_BACK;
	ShadowRasterizerDescription.DepthBias = 1500;
	ShadowRasterizerDescription.DepthBiasClamp = 0.0f;
	ShadowRasterizerDescription.SlopeScaledDepthBias = 2.5f;
	ShadowRasterizerDescription.DepthClipEnable = TRUE;
	Device->CreateRasterizerState(&ShadowRasterizerDescription, &ShadowRasterizerState);

	CompileShader(
		Device,
		"./Shaders/Deferred/ShadowVolumePass.hlsl",
		"VSMain",
		"vs_5_0",
		&ShadowVolumeVertexShaderByteCode,
		reinterpret_cast<ID3D11DeviceChild**>(&ShadowVolumeVertexShader));

	D3D11_INPUT_ELEMENT_DESC ShadowVolumeInputElementDescription = {};
	ShadowVolumeInputElementDescription.SemanticName = "POSITION";
	ShadowVolumeInputElementDescription.SemanticIndex = 0;
	ShadowVolumeInputElementDescription.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	ShadowVolumeInputElementDescription.InputSlot = 0;
	ShadowVolumeInputElementDescription.AlignedByteOffset = 0;
	ShadowVolumeInputElementDescription.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	ShadowVolumeInputElementDescription.InstanceDataStepRate = 0;
	Device->CreateInputLayout(
		&ShadowVolumeInputElementDescription,
		1,
		ShadowVolumeVertexShaderByteCode->GetBufferPointer(),
		ShadowVolumeVertexShaderByteCode->GetBufferSize(),
		&ShadowVolumeInputLayout);

	D3D11_BUFFER_DESC ShadowVolumeTransformBufferDescription = {};
	ShadowVolumeTransformBufferDescription.Usage = D3D11_USAGE_DEFAULT;
	ShadowVolumeTransformBufferDescription.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	ShadowVolumeTransformBufferDescription.ByteWidth = static_cast<UINT>(sizeof(ShadowVolumeTransformBufferData));
	Device->CreateBuffer(&ShadowVolumeTransformBufferDescription, nullptr, &ShadowVolumeTransformConstantBuffer);

	D3D11_DEPTH_STENCIL_DESC ShadowVolumeDepthStencilPass1Description = {};
	ShadowVolumeDepthStencilPass1Description.DepthEnable = TRUE;
	ShadowVolumeDepthStencilPass1Description.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	ShadowVolumeDepthStencilPass1Description.DepthFunc = D3D11_COMPARISON_LESS;
	ShadowVolumeDepthStencilPass1Description.StencilEnable = TRUE;
	ShadowVolumeDepthStencilPass1Description.StencilReadMask = 0xFF;
	ShadowVolumeDepthStencilPass1Description.StencilWriteMask = 0xFF;
	ShadowVolumeDepthStencilPass1Description.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	ShadowVolumeDepthStencilPass1Description.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	ShadowVolumeDepthStencilPass1Description.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	ShadowVolumeDepthStencilPass1Description.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	ShadowVolumeDepthStencilPass1Description.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	ShadowVolumeDepthStencilPass1Description.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	ShadowVolumeDepthStencilPass1Description.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	ShadowVolumeDepthStencilPass1Description.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	Device->CreateDepthStencilState(&ShadowVolumeDepthStencilPass1Description, &ShadowVolumeDepthStencilDepthFailPass1);

	D3D11_DEPTH_STENCIL_DESC ShadowVolumeDepthStencilPass2Description = {};
	ShadowVolumeDepthStencilPass2Description.DepthEnable = TRUE;
	ShadowVolumeDepthStencilPass2Description.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	ShadowVolumeDepthStencilPass2Description.DepthFunc = D3D11_COMPARISON_LESS;
	ShadowVolumeDepthStencilPass2Description.StencilEnable = TRUE;
	ShadowVolumeDepthStencilPass2Description.StencilReadMask = 0xFF;
	ShadowVolumeDepthStencilPass2Description.StencilWriteMask = 0xFF;
	ShadowVolumeDepthStencilPass2Description.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	ShadowVolumeDepthStencilPass2Description.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	ShadowVolumeDepthStencilPass2Description.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	ShadowVolumeDepthStencilPass2Description.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	ShadowVolumeDepthStencilPass2Description.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	ShadowVolumeDepthStencilPass2Description.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	ShadowVolumeDepthStencilPass2Description.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	ShadowVolumeDepthStencilPass2Description.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	Device->CreateDepthStencilState(&ShadowVolumeDepthStencilPass2Description, &ShadowVolumeDepthStencilDepthFailPass2);

	D3D11_RASTERIZER_DESC ShadowVolumeRasterizerCullFrontDescription = {};
	ShadowVolumeRasterizerCullFrontDescription.FillMode = D3D11_FILL_SOLID;
	ShadowVolumeRasterizerCullFrontDescription.CullMode = D3D11_CULL_FRONT;
	ShadowVolumeRasterizerCullFrontDescription.FrontCounterClockwise = FALSE;
	ShadowVolumeRasterizerCullFrontDescription.DepthClipEnable = TRUE;
	Device->CreateRasterizerState(&ShadowVolumeRasterizerCullFrontDescription, &ShadowVolumeRasterizerCullFront);

	D3D11_RASTERIZER_DESC ShadowVolumeRasterizerCullBackDescription = {};
	ShadowVolumeRasterizerCullBackDescription.FillMode = D3D11_FILL_SOLID;
	ShadowVolumeRasterizerCullBackDescription.CullMode = D3D11_CULL_BACK;
	ShadowVolumeRasterizerCullBackDescription.FrontCounterClockwise = FALSE;
	ShadowVolumeRasterizerCullBackDescription.DepthClipEnable = TRUE;
	Device->CreateRasterizerState(&ShadowVolumeRasterizerCullBackDescription, &ShadowVolumeRasterizerCullBack);

	D3D11_DEPTH_STENCIL_DESC LightingStencilEqualZeroDescription = {};
	LightingStencilEqualZeroDescription.DepthEnable = FALSE;
	LightingStencilEqualZeroDescription.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	LightingStencilEqualZeroDescription.DepthFunc = D3D11_COMPARISON_ALWAYS;
	LightingStencilEqualZeroDescription.StencilEnable = TRUE;
	LightingStencilEqualZeroDescription.StencilReadMask = 0xFF;
	LightingStencilEqualZeroDescription.StencilWriteMask = 0x00;
	LightingStencilEqualZeroDescription.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	LightingStencilEqualZeroDescription.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	LightingStencilEqualZeroDescription.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	LightingStencilEqualZeroDescription.FrontFace.StencilFunc = D3D11_COMPARISON_EQUAL;
	LightingStencilEqualZeroDescription.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	LightingStencilEqualZeroDescription.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	LightingStencilEqualZeroDescription.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	LightingStencilEqualZeroDescription.BackFace.StencilFunc = D3D11_COMPARISON_EQUAL;
	Device->CreateDepthStencilState(&LightingStencilEqualZeroDescription, &LightingStencilEqualZeroState);

	D3D11_BLEND_DESC LightingAdditiveBlendDescription = {};
	LightingAdditiveBlendDescription.RenderTarget[0].BlendEnable = TRUE;
	LightingAdditiveBlendDescription.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	LightingAdditiveBlendDescription.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	LightingAdditiveBlendDescription.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	LightingAdditiveBlendDescription.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
	LightingAdditiveBlendDescription.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
	LightingAdditiveBlendDescription.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	LightingAdditiveBlendDescription.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	Device->CreateBlendState(&LightingAdditiveBlendDescription, &LightingAdditiveBlendState);

	D3D11_BLEND_DESC ShadowVolumeColorWriteDisabledBlendDescription = {};
	for (int RenderTargetIndex = 0; RenderTargetIndex < 8; ++RenderTargetIndex)
	{
		ShadowVolumeColorWriteDisabledBlendDescription.RenderTarget[RenderTargetIndex].RenderTargetWriteMask = 0;
	}
	Device->CreateBlendState(&ShadowVolumeColorWriteDisabledBlendDescription, &ShadowVolumeColorWriteDisabledBlendState);
}

void DeferredRenderer::Shutdown()
{
	ReleaseTargets();
	ReleaseShadowResources();
	ReleaseStencilShadowVolumeResources();

	if (LightingVertexShader != nullptr)
	{
		LightingVertexShader->Release();
		LightingVertexShader = nullptr;
	}

	if (LightingPixelShader != nullptr)
	{
		LightingPixelShader->Release();
		LightingPixelShader = nullptr;
	}

	if (LightingVertexShaderByteCode != nullptr)
	{
		LightingVertexShaderByteCode->Release();
		LightingVertexShaderByteCode = nullptr;
	}

	if (LightingPixelShaderByteCode != nullptr)
	{
		LightingPixelShaderByteCode->Release();
		LightingPixelShaderByteCode = nullptr;
	}

	if (CameraConstantBuffer != nullptr)
	{
		CameraConstantBuffer->Release();
		CameraConstantBuffer = nullptr;
	}

	if (LightConstantBuffer != nullptr)
	{
		LightConstantBuffer->Release();
		LightConstantBuffer = nullptr;
	}

	if (GBufferSampler != nullptr)
	{
		GBufferSampler->Release();
		GBufferSampler = nullptr;
	}

	if (ShadowComparisonSampler != nullptr)
	{
		ShadowComparisonSampler->Release();
		ShadowComparisonSampler = nullptr;
	}

	if (ShadowRasterizerState != nullptr)
	{
		ShadowRasterizerState->Release();
		ShadowRasterizerState = nullptr;
	}
}

void DeferredRenderer::ReleaseStencilShadowVolumeResources()
{
	if (ShadowVolumeVertexShader != nullptr)
	{
		ShadowVolumeVertexShader->Release();
		ShadowVolumeVertexShader = nullptr;
	}

	if (ShadowVolumeVertexShaderByteCode != nullptr)
	{
		ShadowVolumeVertexShaderByteCode->Release();
		ShadowVolumeVertexShaderByteCode = nullptr;
	}

	if (ShadowVolumeInputLayout != nullptr)
	{
		ShadowVolumeInputLayout->Release();
		ShadowVolumeInputLayout = nullptr;
	}

	if (ShadowVolumeTransformConstantBuffer != nullptr)
	{
		ShadowVolumeTransformConstantBuffer->Release();
		ShadowVolumeTransformConstantBuffer = nullptr;
	}

	if (ShadowVolumeDepthStencilDepthFailPass1 != nullptr)
	{
		ShadowVolumeDepthStencilDepthFailPass1->Release();
		ShadowVolumeDepthStencilDepthFailPass1 = nullptr;
	}

	if (ShadowVolumeDepthStencilDepthFailPass2 != nullptr)
	{
		ShadowVolumeDepthStencilDepthFailPass2->Release();
		ShadowVolumeDepthStencilDepthFailPass2 = nullptr;
	}

	if (ShadowVolumeRasterizerCullFront != nullptr)
	{
		ShadowVolumeRasterizerCullFront->Release();
		ShadowVolumeRasterizerCullFront = nullptr;
	}

	if (ShadowVolumeRasterizerCullBack != nullptr)
	{
		ShadowVolumeRasterizerCullBack->Release();
		ShadowVolumeRasterizerCullBack = nullptr;
	}

	if (LightingStencilEqualZeroState != nullptr)
	{
		LightingStencilEqualZeroState->Release();
		LightingStencilEqualZeroState = nullptr;
	}

	if (LightingAdditiveBlendState != nullptr)
	{
		LightingAdditiveBlendState->Release();
		LightingAdditiveBlendState = nullptr;
	}

	if (ShadowVolumeColorWriteDisabledBlendState != nullptr)
	{
		ShadowVolumeColorWriteDisabledBlendState->Release();
		ShadowVolumeColorWriteDisabledBlendState = nullptr;
	}
}

void DeferredRenderer::EnsureTargets(ID3D11Device* Device, int ScreenWidth, int ScreenHeight)
{
	if (Device == nullptr || ScreenWidth <= 0 || ScreenHeight <= 0)
	{
		return;
	}

	if (CachedWidth == ScreenWidth && CachedHeight == ScreenHeight && GBufferAlbedoRTV != nullptr)
	{
		return;
	}

	ReleaseTargets();
	CachedWidth = ScreenWidth;
	CachedHeight = ScreenHeight;

	D3D11_TEXTURE2D_DESC RenderTargetDescription = {};
	RenderTargetDescription.Width = static_cast<UINT>(ScreenWidth);
	RenderTargetDescription.Height = static_cast<UINT>(ScreenHeight);
	RenderTargetDescription.MipLevels = 1;
	RenderTargetDescription.ArraySize = 1;
	RenderTargetDescription.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	RenderTargetDescription.SampleDesc.Count = 1;
	RenderTargetDescription.Usage = D3D11_USAGE_DEFAULT;
	RenderTargetDescription.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

	Device->CreateTexture2D(&RenderTargetDescription, nullptr, &GBufferAlbedoTexture);
	Device->CreateTexture2D(&RenderTargetDescription, nullptr, &GBufferNormalTexture);
	Device->CreateTexture2D(&RenderTargetDescription, nullptr, &GBufferMaterialTexture);
	Device->CreateRenderTargetView(GBufferAlbedoTexture, nullptr, &GBufferAlbedoRTV);
	Device->CreateRenderTargetView(GBufferNormalTexture, nullptr, &GBufferNormalRTV);
	Device->CreateRenderTargetView(GBufferMaterialTexture, nullptr, &GBufferMaterialRTV);
	Device->CreateShaderResourceView(GBufferAlbedoTexture, nullptr, &GBufferAlbedoSRV);
	Device->CreateShaderResourceView(GBufferNormalTexture, nullptr, &GBufferNormalSRV);
	Device->CreateShaderResourceView(GBufferMaterialTexture, nullptr, &GBufferMaterialSRV);

	D3D11_TEXTURE2D_DESC DepthDescription = {};
	DepthDescription.Width = static_cast<UINT>(ScreenWidth);
	DepthDescription.Height = static_cast<UINT>(ScreenHeight);
	DepthDescription.MipLevels = 1;
	DepthDescription.ArraySize = 1;
	DepthDescription.Format = DXGI_FORMAT_R24G8_TYPELESS;
	DepthDescription.SampleDesc.Count = 1;
	DepthDescription.Usage = D3D11_USAGE_DEFAULT;
	DepthDescription.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	Device->CreateTexture2D(&DepthDescription, nullptr, &GBufferDepthTexture);

	D3D11_DEPTH_STENCIL_VIEW_DESC DepthStencilViewDescription = {};
	DepthStencilViewDescription.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	DepthStencilViewDescription.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	DepthStencilViewDescription.Texture2D.MipSlice = 0;
	Device->CreateDepthStencilView(GBufferDepthTexture, &DepthStencilViewDescription, &GBufferDepthDSV);

	D3D11_SHADER_RESOURCE_VIEW_DESC DepthShaderResourceDescription = {};
	DepthShaderResourceDescription.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	DepthShaderResourceDescription.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	DepthShaderResourceDescription.Texture2D.MostDetailedMip = 0;
	DepthShaderResourceDescription.Texture2D.MipLevels = 1;
	Device->CreateShaderResourceView(GBufferDepthTexture, &DepthShaderResourceDescription, &GBufferDepthSRV);
}

void DeferredRenderer::BeginGeometryPass(ID3D11DeviceContext* DeviceContext)
{
	if (DeviceContext == nullptr || GBufferDepthDSV == nullptr)
	{
		return;
	}

	ID3D11RenderTargetView* GeometryTargets[3] = {
		GBufferAlbedoRTV,
		GBufferNormalRTV,
		GBufferMaterialRTV
	};
	DeviceContext->OMSetRenderTargets(3, GeometryTargets, GBufferDepthDSV);

	const float ClearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	DeviceContext->ClearRenderTargetView(GBufferAlbedoRTV, ClearColor);
	DeviceContext->ClearRenderTargetView(GBufferNormalRTV, ClearColor);
	DeviceContext->ClearRenderTargetView(GBufferMaterialRTV, ClearColor);
	DeviceContext->ClearDepthStencilView(GBufferDepthDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

void DeferredRenderer::EndGeometryPass(ID3D11DeviceContext* DeviceContext)
{
	if (DeviceContext == nullptr)
	{
		return;
	}

	ID3D11RenderTargetView* NullRenderTargets[3] = { nullptr, nullptr, nullptr };
	DeviceContext->OMSetRenderTargets(3, NullRenderTargets, nullptr);
}

void DeferredRenderer::ExecuteDeferredLightingDrawCall(
	ID3D11DeviceContext* DeviceContext,
	ID3D11RenderTargetView* FinalRenderTargetView,
	bool UseAdditiveBlend,
	bool IsStencilShadowTestEnabled)
{
	if (DeviceContext == nullptr || FinalRenderTargetView == nullptr || LightingVertexShader == nullptr || LightingPixelShader == nullptr)
	{
		return;
	}

	if (UseAdditiveBlend)
	{
		DeviceContext->OMSetBlendState(LightingAdditiveBlendState, nullptr, 0xffffffff);
	}
	else
	{
		DeviceContext->OMSetBlendState(nullptr, nullptr, 0xffffffff);
	}

	if (IsStencilShadowTestEnabled && GBufferDepthDSV != nullptr)
	{
		DeviceContext->OMSetRenderTargets(1, &FinalRenderTargetView, GBufferDepthDSV);
		DeviceContext->OMSetDepthStencilState(LightingStencilEqualZeroState, 0);
	}
	else
	{
		DeviceContext->OMSetRenderTargets(1, &FinalRenderTargetView, nullptr);
		DeviceContext->OMSetDepthStencilState(nullptr, 0);
	}

	ID3D11ShaderResourceView* ShaderResourceViews[5] = {
		GBufferAlbedoSRV,
		GBufferNormalSRV,
		GBufferMaterialSRV,
		GBufferDepthSRV,
		ShadowDepthSRV
	};
	ID3D11SamplerState* SamplerStates[2] = { GBufferSampler, ShadowComparisonSampler };
	DeviceContext->PSSetShaderResources(0, 5, ShaderResourceViews);
	DeviceContext->PSSetSamplers(0, 2, SamplerStates);
	DeviceContext->VSSetShader(LightingVertexShader, nullptr, 0);
	DeviceContext->PSSetShader(LightingPixelShader, nullptr, 0);
	DeviceContext->VSSetConstantBuffers(0, 1, &CameraConstantBuffer);
	DeviceContext->PSSetConstantBuffers(0, 1, &CameraConstantBuffer);
	DeviceContext->PSSetConstantBuffers(1, 1, &LightConstantBuffer);
	DeviceContext->IASetInputLayout(nullptr);
	DeviceContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	DeviceContext->Draw(3, 0);

	ID3D11ShaderResourceView* NullShaderResources[5] = { nullptr, nullptr, nullptr, nullptr, nullptr };
	DeviceContext->PSSetShaderResources(0, 5, NullShaderResources);
	DeviceContext->OMSetBlendState(nullptr, nullptr, 0xffffffff);
	DeviceContext->OMSetDepthStencilState(nullptr, 0);
	DeviceContext->OMSetRenderTargets(0, nullptr, nullptr);
}

void DeferredRenderer::RenderDirectionalDeferredLightingPass(
	ID3D11DeviceContext* DeviceContext,
	ID3D11RenderTargetView* FinalRenderTargetView,
	const DirectX::XMMATRIX& ViewMatrix,
	const DirectX::XMMATRIX& InverseViewProjectionMatrix,
	const DirectX::XMFLOAT3& CameraWorldPosition,
	const DirectX::XMFLOAT3& DirectionalLightDirection,
	const DirectX::XMFLOAT4& DirectionalLightColor,
	float DirectionalLightIntensity,
	float UseFullBrightnessWithoutLighting,
	float ShadowStrength,
	float DeferredDebugBufferViewMode)
{
	if (DeviceContext == nullptr || FinalRenderTargetView == nullptr || LightingVertexShader == nullptr || LightingPixelShader == nullptr)
	{
		return;
	}

	DeferredCameraBufferData CameraBufferData = {};
	DirectX::XMStoreFloat4x4(&CameraBufferData.ViewMatrix, DirectX::XMMatrixTranspose(ViewMatrix));
	DirectX::XMStoreFloat4x4(&CameraBufferData.InverseViewProjectionMatrix, DirectX::XMMatrixTranspose(InverseViewProjectionMatrix));
	CameraBufferData.CameraWorldPosition = CameraWorldPosition;
	DeviceContext->UpdateSubresource(CameraConstantBuffer, 0, nullptr, &CameraBufferData, 0, 0);

	DeferredLightBufferData LightBufferData = {};
	LightBufferData.DirectionalLightDirection = DirectionalLightDirection;
	LightBufferData.DirectionalLightColor = DirectionalLightColor;
	LightBufferData.DirectionalLightIntensity = DirectionalLightIntensity;
	LightBufferData.PointLightCountValue = 0.0f;
	LightBufferData.SpotLightCountValue = 0.0f;
	LightBufferData.UseFullBrightnessWithoutLighting = UseFullBrightnessWithoutLighting;
	LightBufferData.ShadowBias = 0.0015f;
	LightBufferData.ShadowStrength = ShadowStrength;
	LightBufferData.ShadowMapTexelSize = 1.0f / static_cast<float>(ShadowMapResolution);
	LightBufferData.CascadeSplitDepths = ShadowCascadeSplitDepths;
	LightBufferData.ShadowCascadeCountValue = static_cast<float>(ShadowCascadeCountSetting);
	for (int CascadeIndex = 0; CascadeIndex < ShadowCascadeCount; ++CascadeIndex)
	{
		LightBufferData.CascadeViewProjectionMatrices[CascadeIndex] = ShadowCascadeViewProjectionMatricesStorage[CascadeIndex];
	}
	LightBufferData.LightingPassMode = 0u;
	LightBufferData.SingleLightIndex = 0u;
	LightBufferData.LightingPassPadding0 = 0u;
	LightBufferData.LightingPassPadding1 = 0u;
	LightBufferData.DeferredDebugBufferViewMode = DeferredDebugBufferViewMode;
	DeviceContext->UpdateSubresource(LightConstantBuffer, 0, nullptr, &LightBufferData, 0, 0);

	ExecuteDeferredLightingDrawCall(DeviceContext, FinalRenderTargetView, false, false);
}

void DeferredRenderer::RenderSinglePointLightDeferredLightingPass(
	ID3D11DeviceContext* DeviceContext,
	ID3D11RenderTargetView* FinalRenderTargetView,
	const DirectX::XMMATRIX& ViewMatrix,
	const DirectX::XMMATRIX& InverseViewProjectionMatrix,
	const DirectX::XMFLOAT3& CameraWorldPosition,
	const std::vector<DeferredPointLightData>& PointLights,
	int SinglePointLightIndex,
	float UseFullBrightnessWithoutLighting,
	float ShadowStrength,
	float DeferredDebugBufferViewMode,
	bool IsStencilShadowTestEnabled)
{
	if (DeviceContext == nullptr || FinalRenderTargetView == nullptr || LightingVertexShader == nullptr || LightingPixelShader == nullptr)
	{
		return;
	}

	if (SinglePointLightIndex < 0 || SinglePointLightIndex >= MaximumDeferredPointLightCount)
	{
		return;
	}

	DeferredCameraBufferData CameraBufferData = {};
	DirectX::XMStoreFloat4x4(&CameraBufferData.ViewMatrix, DirectX::XMMatrixTranspose(ViewMatrix));
	DirectX::XMStoreFloat4x4(&CameraBufferData.InverseViewProjectionMatrix, DirectX::XMMatrixTranspose(InverseViewProjectionMatrix));
	CameraBufferData.CameraWorldPosition = CameraWorldPosition;
	DeviceContext->UpdateSubresource(CameraConstantBuffer, 0, nullptr, &CameraBufferData, 0, 0);

	DeferredLightBufferData LightBufferData = {};
	LightBufferData.DirectionalLightDirection = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
	LightBufferData.DirectionalLightIntensity = 0.0f;
	LightBufferData.DirectionalLightColor = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	const int PointLightCount = (std::min)(static_cast<int>(PointLights.size()), MaximumDeferredPointLightCount);
	for (int PointLightIndex = 0; PointLightIndex < PointLightCount; ++PointLightIndex)
	{
		LightBufferData.PointLights[PointLightIndex] = PointLights[PointLightIndex];
	}
	LightBufferData.PointLightCountValue = 0.0f;
	LightBufferData.SpotLightCountValue = 0.0f;
	LightBufferData.UseFullBrightnessWithoutLighting = UseFullBrightnessWithoutLighting;
	LightBufferData.ShadowBias = 0.0015f;
	LightBufferData.ShadowStrength = ShadowStrength;
	LightBufferData.ShadowMapTexelSize = 1.0f / static_cast<float>(ShadowMapResolution);
	LightBufferData.CascadeSplitDepths = ShadowCascadeSplitDepths;
	LightBufferData.ShadowCascadeCountValue = static_cast<float>(ShadowCascadeCountSetting);
	for (int CascadeIndex = 0; CascadeIndex < ShadowCascadeCount; ++CascadeIndex)
	{
		LightBufferData.CascadeViewProjectionMatrices[CascadeIndex] = ShadowCascadeViewProjectionMatricesStorage[CascadeIndex];
	}
	LightBufferData.LightingPassMode = 1u;
	LightBufferData.SingleLightIndex = static_cast<uint32_t>(SinglePointLightIndex);
	LightBufferData.LightingPassPadding0 = 0u;
	LightBufferData.LightingPassPadding1 = 0u;
	LightBufferData.DeferredDebugBufferViewMode = DeferredDebugBufferViewMode;
	DeviceContext->UpdateSubresource(LightConstantBuffer, 0, nullptr, &LightBufferData, 0, 0);

	ExecuteDeferredLightingDrawCall(DeviceContext, FinalRenderTargetView, true, IsStencilShadowTestEnabled);
}

void DeferredRenderer::RenderSingleSpotLightDeferredLightingPass(
	ID3D11DeviceContext* DeviceContext,
	ID3D11RenderTargetView* FinalRenderTargetView,
	const DirectX::XMMATRIX& ViewMatrix,
	const DirectX::XMMATRIX& InverseViewProjectionMatrix,
	const DirectX::XMFLOAT3& CameraWorldPosition,
	const std::vector<DeferredSpotLightData>& SpotLights,
	int SingleSpotLightIndex,
	float UseFullBrightnessWithoutLighting,
	float ShadowStrength,
	float DeferredDebugBufferViewMode,
	bool IsStencilShadowTestEnabled)
{
	if (DeviceContext == nullptr || FinalRenderTargetView == nullptr || LightingVertexShader == nullptr || LightingPixelShader == nullptr)
	{
		return;
	}

	if (SingleSpotLightIndex < 0 || SingleSpotLightIndex >= MaximumDeferredSpotLightCount)
	{
		return;
	}

	DeferredCameraBufferData CameraBufferData = {};
	DirectX::XMStoreFloat4x4(&CameraBufferData.ViewMatrix, DirectX::XMMatrixTranspose(ViewMatrix));
	DirectX::XMStoreFloat4x4(&CameraBufferData.InverseViewProjectionMatrix, DirectX::XMMatrixTranspose(InverseViewProjectionMatrix));
	CameraBufferData.CameraWorldPosition = CameraWorldPosition;
	DeviceContext->UpdateSubresource(CameraConstantBuffer, 0, nullptr, &CameraBufferData, 0, 0);

	DeferredLightBufferData LightBufferData = {};
	LightBufferData.DirectionalLightDirection = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
	LightBufferData.DirectionalLightIntensity = 0.0f;
	LightBufferData.DirectionalLightColor = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	const int SpotLightCount = (std::min)(static_cast<int>(SpotLights.size()), MaximumDeferredSpotLightCount);
	for (int SpotLightIndex = 0; SpotLightIndex < SpotLightCount; ++SpotLightIndex)
	{
		LightBufferData.SpotLights[SpotLightIndex] = SpotLights[SpotLightIndex];
	}
	LightBufferData.PointLightCountValue = 0.0f;
	LightBufferData.SpotLightCountValue = 0.0f;
	LightBufferData.UseFullBrightnessWithoutLighting = UseFullBrightnessWithoutLighting;
	LightBufferData.ShadowBias = 0.0015f;
	LightBufferData.ShadowStrength = ShadowStrength;
	LightBufferData.ShadowMapTexelSize = 1.0f / static_cast<float>(ShadowMapResolution);
	LightBufferData.CascadeSplitDepths = ShadowCascadeSplitDepths;
	LightBufferData.ShadowCascadeCountValue = static_cast<float>(ShadowCascadeCountSetting);
	for (int CascadeIndex = 0; CascadeIndex < ShadowCascadeCount; ++CascadeIndex)
	{
		LightBufferData.CascadeViewProjectionMatrices[CascadeIndex] = ShadowCascadeViewProjectionMatricesStorage[CascadeIndex];
	}
	LightBufferData.LightingPassMode = 2u;
	LightBufferData.SingleLightIndex = static_cast<uint32_t>(SingleSpotLightIndex);
	LightBufferData.LightingPassPadding0 = 0u;
	LightBufferData.LightingPassPadding1 = 0u;
	LightBufferData.DeferredDebugBufferViewMode = DeferredDebugBufferViewMode;
	DeviceContext->UpdateSubresource(LightConstantBuffer, 0, nullptr, &LightBufferData, 0, 0);

	ExecuteDeferredLightingDrawCall(DeviceContext, FinalRenderTargetView, true, IsStencilShadowTestEnabled);
}

void DeferredRenderer::ClearGBufferStencil(ID3D11DeviceContext* DeviceContext)
{
	if (DeviceContext == nullptr || GBufferDepthDSV == nullptr)
	{
		return;
	}

	DeviceContext->ClearDepthStencilView(GBufferDepthDSV, D3D11_CLEAR_STENCIL, 1.0f, 0);
}

void DeferredRenderer::BeginShadowVolumeStencilPass(ID3D11DeviceContext* DeviceContext)
{
	if (DeviceContext == nullptr || GBufferDepthDSV == nullptr || ShadowVolumeVertexShader == nullptr || ShadowVolumeInputLayout == nullptr)
	{
		return;
	}

	if (ShadowVolumeColorWriteDisabledBlendState != nullptr)
	{
		DeviceContext->OMSetBlendState(ShadowVolumeColorWriteDisabledBlendState, nullptr, 0xffffffff);
	}

	DeviceContext->OMSetRenderTargets(0, nullptr, nullptr);
	DeviceContext->OMSetRenderTargets(0, nullptr, GBufferDepthDSV);
	DeviceContext->VSSetShader(ShadowVolumeVertexShader, nullptr, 0);
	DeviceContext->PSSetShader(nullptr, nullptr, 0);
	DeviceContext->IASetInputLayout(ShadowVolumeInputLayout);
	DeviceContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void DeferredRenderer::EndShadowVolumeStencilPass(ID3D11DeviceContext* DeviceContext)
{
	if (DeviceContext == nullptr)
	{
		return;
	}

	DeviceContext->OMSetRenderTargets(0, nullptr, nullptr);
	DeviceContext->RSSetState(nullptr);
	DeviceContext->OMSetDepthStencilState(nullptr, 0);
	DeviceContext->OMSetBlendState(nullptr, nullptr, 0xffffffff);
	DeviceContext->VSSetShader(nullptr, nullptr, 0);
	DeviceContext->IASetInputLayout(nullptr);
}

void DeferredRenderer::BindShadowVolumePassPipelineForSubPass(ID3D11DeviceContext* DeviceContext, int SubPassIndex) const
{
	if (DeviceContext == nullptr)
	{
		return;
	}

	if (SubPassIndex == 0)
	{
		DeviceContext->RSSetState(ShadowVolumeRasterizerCullFront);
		DeviceContext->OMSetDepthStencilState(ShadowVolumeDepthStencilDepthFailPass1, 0);
	}
	else
	{
		DeviceContext->RSSetState(ShadowVolumeRasterizerCullBack);
		DeviceContext->OMSetDepthStencilState(ShadowVolumeDepthStencilDepthFailPass2, 0);
	}

	DeviceContext->VSSetConstantBuffers(0, 1, &ShadowVolumeTransformConstantBuffer);
}

void DeferredRenderer::PrepareCascadedShadowMaps(
	const DirectX::XMMATRIX& CameraViewMatrix,
	const DirectX::XMMATRIX& CameraProjectionMatrix,
	const DirectX::XMFLOAT3& CameraWorldPosition,
	const DirectX::XMFLOAT3& DirectionalLightDirection)
{
	float CameraNearPlaneDistance = 0.1f;
	float CameraFarPlaneDistance = 1000.0f;
	CalculateCameraNearFar(CameraProjectionMatrix, CameraNearPlaneDistance, CameraFarPlaneDistance);

	const int ActiveShadowCascadeCount = (std::max)(1, (std::min)(ShadowCascadeCountSetting, ShadowCascadeCount));
	const float ClampedShadowMaximumDistance = (std::max)(10.0f, ShadowMaximumDistanceSetting);
	const float ShadowMaximumDistance = (std::min)(CameraFarPlaneDistance, ClampedShadowMaximumDistance);
	std::array<float, ShadowCascadeCount> CascadeSplitDepths = {};
	for (int CascadeIndex = 0; CascadeIndex < ShadowCascadeCount; ++CascadeIndex)
	{
		if (CascadeIndex < ActiveShadowCascadeCount)
		{
			const float NormalizedSplitDepth = GetShadowCascadeSplitFactor(CascadeIndex, ActiveShadowCascadeCount);
			CascadeSplitDepths[CascadeIndex] = CameraNearPlaneDistance + ((ShadowMaximumDistance - CameraNearPlaneDistance) * NormalizedSplitDepth);
		}
		else
		{
			const float InactiveCascadeDepthOffset = 10000.0f + (1000.0f * static_cast<float>(CascadeIndex));
			CascadeSplitDepths[CascadeIndex] = CameraFarPlaneDistance + InactiveCascadeDepthOffset;
		}
	}
	ShadowCascadeSplitDepths = DirectX::XMFLOAT4(
		CascadeSplitDepths[0],
		CascadeSplitDepths[1],
		CascadeSplitDepths[2],
		CascadeSplitDepths[3]);

	const DirectX::XMMATRIX InverseCameraViewProjectionMatrix = DirectX::XMMatrixInverse(nullptr, CameraViewMatrix * CameraProjectionMatrix);
	std::array<DirectX::XMVECTOR, 8> BaseFrustumCorners = {};
	BuildBaseFrustumCorners(InverseCameraViewProjectionMatrix, BaseFrustumCorners);

	const DirectX::XMFLOAT3 LightDirectionNormalized = NormalizeVector3(DirectionalLightDirection);
	const DirectX::XMVECTOR LightDirectionVector = DirectX::XMLoadFloat3(&LightDirectionNormalized);
	const DirectX::XMVECTOR CameraWorldPositionVector = DirectX::XMLoadFloat3(&CameraWorldPosition);
	DirectX::XMVECTOR UpDirection = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	const float ParallelFactor = std::fabs(DirectX::XMVectorGetX(DirectX::XMVector3Dot(DirectX::XMVector3Normalize(LightDirectionVector), UpDirection)));
	if (ParallelFactor > 0.98f)
	{
		UpDirection = DirectX::XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
	}

	float PreviousSplitDepth = CameraNearPlaneDistance;
	for (int CascadeIndex = 0; CascadeIndex < ActiveShadowCascadeCount; ++CascadeIndex)
	{
		const float CascadeSplitDepth = CascadeSplitDepths[CascadeIndex];
		const float NearBlendFactor = (PreviousSplitDepth - CameraNearPlaneDistance) / (CameraFarPlaneDistance - CameraNearPlaneDistance);
		const float FarBlendFactor = (CascadeSplitDepth - CameraNearPlaneDistance) / (CameraFarPlaneDistance - CameraNearPlaneDistance);

		std::array<DirectX::XMVECTOR, 8> CascadeCorners = {};
		for (int CornerIndex = 0; CornerIndex < 4; ++CornerIndex)
		{
			const DirectX::XMVECTOR BaseNearCorner = BaseFrustumCorners[CornerIndex];
			const DirectX::XMVECTOR BaseFarCorner = BaseFrustumCorners[CornerIndex + 4];
			const DirectX::XMVECTOR FrustumSegmentVector = DirectX::XMVectorSubtract(BaseFarCorner, BaseNearCorner);
			CascadeCorners[CornerIndex] = DirectX::XMVectorAdd(
				BaseNearCorner,
				DirectX::XMVectorScale(FrustumSegmentVector, NearBlendFactor));
			CascadeCorners[CornerIndex + 4] = DirectX::XMVectorAdd(
				BaseNearCorner,
				DirectX::XMVectorScale(FrustumSegmentVector, FarBlendFactor));
		}

		DirectX::XMVECTOR CascadeCenter = DirectX::XMVectorZero();
		for (const DirectX::XMVECTOR ExistingCascadeCorner : CascadeCorners)
		{
			CascadeCenter = DirectX::XMVectorAdd(CascadeCenter, ExistingCascadeCorner);
		}
		CascadeCenter = DirectX::XMVectorScale(CascadeCenter, 1.0f / static_cast<float>(CascadeCorners.size()));

		float CascadeRadius = 0.0f;
		for (const DirectX::XMVECTOR ExistingCascadeCorner : CascadeCorners)
		{
			const DirectX::XMVECTOR CornerOffset = DirectX::XMVectorSubtract(ExistingCascadeCorner, CascadeCenter);
			const float CornerDistance = DirectX::XMVectorGetX(DirectX::XMVector3Length(CornerOffset));
			CascadeRadius = (std::max)(CascadeRadius, CornerDistance);
		}
		CascadeRadius = std::ceil(CascadeRadius * 16.0f) / 16.0f;

		const DirectX::XMVECTOR CameraToCascadeVector = DirectX::XMVectorSubtract(CascadeCenter, CameraWorldPositionVector);
		const float CameraToCascadeDistance = DirectX::XMVectorGetX(DirectX::XMVector3Length(CameraToCascadeVector));
		const float LightPullbackDistance = (std::max)(CascadeRadius + 120.0f, CameraToCascadeDistance + 50.0f);
		const DirectX::XMVECTOR LightDirectionNormalizedVector = DirectX::XMVector3Normalize(LightDirectionVector);
		const DirectX::XMVECTOR LightOffset = DirectX::XMVectorScale(LightDirectionNormalizedVector, LightPullbackDistance);
		const DirectX::XMVECTOR LightPosition = DirectX::XMVectorSubtract(CascadeCenter, LightOffset);
		const DirectX::XMMATRIX LightViewMatrix = DirectX::XMMatrixLookAtLH(LightPosition, CascadeCenter, UpDirection);
		const float MinimumBoundX = -CascadeRadius;
		const float MaximumBoundX = CascadeRadius;
		const float MinimumBoundY = -CascadeRadius;
		const float MaximumBoundY = CascadeRadius;
		const float MinimumBoundZ = 0.1f;
		const float MaximumBoundZ = (CascadeRadius * 2.0f) + 240.0f;
		const DirectX::XMMATRIX LightProjectionMatrix = DirectX::XMMatrixOrthographicOffCenterLH(
			MinimumBoundX,
			MaximumBoundX,
			MinimumBoundY,
			MaximumBoundY,
			MinimumBoundZ,
			MaximumBoundZ);
		const DirectX::XMMATRIX LightViewProjectionMatrix = LightViewMatrix * LightProjectionMatrix;

		DirectX::XMStoreFloat4x4(&ShadowCascadeViewMatricesStorage[CascadeIndex], LightViewMatrix);
		DirectX::XMStoreFloat4x4(&ShadowCascadeProjectionMatricesStorage[CascadeIndex], LightProjectionMatrix);
		DirectX::XMStoreFloat4x4(&ShadowCascadeViewProjectionMatricesStorage[CascadeIndex], DirectX::XMMatrixTranspose(LightViewProjectionMatrix));

		PreviousSplitDepth = CascadeSplitDepth;
	}

	for (int CascadeIndex = ActiveShadowCascadeCount; CascadeIndex < ShadowCascadeCount; ++CascadeIndex)
	{
		DirectX::XMStoreFloat4x4(&ShadowCascadeViewMatricesStorage[CascadeIndex], DirectX::XMMatrixIdentity());
		DirectX::XMStoreFloat4x4(&ShadowCascadeProjectionMatricesStorage[CascadeIndex], DirectX::XMMatrixIdentity());
		DirectX::XMStoreFloat4x4(&ShadowCascadeViewProjectionMatricesStorage[CascadeIndex], DirectX::XMMatrixIdentity());
	}
}

bool DeferredRenderer::BeginShadowCascadePass(ID3D11DeviceContext* DeviceContext, int CascadeIndex)
{
	if (
		DeviceContext == nullptr ||
		CascadeIndex < 0 ||
		CascadeIndex >= ShadowCascadeCount ||
		ShadowDepthDSVs[CascadeIndex] == nullptr)
	{
		return false;
	}

	DeviceContext->OMSetRenderTargets(0, nullptr, ShadowDepthDSVs[CascadeIndex]);
	DeviceContext->ClearDepthStencilView(ShadowDepthDSVs[CascadeIndex], D3D11_CLEAR_DEPTH, 1.0f, 0);

	D3D11_VIEWPORT ShadowViewport = {};
	ShadowViewport.TopLeftX = 0.0f;
	ShadowViewport.TopLeftY = 0.0f;
	ShadowViewport.Width = static_cast<float>(ShadowMapResolution);
	ShadowViewport.Height = static_cast<float>(ShadowMapResolution);
	ShadowViewport.MinDepth = 0.0f;
	ShadowViewport.MaxDepth = 1.0f;
	DeviceContext->RSSetViewports(1, &ShadowViewport);
	DeviceContext->RSSetState(ShadowRasterizerState);
	return true;
}

void DeferredRenderer::EndShadowPass(ID3D11DeviceContext* DeviceContext)
{
	if (DeviceContext == nullptr)
	{
		return;
	}

	DeviceContext->RSSetState(nullptr);
	DeviceContext->OMSetRenderTargets(0, nullptr, nullptr);
}

int DeferredRenderer::GetShadowCascadeCount() const
{
	return ShadowCascadeCountSetting;
}

void DeferredRenderer::SetShadowCascadeSettings(int NewShadowCascadeCount, float NewShadowMaximumDistance)
{
	ShadowCascadeCountSetting = (std::max)(1, (std::min)(NewShadowCascadeCount, ShadowCascadeCount));
	ShadowMaximumDistanceSetting = (std::max)(10.0f, NewShadowMaximumDistance);
}

int DeferredRenderer::GetShadowCascadeCountSetting() const
{
	return ShadowCascadeCountSetting;
}

float DeferredRenderer::GetShadowMaximumDistanceSetting() const
{
	return ShadowMaximumDistanceSetting;
}

DirectX::XMMATRIX DeferredRenderer::GetShadowCascadeViewMatrix(int CascadeIndex) const
{
	if (CascadeIndex < 0 || CascadeIndex >= ShadowCascadeCount)
	{
		return DirectX::XMMatrixIdentity();
	}

	return DirectX::XMLoadFloat4x4(&ShadowCascadeViewMatricesStorage[CascadeIndex]);
}

DirectX::XMMATRIX DeferredRenderer::GetShadowCascadeProjectionMatrix(int CascadeIndex) const
{
	if (CascadeIndex < 0 || CascadeIndex >= ShadowCascadeCount)
	{
		return DirectX::XMMatrixIdentity();
	}

	return DirectX::XMLoadFloat4x4(&ShadowCascadeProjectionMatricesStorage[CascadeIndex]);
}

ID3D11DepthStencilView* DeferredRenderer::GetDepthStencilView() const
{
	return GBufferDepthDSV;
}

ID3D11ShaderResourceView* DeferredRenderer::GetGBufferDepthShaderResourceView() const
{
	return GBufferDepthSRV;
}

ID3D11Buffer* DeferredRenderer::GetShadowVolumeTransformConstantBuffer() const
{
	return ShadowVolumeTransformConstantBuffer;
}

void DeferredRenderer::ReleaseTargets()
{
	if (GBufferAlbedoRTV != nullptr)
	{
		GBufferAlbedoRTV->Release();
		GBufferAlbedoRTV = nullptr;
	}

	if (GBufferNormalRTV != nullptr)
	{
		GBufferNormalRTV->Release();
		GBufferNormalRTV = nullptr;
	}

	if (GBufferMaterialRTV != nullptr)
	{
		GBufferMaterialRTV->Release();
		GBufferMaterialRTV = nullptr;
	}

	if (GBufferAlbedoSRV != nullptr)
	{
		GBufferAlbedoSRV->Release();
		GBufferAlbedoSRV = nullptr;
	}

	if (GBufferNormalSRV != nullptr)
	{
		GBufferNormalSRV->Release();
		GBufferNormalSRV = nullptr;
	}

	if (GBufferMaterialSRV != nullptr)
	{
		GBufferMaterialSRV->Release();
		GBufferMaterialSRV = nullptr;
	}

	if (GBufferDepthSRV != nullptr)
	{
		GBufferDepthSRV->Release();
		GBufferDepthSRV = nullptr;
	}

	if (GBufferDepthDSV != nullptr)
	{
		GBufferDepthDSV->Release();
		GBufferDepthDSV = nullptr;
	}

	if (GBufferAlbedoTexture != nullptr)
	{
		GBufferAlbedoTexture->Release();
		GBufferAlbedoTexture = nullptr;
	}

	if (GBufferNormalTexture != nullptr)
	{
		GBufferNormalTexture->Release();
		GBufferNormalTexture = nullptr;
	}

	if (GBufferMaterialTexture != nullptr)
	{
		GBufferMaterialTexture->Release();
		GBufferMaterialTexture = nullptr;
	}

	if (GBufferDepthTexture != nullptr)
	{
		GBufferDepthTexture->Release();
		GBufferDepthTexture = nullptr;
	}
}

void DeferredRenderer::ReleaseShadowResources()
{
	if (ShadowDepthSRV != nullptr)
	{
		ShadowDepthSRV->Release();
		ShadowDepthSRV = nullptr;
	}

	for (int CascadeIndex = 0; CascadeIndex < ShadowCascadeCount; ++CascadeIndex)
	{
		if (ShadowDepthDSVs[CascadeIndex] != nullptr)
		{
			ShadowDepthDSVs[CascadeIndex]->Release();
			ShadowDepthDSVs[CascadeIndex] = nullptr;
		}
	}

	if (ShadowDepthTextureArray != nullptr)
	{
		ShadowDepthTextureArray->Release();
		ShadowDepthTextureArray = nullptr;
	}
}

bool DeferredRenderer::CompileShader(ID3D11Device* Device, const std::string& Path, const char* EntryPoint, const char* Model, ID3DBlob** ByteCode, ID3D11DeviceChild** ShaderObject)
{
	if (Device == nullptr || ByteCode == nullptr || ShaderObject == nullptr)
	{
		return false;
	}

	std::wstring ShaderFilePath(Path.begin(), Path.end());
	ID3DBlob* ErrorCode = nullptr;
	HRESULT Result = D3DCompileFromFile(
		ShaderFilePath.c_str(),
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		EntryPoint,
		Model,
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		ByteCode,
		&ErrorCode);
	if (FAILED(Result))
	{
		if (ErrorCode != nullptr)
		{
			ErrorCode->Release();
		}
		return false;
	}

	if (std::string(Model).find("vs_") == 0)
	{
		Result = Device->CreateVertexShader((*ByteCode)->GetBufferPointer(), (*ByteCode)->GetBufferSize(), nullptr, reinterpret_cast<ID3D11VertexShader**>(ShaderObject));
	}
	else
	{
		Result = Device->CreatePixelShader((*ByteCode)->GetBufferPointer(), (*ByteCode)->GetBufferSize(), nullptr, reinterpret_cast<ID3D11PixelShader**>(ShaderObject));
	}
	return SUCCEEDED(Result);
}
