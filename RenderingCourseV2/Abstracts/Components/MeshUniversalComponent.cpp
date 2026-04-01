#include "Abstracts/Components/MeshUniversalComponent.h"
#include "Abstracts/Core/Actor.h"
#include "Abstracts/Core/Game.h"
#include "Abstracts/Rendering/RenderProxy/MeshUniversalDeferredRendererProxyObject.h"
#include "Abstracts/Rendering/RenderProxy/MeshUniversalForwardRendererProxyObject.h"
#include "Abstracts/Resources/MeshResourceBindingHelper.h"
#include "Abstracts/Resources/ModelResource.h"
#include "Abstracts/Resources/ResourceManager.h"
#include "Abstracts/Subsystems/SceneViewportSubsystem.h"
#include <d3dcompiler.h>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <map>
#include <memory>
#include <utility>

#pragma comment(lib, "d3dcompiler.lib")

MeshUniversalComponent::MeshUniversalComponent()
	: RenderingComponent()
	, Layout(nullptr)
	, VertexShader(nullptr)
	, VertexShaderByteCode(nullptr)
	, PixelShader(nullptr)
	, PixelShaderByteCode(nullptr)
	, DeferredVertexShader(nullptr)
	, DeferredVertexShaderByteCode(nullptr)
	, DeferredPixelShader(nullptr)
	, DeferredPixelShaderByteCode(nullptr)
	, TransformConstantBuffer(nullptr)
	, LightConstantBuffer(nullptr)
	, MaterialConstantBuffer(nullptr)
	, VertexBuffer(nullptr)
	, IndexBuffer(nullptr)
	, RasterState(nullptr)
	, DefaultSamplerState(nullptr)
	, IndexCount(0)
	, UseOrthographicProjection(false)
	, OrthographicProjectionWidth(4.5f)
	, OrthographicProjectionHeight(2.6f)
	, ShadowVolumeVertexBuffer(nullptr)
	, ShadowVolumeIndexBuffer(nullptr)
	, ShadowVolumeIndexCount(0)
	, CachedShadowVolumeLightWorldPosition(0.0f, 0.0f, 0.0f)
	, ShadowVolumeGeometryValid(false)
{
	Vertices = {
		MeshUniversalVertex{
			DirectX::XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f),
			DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f),
			DirectX::XMFLOAT3(0.0f, 0.0f, -1.0f),
			DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f),
			DirectX::XMFLOAT2(1.0f, 0.0f) },
		MeshUniversalVertex{
			DirectX::XMFLOAT4(-0.5f, -0.5f, 0.5f, 1.0f),
			DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f),
			DirectX::XMFLOAT3(0.0f, 0.0f, -1.0f),
			DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f),
			DirectX::XMFLOAT2(0.0f, 1.0f) },
		MeshUniversalVertex{
			DirectX::XMFLOAT4(0.5f, -0.5f, 0.5f, 1.0f),
			DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f),
			DirectX::XMFLOAT3(0.0f, 0.0f, -1.0f),
			DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f),
			DirectX::XMFLOAT2(1.0f, 1.0f) },
		MeshUniversalVertex{
			DirectX::XMFLOAT4(-0.5f, 0.5f, 0.5f, 1.0f),
			DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
			DirectX::XMFLOAT3(0.0f, 0.0f, -1.0f),
			DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f),
			DirectX::XMFLOAT2(0.0f, 0.0f) }
	};

	Indices = { 0, 1, 2, 1, 0, 3 };

	SetForwardRendererProxyObject(std::make_unique<MeshUniversalForwardRendererProxyObject>(this));
	SetDeferredRendererProxyObject(std::make_unique<MeshUniversalDeferredRendererProxyObject>(this));
}

MeshUniversalComponent::~MeshUniversalComponent()
{
	Shutdown();
}

void MeshUniversalComponent::Initialize()
{
	RenderingComponent::Initialize();

	Game* OwningGame = GetOwningGame();
	if (OwningGame == nullptr)
	{
		return;
	}

	SceneViewportSubsystem* SceneViewport = OwningGame->GetSubsystem<SceneViewportSubsystem>();
	if (SceneViewport == nullptr)
	{
		return;
	}

	ID3D11Device* Device = SceneViewport->GetDevice();
	if (Device == nullptr)
	{
		return;
	}

	if (!InitializeShaderProgram(Device, SceneViewport))
	{
		return;
	}

	InitializeRenderResources(Device, SceneViewport);
}

bool MeshUniversalComponent::InitializeShaderProgram(ID3D11Device* Device, SceneViewportSubsystem* SceneViewport)
{
	(void)SceneViewport;

	if (VertexShaderName.empty())
	{
		VertexShaderName = "./Shaders/Abstracts/MeshUniversalDefault.hlsl";
	}

	if (PixelShaderName.empty())
	{
		PixelShaderName = "./Shaders/Abstracts/MeshUniversalDefault.hlsl";
	}

	if (!CompileShaderFromFile(VertexShaderName, "VSMain", "vs_5_0", &VertexShaderByteCode))
	{
		return false;
	}

	if (!CompileShaderFromFile(PixelShaderName, "PSMain", "ps_5_0", &PixelShaderByteCode))
	{
		return false;
	}

	Device->CreateVertexShader(
		VertexShaderByteCode->GetBufferPointer(),
		VertexShaderByteCode->GetBufferSize(),
		nullptr,
		&VertexShader);

	Device->CreatePixelShader(
		PixelShaderByteCode->GetBufferPointer(),
		PixelShaderByteCode->GetBufferSize(),
		nullptr,
		&PixelShader);

	CompileShaderFromFile(DeferredVertexShaderName, "VSMain", "vs_5_0", &DeferredVertexShaderByteCode);
	CompileShaderFromFile(DeferredPixelShaderName, "PSMain", "ps_5_0", &DeferredPixelShaderByteCode);

	if (DeferredVertexShaderByteCode != nullptr)
	{
		Device->CreateVertexShader(
			DeferredVertexShaderByteCode->GetBufferPointer(),
			DeferredVertexShaderByteCode->GetBufferSize(),
			nullptr,
			&DeferredVertexShader);
	}

	if (DeferredPixelShaderByteCode != nullptr)
	{
		Device->CreatePixelShader(
			DeferredPixelShaderByteCode->GetBufferPointer(),
			DeferredPixelShaderByteCode->GetBufferSize(),
			nullptr,
			&DeferredPixelShader);
	}

	D3D11_INPUT_ELEMENT_DESC InputElements[] = {
		D3D11_INPUT_ELEMENT_DESC{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		D3D11_INPUT_ELEMENT_DESC{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		D3D11_INPUT_ELEMENT_DESC{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		D3D11_INPUT_ELEMENT_DESC{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		D3D11_INPUT_ELEMENT_DESC{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	HRESULT Result = Device->CreateInputLayout(
		InputElements,
		5,
		VertexShaderByteCode->GetBufferPointer(),
		VertexShaderByteCode->GetBufferSize(),
		&Layout);
	return SUCCEEDED(Result);
}

bool MeshUniversalComponent::InitializeRenderResources(ID3D11Device* Device, SceneViewportSubsystem* SceneViewport)
{
	(void)SceneViewport;

	ResourceManager* Resources = GetOwningGame() != nullptr ? GetOwningGame()->GetResourceManager() : nullptr;
	if (!ModelMeshPath.empty() && Resources != nullptr)
	{
		std::shared_ptr<ModelResource> LoadedModelResource = Resources->LoadModelResource(ModelMeshPath);
		if (LoadedModelResource && !LoadedModelResource->Vertices.empty() && !LoadedModelResource->Indices.empty())
		{
			Vertices.clear();
			Indices.clear();
			Vertices.reserve(LoadedModelResource->Vertices.size());
			Indices = LoadedModelResource->Indices;

			for (const ModelResourceVertex& ExistingVertex : LoadedModelResource->Vertices)
			{
				MeshUniversalVertex NewVertex = {};
				NewVertex.Position = ExistingVertex.Position;
				NewVertex.Color = ExistingVertex.Color;
				NewVertex.Normal = ExistingVertex.Normal;
				NewVertex.Tangent = ExistingVertex.Tangent;
				NewVertex.TextureCoordinates = ExistingVertex.TextureCoordinates;
				Vertices.push_back(NewVertex);
			}
		}
	}

	if (Vertices.empty() || Indices.empty())
	{
		return false;
	}

	D3D11_BUFFER_DESC VertexBufferDescription = {};
	VertexBufferDescription.Usage = D3D11_USAGE_DEFAULT;
	VertexBufferDescription.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	VertexBufferDescription.ByteWidth = static_cast<UINT>(sizeof(MeshUniversalVertex) * Vertices.size());

	D3D11_SUBRESOURCE_DATA VertexData = {};
	VertexData.pSysMem = Vertices.data();
	Device->CreateBuffer(&VertexBufferDescription, &VertexData, &VertexBuffer);

	IndexCount = static_cast<UINT>(Indices.size());

	D3D11_BUFFER_DESC IndexBufferDescription = {};
	IndexBufferDescription.Usage = D3D11_USAGE_DEFAULT;
	IndexBufferDescription.BindFlags = D3D11_BIND_INDEX_BUFFER;
	IndexBufferDescription.ByteWidth = static_cast<UINT>(sizeof(unsigned int) * Indices.size());

	D3D11_SUBRESOURCE_DATA IndexData = {};
	IndexData.pSysMem = Indices.data();
	Device->CreateBuffer(&IndexBufferDescription, &IndexData, &IndexBuffer);

	D3D11_BUFFER_DESC TransformBufferDescription = {};
	TransformBufferDescription.Usage = D3D11_USAGE_DEFAULT;
	TransformBufferDescription.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	TransformBufferDescription.ByteWidth = static_cast<UINT>(sizeof(MeshUniversalTransformBufferData));
	Device->CreateBuffer(&TransformBufferDescription, nullptr, &TransformConstantBuffer);

	D3D11_BUFFER_DESC LightBufferDescription = {};
	LightBufferDescription.Usage = D3D11_USAGE_DEFAULT;
	LightBufferDescription.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	LightBufferDescription.ByteWidth = static_cast<UINT>(sizeof(MeshUniversalLightBufferData));
	Device->CreateBuffer(&LightBufferDescription, nullptr, &LightConstantBuffer);

	D3D11_BUFFER_DESC MaterialBufferDescription = {};
	MaterialBufferDescription.Usage = D3D11_USAGE_DEFAULT;
	MaterialBufferDescription.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	MaterialBufferDescription.ByteWidth = static_cast<UINT>(sizeof(MeshUniversalMaterialBufferData));
	Device->CreateBuffer(&MaterialBufferDescription, nullptr, &MaterialConstantBuffer);

	D3D11_SAMPLER_DESC SamplerDescription = {};
	SamplerDescription.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	SamplerDescription.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	SamplerDescription.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	SamplerDescription.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	SamplerDescription.ComparisonFunc = D3D11_COMPARISON_NEVER;
	SamplerDescription.MinLOD = 0.0f;
	SamplerDescription.MaxLOD = D3D11_FLOAT32_MAX;
	Device->CreateSamplerState(&SamplerDescription, &DefaultSamplerState);

	CD3D11_RASTERIZER_DESC RasterizerDescription = {};
	RasterizerDescription.CullMode = D3D11_CULL_NONE;
	RasterizerDescription.FillMode = D3D11_FILL_SOLID;
	Device->CreateRasterizerState(&RasterizerDescription, &RasterState);

	AlbedoTexture = MeshResourceBindingHelper::LoadTexture(Resources, Device, AlbedoTexturePath);
	NormalTexture = MeshResourceBindingHelper::LoadTexture(Resources, Device, NormalTexturePath);
	SpecularTexture = MeshResourceBindingHelper::LoadTexture(Resources, Device, SpecularTexturePath);
	EmissiveTexture = MeshResourceBindingHelper::LoadTexture(Resources, Device, EmissiveTexturePath);
	return true;
}

void MeshUniversalComponent::BindMaterialResources(ID3D11DeviceContext* DeviceContext)
{
	ID3D11ShaderResourceView* ShaderResourceViews[4] = {
		AlbedoTexture.Get(),
		NormalTexture.Get(),
		SpecularTexture.Get(),
		EmissiveTexture.Get()
	};
	DeviceContext->PSSetShaderResources(0, 4, ShaderResourceViews);
	DeviceContext->PSSetSamplers(0, 1, &DefaultSamplerState);
}

void MeshUniversalComponent::Update(float DeltaTime)
{
	RenderingComponent::Update(DeltaTime);
}

void MeshUniversalComponent::SetUseOrthographicProjection(bool NewUseOrthographicProjection)
{
	UseOrthographicProjection = NewUseOrthographicProjection;
}

bool MeshUniversalComponent::GetUseOrthographicProjection() const
{
	return UseOrthographicProjection;
}

void MeshUniversalComponent::SetOrthographicProjectionSize(float NewOrthographicProjectionWidth, float NewOrthographicProjectionHeight)
{
	if (NewOrthographicProjectionWidth > 0.0f)
	{
		OrthographicProjectionWidth = NewOrthographicProjectionWidth;
	}

	if (NewOrthographicProjectionHeight > 0.0f)
	{
		OrthographicProjectionHeight = NewOrthographicProjectionHeight;
	}
}

bool MeshUniversalComponent::CompileShaderFromFile(
	const std::string& ShaderPath,
	const char* EntryPoint,
	const char* ShaderModel,
	ID3DBlob** OutputByteCode) const
{
	if (ShaderPath.empty() || EntryPoint == nullptr || ShaderModel == nullptr || OutputByteCode == nullptr)
	{
		return false;
	}

	std::wstring ShaderFilePath(ShaderPath.begin(), ShaderPath.end());
	ID3DBlob* ErrorCode = nullptr;
	HRESULT Result = D3DCompileFromFile(
		ShaderFilePath.c_str(),
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		EntryPoint,
		ShaderModel,
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		OutputByteCode,
		&ErrorCode);

	if (FAILED(Result))
	{
		if (ErrorCode != nullptr)
		{
			char* CompileErrors = static_cast<char*>(ErrorCode->GetBufferPointer());
			std::cout << CompileErrors << std::endl;
			ErrorCode->Release();
		}
		return false;
	}

	if (ErrorCode != nullptr)
	{
		ErrorCode->Release();
	}

	return true;
}

void MeshUniversalComponent::ReleaseShaderProgramResources()
{
	if (Layout != nullptr)
	{
		Layout->Release();
		Layout = nullptr;
	}

	if (VertexShader != nullptr)
	{
		VertexShader->Release();
		VertexShader = nullptr;
	}

	if (VertexShaderByteCode != nullptr)
	{
		VertexShaderByteCode->Release();
		VertexShaderByteCode = nullptr;
	}

	if (PixelShader != nullptr)
	{
		PixelShader->Release();
		PixelShader = nullptr;
	}

	if (PixelShaderByteCode != nullptr)
	{
		PixelShaderByteCode->Release();
		PixelShaderByteCode = nullptr;
	}

	if (DeferredVertexShader != nullptr)
	{
		DeferredVertexShader->Release();
		DeferredVertexShader = nullptr;
	}

	if (DeferredVertexShaderByteCode != nullptr)
	{
		DeferredVertexShaderByteCode->Release();
		DeferredVertexShaderByteCode = nullptr;
	}

	if (DeferredPixelShader != nullptr)
	{
		DeferredPixelShader->Release();
		DeferredPixelShader = nullptr;
	}

	if (DeferredPixelShaderByteCode != nullptr)
	{
		DeferredPixelShaderByteCode->Release();
		DeferredPixelShaderByteCode = nullptr;
	}
}

void MeshUniversalComponent::ReleaseShadowVolumeGeometry()
{
	if (ShadowVolumeVertexBuffer != nullptr)
	{
		ShadowVolumeVertexBuffer->Release();
		ShadowVolumeVertexBuffer = nullptr;
	}

	if (ShadowVolumeIndexBuffer != nullptr)
	{
		ShadowVolumeIndexBuffer->Release();
		ShadowVolumeIndexBuffer = nullptr;
	}

	ShadowVolumeIndexCount = 0;
	ShadowVolumeGeometryValid = false;
}

void MeshUniversalComponent::EnsureShadowVolumeGeometryForLight(ID3D11Device* Device, const DirectX::XMFLOAT3& LightWorldPosition)
{
	if (Device == nullptr)
	{
		return;
	}

	const float Epsilon = 0.0001f;
	if (
		ShadowVolumeGeometryValid &&
		std::fabs(CachedShadowVolumeLightWorldPosition.x - LightWorldPosition.x) < Epsilon &&
		std::fabs(CachedShadowVolumeLightWorldPosition.y - LightWorldPosition.y) < Epsilon &&
		std::fabs(CachedShadowVolumeLightWorldPosition.z - LightWorldPosition.z) < Epsilon)
	{
		return;
	}

	ReleaseShadowVolumeGeometry();
	CachedShadowVolumeLightWorldPosition = LightWorldPosition;

	if (Indices.size() < 3 || Vertices.empty())
	{
		return;
	}

	const DirectX::XMMATRIX WorldMatrix = GetWorldTransform().ToMatrix();
	const DirectX::XMVECTOR LightWorldPositionVector = DirectX::XMLoadFloat3(&LightWorldPosition);

	const size_t TriangleCount = Indices.size() / 3;
	std::vector<bool> TriangleFacesLight;
	TriangleFacesLight.resize(TriangleCount);
	std::vector<DirectX::XMFLOAT3> WorldVertexPositions;
	WorldVertexPositions.resize(Vertices.size());
	for (size_t VertexIndex = 0; VertexIndex < Vertices.size(); ++VertexIndex)
	{
		const DirectX::XMVECTOR LocalPosition = DirectX::XMLoadFloat4(&Vertices[VertexIndex].Position);
		const DirectX::XMVECTOR WorldPosition = DirectX::XMVector4Transform(LocalPosition, WorldMatrix);
		DirectX::XMFLOAT3 StoredWorldPosition;
		DirectX::XMStoreFloat3(&StoredWorldPosition, WorldPosition);
		WorldVertexPositions[VertexIndex] = StoredWorldPosition;
	}

	for (size_t TriangleIndex = 0; TriangleIndex < TriangleCount; ++TriangleIndex)
	{
		const uint32_t Index0 = Indices[TriangleIndex * 3 + 0];
		const uint32_t Index1 = Indices[TriangleIndex * 3 + 1];
		const uint32_t Index2 = Indices[TriangleIndex * 3 + 2];
		const DirectX::XMVECTOR Position0 = DirectX::XMLoadFloat3(&WorldVertexPositions[Index0]);
		const DirectX::XMVECTOR Position1 = DirectX::XMLoadFloat3(&WorldVertexPositions[Index1]);
		const DirectX::XMVECTOR Position2 = DirectX::XMLoadFloat3(&WorldVertexPositions[Index2]);
		const DirectX::XMVECTOR Edge0 = DirectX::XMVectorSubtract(Position1, Position0);
		const DirectX::XMVECTOR Edge1 = DirectX::XMVectorSubtract(Position2, Position0);
		DirectX::XMVECTOR Normal = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(Edge0, Edge1));
		const DirectX::XMVECTOR Center = DirectX::XMVectorScale(
			DirectX::XMVectorAdd(DirectX::XMVectorAdd(Position0, Position1), Position2),
			1.0f / 3.0f);
		const DirectX::XMVECTOR ToLight = DirectX::XMVectorSubtract(LightWorldPositionVector, Center);
		const float DotValue = DirectX::XMVectorGetX(DirectX::XMVector3Dot(Normal, DirectX::XMVector3Normalize(ToLight)));
		TriangleFacesLight[TriangleIndex] = DotValue > 0.0f;
	}

	std::map<std::pair<uint32_t, uint32_t>, std::vector<uint32_t>> EdgeToTriangleIndices;
	for (size_t TriangleIndex = 0; TriangleIndex < TriangleCount; ++TriangleIndex)
	{
		const uint32_t Index0 = Indices[TriangleIndex * 3 + 0];
		const uint32_t Index1 = Indices[TriangleIndex * 3 + 1];
		const uint32_t Index2 = Indices[TriangleIndex * 3 + 2];
		const std::pair<uint32_t, uint32_t> Edge0 = Index0 < Index1 ? std::make_pair(Index0, Index1) : std::make_pair(Index1, Index0);
		const std::pair<uint32_t, uint32_t> Edge1 = Index1 < Index2 ? std::make_pair(Index1, Index2) : std::make_pair(Index2, Index1);
		const std::pair<uint32_t, uint32_t> Edge2 = Index2 < Index0 ? std::make_pair(Index2, Index0) : std::make_pair(Index0, Index2);
		EdgeToTriangleIndices[Edge0].push_back(static_cast<uint32_t>(TriangleIndex));
		EdgeToTriangleIndices[Edge1].push_back(static_cast<uint32_t>(TriangleIndex));
		EdgeToTriangleIndices[Edge2].push_back(static_cast<uint32_t>(TriangleIndex));
	}

	std::vector<DirectX::XMFLOAT4> ShadowVolumePositions;
	std::vector<uint32_t> ShadowVolumeIndices;

	const float ShadowVolumeExtrusionDistance = 5000.0f;

	for (size_t TriangleIndex = 0; TriangleIndex < TriangleCount; ++TriangleIndex)
	{
		if (TriangleFacesLight[TriangleIndex])
		{
			continue;
		}

		const uint32_t Index0 = Indices[TriangleIndex * 3 + 0];
		const uint32_t Index1 = Indices[TriangleIndex * 3 + 1];
		const uint32_t Index2 = Indices[TriangleIndex * 3 + 2];
		const uint32_t BaseVertexIndex = static_cast<uint32_t>(ShadowVolumePositions.size());
		ShadowVolumePositions.push_back(DirectX::XMFLOAT4(WorldVertexPositions[Index0].x, WorldVertexPositions[Index0].y, WorldVertexPositions[Index0].z, 1.0f));
		ShadowVolumePositions.push_back(DirectX::XMFLOAT4(WorldVertexPositions[Index1].x, WorldVertexPositions[Index1].y, WorldVertexPositions[Index1].z, 1.0f));
		ShadowVolumePositions.push_back(DirectX::XMFLOAT4(WorldVertexPositions[Index2].x, WorldVertexPositions[Index2].y, WorldVertexPositions[Index2].z, 1.0f));
		ShadowVolumeIndices.push_back(BaseVertexIndex + 0);
		ShadowVolumeIndices.push_back(BaseVertexIndex + 1);
		ShadowVolumeIndices.push_back(BaseVertexIndex + 2);
	}

	for (const auto& EdgeEntry : EdgeToTriangleIndices)
	{
		const std::pair<uint32_t, uint32_t> EdgeKey = EdgeEntry.first;
		const std::vector<uint32_t>& TriangleIndicesForEdge = EdgeEntry.second;
		bool IsSilhouetteEdge = false;
		if (TriangleIndicesForEdge.size() == 2)
		{
			const bool FirstFace = TriangleFacesLight[TriangleIndicesForEdge[0]];
			const bool SecondFace = TriangleFacesLight[TriangleIndicesForEdge[1]];
			IsSilhouetteEdge = (FirstFace != SecondFace);
		}
		else if (TriangleIndicesForEdge.size() == 1)
		{
			IsSilhouetteEdge = TriangleFacesLight[TriangleIndicesForEdge[0]];
		}
		else
		{
			IsSilhouetteEdge = false;
		}

		if (IsSilhouetteEdge == false)
		{
			continue;
		}

		const uint32_t VertexIndex0 = EdgeKey.first;
		const uint32_t VertexIndex1 = EdgeKey.second;
		const DirectX::XMVECTOR Position0 = DirectX::XMLoadFloat3(&WorldVertexPositions[VertexIndex0]);
		const DirectX::XMVECTOR Position1 = DirectX::XMLoadFloat3(&WorldVertexPositions[VertexIndex1]);
		const DirectX::XMVECTOR ExtrudeDirection0 = DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(Position0, LightWorldPositionVector));
		const DirectX::XMVECTOR ExtrudeDirection1 = DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(Position1, LightWorldPositionVector));
		const DirectX::XMVECTOR ExtrudedPosition0 = DirectX::XMVectorAdd(
			Position0,
			DirectX::XMVectorScale(ExtrudeDirection0, ShadowVolumeExtrusionDistance));
		const DirectX::XMVECTOR ExtrudedPosition1 = DirectX::XMVectorAdd(
			Position1,
			DirectX::XMVectorScale(ExtrudeDirection1, ShadowVolumeExtrusionDistance));
		DirectX::XMFLOAT3 StoredExtruded0;
		DirectX::XMFLOAT3 StoredExtruded1;
		DirectX::XMStoreFloat3(&StoredExtruded0, ExtrudedPosition0);
		DirectX::XMStoreFloat3(&StoredExtruded1, ExtrudedPosition1);

		const uint32_t BaseVertexIndex = static_cast<uint32_t>(ShadowVolumePositions.size());
		ShadowVolumePositions.push_back(DirectX::XMFLOAT4(WorldVertexPositions[VertexIndex0].x, WorldVertexPositions[VertexIndex0].y, WorldVertexPositions[VertexIndex0].z, 1.0f));
		ShadowVolumePositions.push_back(DirectX::XMFLOAT4(WorldVertexPositions[VertexIndex1].x, WorldVertexPositions[VertexIndex1].y, WorldVertexPositions[VertexIndex1].z, 1.0f));
		ShadowVolumePositions.push_back(DirectX::XMFLOAT4(StoredExtruded1.x, StoredExtruded1.y, StoredExtruded1.z, 1.0f));
		ShadowVolumePositions.push_back(DirectX::XMFLOAT4(StoredExtruded0.x, StoredExtruded0.y, StoredExtruded0.z, 1.0f));
		ShadowVolumeIndices.push_back(BaseVertexIndex + 0);
		ShadowVolumeIndices.push_back(BaseVertexIndex + 1);
		ShadowVolumeIndices.push_back(BaseVertexIndex + 2);
		ShadowVolumeIndices.push_back(BaseVertexIndex + 0);
		ShadowVolumeIndices.push_back(BaseVertexIndex + 2);
		ShadowVolumeIndices.push_back(BaseVertexIndex + 3);
	}

	if (ShadowVolumeIndices.empty())
	{
		return;
	}

	const UINT VertexBufferByteWidth = static_cast<UINT>(ShadowVolumePositions.size() * sizeof(DirectX::XMFLOAT4));
	const UINT IndexBufferByteWidth = static_cast<UINT>(ShadowVolumeIndices.size() * sizeof(uint32_t));

	D3D11_BUFFER_DESC VertexBufferDescription = {};
	VertexBufferDescription.Usage = D3D11_USAGE_DEFAULT;
	VertexBufferDescription.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	VertexBufferDescription.ByteWidth = VertexBufferByteWidth;

	D3D11_SUBRESOURCE_DATA VertexSubresourceData = {};
	VertexSubresourceData.pSysMem = ShadowVolumePositions.data();

	Device->CreateBuffer(&VertexBufferDescription, &VertexSubresourceData, &ShadowVolumeVertexBuffer);

	D3D11_BUFFER_DESC IndexBufferDescription = {};
	IndexBufferDescription.Usage = D3D11_USAGE_DEFAULT;
	IndexBufferDescription.BindFlags = D3D11_BIND_INDEX_BUFFER;
	IndexBufferDescription.ByteWidth = IndexBufferByteWidth;

	D3D11_SUBRESOURCE_DATA IndexSubresourceData = {};
	IndexSubresourceData.pSysMem = ShadowVolumeIndices.data();

	Device->CreateBuffer(&IndexBufferDescription, &IndexSubresourceData, &ShadowVolumeIndexBuffer);

	ShadowVolumeIndexCount = static_cast<UINT>(ShadowVolumeIndices.size());
	ShadowVolumeGeometryValid = true;
}

void MeshUniversalComponent::ReleaseRenderResources()
{
	ReleaseShadowVolumeGeometry();

	if (TransformConstantBuffer != nullptr)
	{
		TransformConstantBuffer->Release();
		TransformConstantBuffer = nullptr;
	}

	if (LightConstantBuffer != nullptr)
	{
		LightConstantBuffer->Release();
		LightConstantBuffer = nullptr;
	}

	if (MaterialConstantBuffer != nullptr)
	{
		MaterialConstantBuffer->Release();
		MaterialConstantBuffer = nullptr;
	}

	if (VertexBuffer != nullptr)
	{
		VertexBuffer->Release();
		VertexBuffer = nullptr;
	}

	if (IndexBuffer != nullptr)
	{
		IndexBuffer->Release();
		IndexBuffer = nullptr;
	}

	if (RasterState != nullptr)
	{
		RasterState->Release();
		RasterState = nullptr;
	}

	if (DefaultSamplerState != nullptr)
	{
		DefaultSamplerState->Release();
		DefaultSamplerState = nullptr;
	}

	AlbedoTexture.Reset();
	NormalTexture.Reset();
	SpecularTexture.Reset();
	EmissiveTexture.Reset();
}

void MeshUniversalComponent::Shutdown()
{
	if (GetIsInitialized() == false)
	{
		return;
	}

	ReleaseShaderProgramResources();
	ReleaseRenderResources();

	RenderingComponent::Shutdown();
}
