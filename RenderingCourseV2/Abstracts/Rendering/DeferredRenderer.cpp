#include "Abstracts/Rendering/DeferredRenderer.h"
#include <d3dcompiler.h>

#pragma comment(lib, "d3dcompiler.lib")

struct DeferredCameraBufferData
{
	DirectX::XMFLOAT4X4 InverseViewProjectionMatrix;
	DirectX::XMFLOAT3 CameraWorldPosition;
	float Padding0;
};

struct DeferredLightBufferData
{
	DirectX::XMFLOAT3 DirectionalLightDirection;
	float DirectionalLightIntensity;
	DirectX::XMFLOAT4 DirectionalLightColor;
	float UseFullBrightnessWithoutLighting;
	DirectX::XMFLOAT3 Padding0;
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
	, CachedWidth(0)
	, CachedHeight(0)
{
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
}

void DeferredRenderer::Shutdown()
{
	ReleaseTargets();

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

void DeferredRenderer::RenderLightingPass(
	ID3D11DeviceContext* DeviceContext,
	ID3D11RenderTargetView* FinalRenderTargetView,
	const DirectX::XMMATRIX& InverseViewProjectionMatrix,
	const DirectX::XMFLOAT3& CameraWorldPosition,
	const DirectX::XMFLOAT3& DirectionalLightDirection,
	const DirectX::XMFLOAT4& DirectionalLightColor,
	float DirectionalLightIntensity,
	float UseFullBrightnessWithoutLighting)
{
	if (DeviceContext == nullptr || FinalRenderTargetView == nullptr || LightingVertexShader == nullptr || LightingPixelShader == nullptr)
	{
		return;
	}

	DeviceContext->OMSetRenderTargets(1, &FinalRenderTargetView, nullptr);

	DeferredCameraBufferData CameraBufferData = {};
	DirectX::XMStoreFloat4x4(&CameraBufferData.InverseViewProjectionMatrix, DirectX::XMMatrixTranspose(InverseViewProjectionMatrix));
	CameraBufferData.CameraWorldPosition = CameraWorldPosition;
	DeviceContext->UpdateSubresource(CameraConstantBuffer, 0, nullptr, &CameraBufferData, 0, 0);

	DeferredLightBufferData LightBufferData = {};
	LightBufferData.DirectionalLightDirection = DirectionalLightDirection;
	LightBufferData.DirectionalLightColor = DirectionalLightColor;
	LightBufferData.DirectionalLightIntensity = DirectionalLightIntensity;
	LightBufferData.UseFullBrightnessWithoutLighting = UseFullBrightnessWithoutLighting;
	DeviceContext->UpdateSubresource(LightConstantBuffer, 0, nullptr, &LightBufferData, 0, 0);

	ID3D11ShaderResourceView* ShaderResourceViews[4] = {
		GBufferAlbedoSRV,
		GBufferNormalSRV,
		GBufferMaterialSRV,
		GBufferDepthSRV
	};
	DeviceContext->PSSetShaderResources(0, 4, ShaderResourceViews);
	DeviceContext->PSSetSamplers(0, 1, &GBufferSampler);
	DeviceContext->VSSetShader(LightingVertexShader, nullptr, 0);
	DeviceContext->PSSetShader(LightingPixelShader, nullptr, 0);
	DeviceContext->VSSetConstantBuffers(0, 1, &CameraConstantBuffer);
	DeviceContext->PSSetConstantBuffers(0, 1, &CameraConstantBuffer);
	DeviceContext->PSSetConstantBuffers(1, 1, &LightConstantBuffer);
	DeviceContext->IASetInputLayout(nullptr);
	DeviceContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	DeviceContext->Draw(3, 0);

	ID3D11ShaderResourceView* NullShaderResources[4] = { nullptr, nullptr, nullptr, nullptr };
	DeviceContext->PSSetShaderResources(0, 4, NullShaderResources);
}

ID3D11DepthStencilView* DeferredRenderer::GetDepthStencilView() const
{
	return GBufferDepthDSV;
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
