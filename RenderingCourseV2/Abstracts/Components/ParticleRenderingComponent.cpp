#include "Abstracts/Components/ParticleRenderingComponent.h"
#include "Abstracts/Resources/ResourceManager.h"
#include "Abstracts/Rendering/ParticleSimulation/ParticleCollisionObject.h"
#include "Abstracts/Rendering/ParticleSimulation/ParticleDefaultObject.h"
#include "Abstracts/Rendering/ParticleSimulation/ParticleSpawnRateObject.h"
#include "Abstracts/Rendering/ParticleSimulation/ParticleFrictionObject.h"
#include "Abstracts/Rendering/ParticleSimulation/ParticleGravityObject.h"
#include "Abstracts/Rendering/ParticleSimulation/ParticleLinearForceObject.h"
#include "Abstracts/Rendering/ParticleSimulation/ParticlePointForceObject.h"
#include "Abstracts/Core/Game.h"
#include "Abstracts/Rendering/RenderProxy/ParticleDeferredRenderProxyObject.h"
#include "Abstracts/Rendering/RenderProxy/ParticleForwardRenderProxyObject.h"
#include "Abstracts/Subsystems/SceneViewportSubsystem.h"
#include <d3dcompiler.h>
#include <filesystem>
#include <imgui.h>
#include <iostream>
#include <string>
#include <vector>

#pragma comment(lib, "d3dcompiler.lib")

namespace
{
	struct ParticleConstantsBufferData
	{
		UINT ParticleCount;
		DirectX::XMFLOAT3 GravityDirection;
		float DeltaTime;
		float Padding0[3];
	};

	struct ParticleMaterialConstantsBufferData
	{
		DirectX::XMFLOAT4X4 ViewProjectionMatrix;
		DirectX::XMFLOAT3 CameraRightWorld;
		float ParticleSize;
		DirectX::XMFLOAT3 CameraUpWorld;
		float Padding0;
		UINT ParticleDrawCount;
		UINT Padding1[3];
	};

	constexpr UINT ParticleSimulationThreadGroupSize = 256;
}

ParticleRenderingComponent::ParticleRenderingComponent(int NewMaxParticleCount)
	: MaxParticleCount(NewMaxParticleCount > 0 ? NewMaxParticleCount : 4096)
	, LastDeltaTime(0.0f)
	, GravityDirectionSimulation(0.0f, -9.81f, 0.0f)
	, ParticleSizeWorld(0.5f)
	, CachedSceneViewport(nullptr)
	, ParticleStateBuffer(nullptr)
	, ParticleStateUnorderedAccessView(nullptr)
	, ParticleStateShaderResourceView(nullptr)
	, ParticleSimulationConstantsBuffer(nullptr)
	, ParticleMaterialConstantBuffer(nullptr)
	, SceneDepthSamplerState(nullptr)
	, ParticleVertexShader(nullptr)
	, ParticlePixelShader(nullptr)
	, ParticleVertexShaderByteCode(nullptr)
	, ParticlePixelShaderByteCode(nullptr)
	, ParticleAlphaBlendState(nullptr)
	, ParticleDepthStencilState(nullptr)
	, ParticleRasterizerState(nullptr)
	, ParticleSimulationThreadGroupCount(0)
	, ShaderContentRootDirectory()
{
	SetRenderOrder(50);
	SetForwardRendererProxyObject(std::make_unique<ParticleForwardRenderProxyObject>(this));
	SetDeferredRendererProxyObject(std::make_unique<ParticleDeferredRenderProxyObject>(this));
	ParticleSimulationThreadGroupCount = (static_cast<UINT>(MaxParticleCount) + ParticleSimulationThreadGroupSize - 1) / ParticleSimulationThreadGroupSize;
}

ParticleRenderingComponent::~ParticleRenderingComponent()
{
	Shutdown();
}

bool ParticleRenderingComponent::CompileShaderFromFile(
	const std::string& ShaderPath,
	const char* EntryPoint,
	const char* ShaderModel,
	ID3DBlob** OutputByteCode) const
{
	if (ShaderPath.empty() || EntryPoint == nullptr || ShaderModel == nullptr || OutputByteCode == nullptr)
	{
		return false;
	}

	std::filesystem::path AbsoluteShaderPath;
	if (ShaderContentRootDirectory.empty())
	{
		AbsoluteShaderPath = ShaderPath;
	}
	else
	{
		std::string RelativeShaderPath = ShaderPath;
		if (RelativeShaderPath.size() >= 2 && RelativeShaderPath[0] == '.' && RelativeShaderPath[1] == '/')
		{
			RelativeShaderPath = RelativeShaderPath.substr(2);
		}
		AbsoluteShaderPath = std::filesystem::path(ShaderContentRootDirectory) / RelativeShaderPath;
	}

	const std::wstring ShaderFilePath = AbsoluteShaderPath.wstring();
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

bool ParticleRenderingComponent::CreateDrawShaderProgram(ID3D11Device* Device)
{
	ReleaseDrawShaderProgram();

	if (Device == nullptr)
	{
		return false;
	}

	if (!CompileShaderFromFile("./Shaders/ParticleSystem/BaseParticleMaterial.hlsl", "VSMain", "vs_5_0", &ParticleVertexShaderByteCode))
	{
		return false;
	}

	if (!CompileShaderFromFile("./Shaders/ParticleSystem/BaseParticleMaterial.hlsl", "PSMain", "ps_5_0", &ParticlePixelShaderByteCode))
	{
		return false;
	}

	HRESULT VertexShaderResult = Device->CreateVertexShader(
		ParticleVertexShaderByteCode->GetBufferPointer(),
		ParticleVertexShaderByteCode->GetBufferSize(),
		nullptr,
		&ParticleVertexShader);

	HRESULT PixelShaderResult = Device->CreatePixelShader(
		ParticlePixelShaderByteCode->GetBufferPointer(),
		ParticlePixelShaderByteCode->GetBufferSize(),
		nullptr,
		&ParticlePixelShader);

	return SUCCEEDED(VertexShaderResult) && SUCCEEDED(PixelShaderResult);
}

void ParticleRenderingComponent::ReleaseDrawShaderProgram()
{
	if (ParticleVertexShader != nullptr)
	{
		ParticleVertexShader->Release();
		ParticleVertexShader = nullptr;
	}

	if (ParticlePixelShader != nullptr)
	{
		ParticlePixelShader->Release();
		ParticlePixelShader = nullptr;
	}

	if (ParticleVertexShaderByteCode != nullptr)
	{
		ParticleVertexShaderByteCode->Release();
		ParticleVertexShaderByteCode = nullptr;
	}

	if (ParticlePixelShaderByteCode != nullptr)
	{
		ParticlePixelShaderByteCode->Release();
		ParticlePixelShaderByteCode = nullptr;
	}
}

bool ParticleRenderingComponent::CreateParticleSimulationResources(ID3D11Device* Device)
{
	ReleaseParticleSimulationResources();

	if (Device == nullptr)
	{
		return false;
	}

	const int StructureByteStride = static_cast<int>(sizeof(ParticleStructData));
	const int BufferByteCount = MaxParticleCount * StructureByteStride;

	D3D11_BUFFER_DESC BufferDescription = {};
	BufferDescription.ByteWidth = static_cast<UINT>(BufferByteCount);
	BufferDescription.Usage = D3D11_USAGE_DEFAULT;
	BufferDescription.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	BufferDescription.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	BufferDescription.StructureByteStride = StructureByteStride;

	std::vector<ParticleStructData> InitialParticles(static_cast<size_t>(MaxParticleCount));
	D3D11_SUBRESOURCE_DATA InitialBufferData = {};
	InitialBufferData.pSysMem = InitialParticles.data();
	HRESULT CreateBufferResult = Device->CreateBuffer(&BufferDescription, &InitialBufferData, &ParticleStateBuffer);
	if (FAILED(CreateBufferResult))
	{
		return false;
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC ShaderResourceViewDescription = {};
	ShaderResourceViewDescription.Format = DXGI_FORMAT_UNKNOWN;
	ShaderResourceViewDescription.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	ShaderResourceViewDescription.Buffer.FirstElement = 0;
	ShaderResourceViewDescription.Buffer.NumElements = static_cast<UINT>(MaxParticleCount);

	HRESULT CreateShaderResourceViewResult = Device->CreateShaderResourceView(
		ParticleStateBuffer,
		&ShaderResourceViewDescription,
		&ParticleStateShaderResourceView);
	if (FAILED(CreateShaderResourceViewResult))
	{
		return false;
	}

	D3D11_UNORDERED_ACCESS_VIEW_DESC UnorderedAccessViewDescription = {};
	UnorderedAccessViewDescription.Format = DXGI_FORMAT_UNKNOWN;
	UnorderedAccessViewDescription.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	UnorderedAccessViewDescription.Buffer.FirstElement = 0;
	UnorderedAccessViewDescription.Buffer.NumElements = static_cast<UINT>(MaxParticleCount);
	UnorderedAccessViewDescription.Buffer.Flags = 0;

	HRESULT CreateUnorderedAccessViewResult = Device->CreateUnorderedAccessView(
		ParticleStateBuffer,
		&UnorderedAccessViewDescription,
		&ParticleStateUnorderedAccessView);
	if (FAILED(CreateUnorderedAccessViewResult))
	{
		return false;
	}

	D3D11_BUFFER_DESC ConstantBufferDescription = {};
	ConstantBufferDescription.ByteWidth = sizeof(ParticleConstantsBufferData);
	if (ConstantBufferDescription.ByteWidth % 16 != 0)
	{
		ConstantBufferDescription.ByteWidth = (ConstantBufferDescription.ByteWidth + 15) & ~15u;
	}
	ConstantBufferDescription.Usage = D3D11_USAGE_DYNAMIC;
	ConstantBufferDescription.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	ConstantBufferDescription.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	HRESULT CreateConstantBufferResult = Device->CreateBuffer(&ConstantBufferDescription, nullptr, &ParticleSimulationConstantsBuffer);
	if (FAILED(CreateConstantBufferResult))
	{
		return false;
	}

	D3D11_BUFFER_DESC MaterialConstantBufferDescription = {};
	MaterialConstantBufferDescription.ByteWidth = sizeof(ParticleMaterialConstantsBufferData);
	if (MaterialConstantBufferDescription.ByteWidth % 16 != 0)
	{
		MaterialConstantBufferDescription.ByteWidth = (MaterialConstantBufferDescription.ByteWidth + 15) & ~15u;
	}
	MaterialConstantBufferDescription.Usage = D3D11_USAGE_DYNAMIC;
	MaterialConstantBufferDescription.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	MaterialConstantBufferDescription.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	HRESULT CreateMaterialConstantBufferResult = Device->CreateBuffer(&MaterialConstantBufferDescription, nullptr, &ParticleMaterialConstantBuffer);
	if (FAILED(CreateMaterialConstantBufferResult))
	{
		return false;
	}

	D3D11_SAMPLER_DESC SamplerDescription = {};
	SamplerDescription.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	SamplerDescription.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	SamplerDescription.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	SamplerDescription.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	SamplerDescription.ComparisonFunc = D3D11_COMPARISON_NEVER;
	SamplerDescription.MinLOD = 0.0f;
	SamplerDescription.MaxLOD = D3D11_FLOAT32_MAX;

	HRESULT CreateSamplerResult = Device->CreateSamplerState(&SamplerDescription, &SceneDepthSamplerState);
	if (FAILED(CreateSamplerResult))
	{
		return false;
	}

	D3D11_BLEND_DESC BlendDescription = {};
	BlendDescription.RenderTarget[0].BlendEnable = TRUE;
	BlendDescription.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	BlendDescription.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	BlendDescription.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	BlendDescription.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	BlendDescription.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	BlendDescription.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	BlendDescription.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	HRESULT CreateBlendResult = Device->CreateBlendState(&BlendDescription, &ParticleAlphaBlendState);
	if (FAILED(CreateBlendResult))
	{
		return false;
	}

	D3D11_DEPTH_STENCIL_DESC DepthStencilDescription = {};
	DepthStencilDescription.DepthEnable = TRUE;
	DepthStencilDescription.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	DepthStencilDescription.DepthFunc = D3D11_COMPARISON_LESS;

	HRESULT CreateDepthStencilResult = Device->CreateDepthStencilState(&DepthStencilDescription, &ParticleDepthStencilState);
	if (FAILED(CreateDepthStencilResult))
	{
		return false;
	}

	D3D11_RASTERIZER_DESC RasterizerDescription = {};
	RasterizerDescription.FillMode = D3D11_FILL_SOLID;
	RasterizerDescription.CullMode = D3D11_CULL_NONE;
	RasterizerDescription.FrontCounterClockwise = FALSE;
	RasterizerDescription.DepthBias = 0;
	RasterizerDescription.DepthBiasClamp = 0.0f;
	RasterizerDescription.SlopeScaledDepthBias = 0.0f;
	RasterizerDescription.DepthClipEnable = TRUE;
	RasterizerDescription.MultisampleEnable = FALSE;
	RasterizerDescription.AntialiasedLineEnable = FALSE;

	HRESULT CreateRasterizerResult = Device->CreateRasterizerState(&RasterizerDescription, &ParticleRasterizerState);
	if (FAILED(CreateRasterizerResult))
	{
		return false;
	}

	return CreateDrawShaderProgram(Device);
}

void ParticleRenderingComponent::ReleaseParticleSimulationResources()
{
	ReleaseDrawShaderProgram();

	if (ParticleRasterizerState != nullptr)
	{
		ParticleRasterizerState->Release();
		ParticleRasterizerState = nullptr;
	}

	if (ParticleDepthStencilState != nullptr)
	{
		ParticleDepthStencilState->Release();
		ParticleDepthStencilState = nullptr;
	}

	if (ParticleAlphaBlendState != nullptr)
	{
		ParticleAlphaBlendState->Release();
		ParticleAlphaBlendState = nullptr;
	}

	if (SceneDepthSamplerState != nullptr)
	{
		SceneDepthSamplerState->Release();
		SceneDepthSamplerState = nullptr;
	}

	if (ParticleMaterialConstantBuffer != nullptr)
	{
		ParticleMaterialConstantBuffer->Release();
		ParticleMaterialConstantBuffer = nullptr;
	}

	if (ParticleSimulationConstantsBuffer != nullptr)
	{
		ParticleSimulationConstantsBuffer->Release();
		ParticleSimulationConstantsBuffer = nullptr;
	}

	if (ParticleStateUnorderedAccessView != nullptr)
	{
		ParticleStateUnorderedAccessView->Release();
		ParticleStateUnorderedAccessView = nullptr;
	}

	if (ParticleStateShaderResourceView != nullptr)
	{
		ParticleStateShaderResourceView->Release();
		ParticleStateShaderResourceView = nullptr;
	}

	if (ParticleStateBuffer != nullptr)
	{
		ParticleStateBuffer->Release();
		ParticleStateBuffer = nullptr;
	}
}

void ParticleRenderingComponent::BuildDefaultSimulationPipeline(ID3D11Device* Device)
{
	SimulationStages.clear();
	SimulationStages.push_back(std::make_unique<ParticleSpawnRateObject>());
	SimulationStages.push_back(std::make_unique<ParticleGravityObject>());
	SimulationStages.push_back(std::make_unique<ParticleLinearForceObject>());
	SimulationStages.push_back(std::make_unique<ParticleFrictionObject>());
	SimulationStages.push_back(std::make_unique<ParticlePointForceObject>());
	SimulationStages.push_back(std::make_unique<ParticleDefaultObject>());
	SimulationStages.push_back(std::make_unique<ParticleCollisionObject>());

	for (std::unique_ptr<ParticleSimulationObject>& ExistingStage : SimulationStages)
	{
		if (ExistingStage == nullptr)
		{
			continue;
		}
		ExistingStage->SetShaderContentRootDirectory(ShaderContentRootDirectory);
		if (ExistingStage->CreateGpuResources(Device) == false)
		{
			std::cout << "Particle simulation stage failed to create GPU resources." << std::endl;
		}
		ExistingStage->Initialize();
	}
}

void ParticleRenderingComponent::Initialize()
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

	CachedSceneViewport = SceneViewport;
	ResourceManager* Resources = OwningGame->GetResourceManager();
	if (Resources != nullptr)
	{
		ShaderContentRootDirectory = Resources->GetProjectRootPath();
	}
	ID3D11Device* Device = SceneViewport->GetDevice();
	if (Device == nullptr)
	{
		return;
	}

	if (CreateParticleSimulationResources(Device) == false)
	{
		return;
	}

	BuildDefaultSimulationPipeline(Device);
}

void ParticleRenderingComponent::UpdateParticleSimulationConstantsBuffer()
{
	if (CachedSceneViewport == nullptr || ParticleSimulationConstantsBuffer == nullptr)
	{
		return;
	}

	ID3D11DeviceContext* DeviceContext = CachedSceneViewport->GetDeviceContext();
	if (DeviceContext == nullptr)
	{
		return;
	}

	ParticleConstantsBufferData BufferData = {};
	BufferData.ParticleCount = static_cast<UINT>(MaxParticleCount);
	BufferData.GravityDirection = GravityDirectionSimulation;
	BufferData.DeltaTime = LastDeltaTime;

	D3D11_MAPPED_SUBRESOURCE MappedResource = {};
	HRESULT MapResult = DeviceContext->Map(ParticleSimulationConstantsBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource);
	if (SUCCEEDED(MapResult))
	{
		memcpy(MappedResource.pData, &BufferData, sizeof(BufferData));
		DeviceContext->Unmap(ParticleSimulationConstantsBuffer, 0);
	}
}

void ParticleRenderingComponent::Update(float DeltaTime)
{
	RenderingComponent::Update(DeltaTime);

	if (CachedSceneViewport == nullptr)
	{
		return;
	}

	ID3D11DeviceContext* DeviceContext = CachedSceneViewport->GetDeviceContext();
	if (DeviceContext == nullptr)
	{
		return;
	}

	LastDeltaTime = DeltaTime;
	UpdateParticleSimulationConstantsBuffer();

	for (std::unique_ptr<ParticleSimulationObject>& ExistingStage : SimulationStages)
	{
		if (ExistingStage != nullptr)
		{
			ExistingStage->Dispatch(this);
		}
	}

	UnbindParticleSimulationCompute();
}

void ParticleRenderingComponent::Shutdown()
{
	for (std::unique_ptr<ParticleSimulationObject>& ExistingStage : SimulationStages)
	{
		if (ExistingStage != nullptr)
		{
			ExistingStage->Shutdown();
		}
	}
	SimulationStages.clear();

	ReleaseParticleSimulationResources();
	CachedSceneViewport = nullptr;

	RenderingComponent::Shutdown();
}

void ParticleRenderingComponent::DrawDearImGuiParticlePanels()
{
	if (CachedSceneViewport == nullptr || CachedSceneViewport->GetIsDearImGuiInitialized() == false)
	{
		return;
	}

	ImGui::Begin("ParticleSimulationStages");
	for (size_t StageIndex = 0; StageIndex < SimulationStages.size(); ++StageIndex)
	{
		ParticleSimulationObject* ExistingStage = SimulationStages[StageIndex].get();
		if (ExistingStage == nullptr)
		{
			continue;
		}

		ImGui::PushID(static_cast<int>(StageIndex));
		if (ImGui::CollapsingHeader(ExistingStage->GetStageDisplayName()))
		{
			ExistingStage->DrawDearImGui(this);
		}
		ImGui::PopID();
	}
	ImGui::End();
}

ID3D11DeviceContext* ParticleRenderingComponent::GetDeviceContext() const
{
	if (CachedSceneViewport == nullptr)
	{
		return nullptr;
	}

	return CachedSceneViewport->GetDeviceContext();
}

SceneViewportSubsystem* ParticleRenderingComponent::GetSceneViewportSubsystem() const
{
	return CachedSceneViewport;
}

ID3D11SamplerState* ParticleRenderingComponent::GetSceneDepthSamplerState() const
{
	return SceneDepthSamplerState;
}

void ParticleRenderingComponent::BindParticleSimulationCommonComputeState(ID3D11ComputeShader* ComputeShader)
{
	if (CachedSceneViewport == nullptr)
	{
		return;
	}

	ID3D11DeviceContext* DeviceContext = CachedSceneViewport->GetDeviceContext();
	if (DeviceContext == nullptr || ComputeShader == nullptr)
	{
		return;
	}

	DeviceContext->CSSetShader(ComputeShader, nullptr, 0);

	if (ParticleSimulationConstantsBuffer != nullptr)
	{
		ID3D11Buffer* ConstantBuffers[] = { ParticleSimulationConstantsBuffer };
		DeviceContext->CSSetConstantBuffers(0, 1, ConstantBuffers);
	}

	if (ParticleStateUnorderedAccessView != nullptr)
	{
		ID3D11UnorderedAccessView* UnorderedAccessViews[] = { ParticleStateUnorderedAccessView };
		DeviceContext->CSSetUnorderedAccessViews(0, 1, UnorderedAccessViews, nullptr);
	}
}

void ParticleRenderingComponent::DispatchParticleSimulationThreadGroups()
{
	if (CachedSceneViewport == nullptr)
	{
		return;
	}

	ID3D11DeviceContext* DeviceContext = CachedSceneViewport->GetDeviceContext();
	if (DeviceContext == nullptr)
	{
		return;
	}

	DeviceContext->Dispatch(ParticleSimulationThreadGroupCount, 1, 1);
}

void ParticleRenderingComponent::DispatchParticleSpawnThreadGroups(UINT TotalSpawnThreads)
{
	if (CachedSceneViewport == nullptr)
	{
		return;
	}

	ID3D11DeviceContext* DeviceContext = CachedSceneViewport->GetDeviceContext();
	if (DeviceContext == nullptr)
	{
		return;
	}

	if (TotalSpawnThreads == 0)
	{
		return;
	}

	const UINT GroupCount = (TotalSpawnThreads + ParticleSimulationThreadGroupSize - 1) / ParticleSimulationThreadGroupSize;
	DeviceContext->Dispatch(GroupCount, 1, 1);
}

float ParticleRenderingComponent::GetLastSimulationDeltaTime() const
{
	return LastDeltaTime;
}

void ParticleRenderingComponent::UnbindParticleSimulationCompute()
{
	if (CachedSceneViewport == nullptr)
	{
		return;
	}

	ID3D11DeviceContext* DeviceContext = CachedSceneViewport->GetDeviceContext();
	if (DeviceContext == nullptr)
	{
		return;
	}

	ID3D11ComputeShader* NullComputeShader = nullptr;
	DeviceContext->CSSetShader(NullComputeShader, nullptr, 0);

	ID3D11UnorderedAccessView* NullUnorderedAccessViews[] = { nullptr };
	DeviceContext->CSSetUnorderedAccessViews(0, 1, NullUnorderedAccessViews, nullptr);
}

DirectX::XMFLOAT3 ParticleRenderingComponent::GetGravityDirectionSimulation() const
{
	return GravityDirectionSimulation;
}

void ParticleRenderingComponent::SetGravityDirectionSimulation(const DirectX::XMFLOAT3& NewGravityDirectionSimulation)
{
	GravityDirectionSimulation = NewGravityDirectionSimulation;
}

int ParticleRenderingComponent::GetMaxParticleCount() const
{
	return MaxParticleCount;
}

float ParticleRenderingComponent::GetParticleSizeWorld() const
{
	return ParticleSizeWorld;
}

void ParticleRenderingComponent::SetParticleSizeWorld(float NewParticleSizeWorld)
{
	ParticleSizeWorld = NewParticleSizeWorld;
}

ID3D11Buffer* ParticleRenderingComponent::GetParticleMaterialConstantBuffer() const
{
	return ParticleMaterialConstantBuffer;
}

ID3D11VertexShader* ParticleRenderingComponent::GetParticleVertexShader() const
{
	return ParticleVertexShader;
}

ID3D11PixelShader* ParticleRenderingComponent::GetParticlePixelShader() const
{
	return ParticlePixelShader;
}

ID3D11ShaderResourceView* ParticleRenderingComponent::GetParticleStateShaderResourceView() const
{
	return ParticleStateShaderResourceView;
}

ID3D11BlendState* ParticleRenderingComponent::GetParticleAlphaBlendState() const
{
	return ParticleAlphaBlendState;
}

ID3D11DepthStencilState* ParticleRenderingComponent::GetParticleDepthStencilState() const
{
	return ParticleDepthStencilState;
}

ID3D11RasterizerState* ParticleRenderingComponent::GetParticleRasterizerState() const
{
	return ParticleRasterizerState;
}

UINT ParticleRenderingComponent::GetParticleDrawInstanceCount() const
{
	return static_cast<UINT>(MaxParticleCount);
}
