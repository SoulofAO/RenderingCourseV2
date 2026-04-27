#include "Abstracts/Subsystems/SceneViewportSubsystem.h"
#include "Abstracts/Components/RenderingComponent.h"
#include "Abstracts/Components/LightComponent.h"
#include "Abstracts/Core/Game.h"
#include "Abstracts/Core/Actor.h"
#include "Abstracts/Core/GameInstance.h"
#include "Abstracts/Rendering/AbstractRenderPipeline.h"
#include "Abstracts/Rendering/ForwardRenderPipeline.h"
#include "Abstracts/Rendering/DeferredRenderPipeline.h"
#include "Abstracts/Subsystems/RenderRuntimeGameInstanceSubsystem.h"
#include <algorithm>
#include <cmath>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")

SceneViewportSubsystem::SceneViewportSubsystem()
	: CameraWorldPosition(0.0f, 0.0f, 0.0f)
	, DirectionalLightDirection(0.0f, -1.0f, 0.0f)
	, DirectionalLightColor(1.0f, 1.0f, 1.0f, 1.0f)
	, DirectionalLightIntensity(1.0f)
	, UseFullBrightnessWithoutLighting(0.0f)
	, IsShadowRenderingEnabled(true)
	, ShadowCascadeCountSetting(4)
	, ShadowMaximumDistanceSetting(160.0f)
	, UseShadowedAlbedoTextureWithoutShadowDimming(true)
	, CurrentDeferredDebugBufferViewMode(DeferredDebugBufferViewMode::FinalLighting)
	, CurrentRenderPipelineType(RenderPipelineType::Forward)
	, ParticleDistanceSortEnabled(true)
	, FrameRenderTargetOverrideView(nullptr)
	, FrameDepthStencilOverrideView(nullptr)
	, FrameOverrideWidth(0)
	, FrameOverrideHeight(0)
	, HasFrameViewportOverride(false)
	, FramePresentEnabled(true)
	, UseExternalRenderFrameContext(false)
	, ExternalRenderFrameContext{}
	, LastKnownWindowHandle(nullptr)
	, CachedRenderRuntimeSubsystem(nullptr)
{
	DirectX::XMStoreFloat4x4(&ViewMatrixStorage, DirectX::XMMatrixIdentity());
	DirectX::XMStoreFloat4x4(&ProjectionMatrixStorage, DirectX::XMMatrixIdentity());
}

SceneViewportSubsystem::~SceneViewportSubsystem()
{
	Shutdown();
}

void SceneViewportSubsystem::Initialize()
{
	CachedRenderRuntimeSubsystem = ResolveRenderRuntimeSubsystem();
	RenderRuntimeGameInstanceSubsystem* RenderRuntimeSubsystem = ResolveRenderRuntimeSubsystem();
	if (RenderRuntimeSubsystem != nullptr)
	{
		LastKnownWindowHandle = RenderRuntimeSubsystem->GetWindowHandle();
	}

	Subsystem::Initialize();
}

void SceneViewportSubsystem::Shutdown()
{
	CachedRenderRuntimeSubsystem = nullptr;

	ForwardRenderPipelineInstance.reset();
	DeferredRenderPipelineInstance.reset();
	if (DeferredRendererInstance != nullptr)
	{
		DeferredRendererInstance->Shutdown();
		DeferredRendererInstance.reset();
	}
	Subsystem::Shutdown();
}

void SceneViewportSubsystem::SetExternalRenderFrameContext(const RenderFrameContext& NewRenderFrameContext)
{
	ExternalRenderFrameContext = NewRenderFrameContext;
	UseExternalRenderFrameContext = true;
	LastKnownWindowHandle = NewRenderFrameContext.WindowHandle;
}

void SceneViewportSubsystem::ClearExternalRenderFrameContext()
{
	UseExternalRenderFrameContext = false;
}

void SceneViewportSubsystem::BeginFrame(float TotalTimeSeconds)
{
	ID3D11DeviceContext* ActiveDeviceContext = GetDeviceContext();
	ID3D11RenderTargetView* ActiveBaseRenderTargetView = UseExternalRenderFrameContext
		? ExternalRenderFrameContext.BackBufferRenderTargetView
		: nullptr;
	if (ActiveDeviceContext == nullptr || ActiveBaseRenderTargetView == nullptr)
	{
		return;
	}

	ID3D11RenderTargetView* ActiveRenderTargetView = FrameRenderTargetOverrideView != nullptr ? FrameRenderTargetOverrideView : ActiveBaseRenderTargetView;
	ID3D11DepthStencilView* ActiveBaseDepthStencilView = UseExternalRenderFrameContext
		? ExternalRenderFrameContext.BackBufferDepthStencilView
		: nullptr;
	ID3D11DepthStencilView* ActiveDepthStencilView = FrameDepthStencilOverrideView != nullptr ? FrameDepthStencilOverrideView : ActiveBaseDepthStencilView;

	if (bDisplayChangedColor)
	{
		float ColorCycle = TotalTimeSeconds;
		while (ColorCycle > 1.0f)
		{
			ColorCycle -= 1.0f;
		}

		float ClearColor[] = { ColorCycle, 0.1f, 0.1f, 1.0f };
		ActiveDeviceContext->ClearRenderTargetView(ActiveRenderTargetView, ClearColor);
	}
	else
	{
		float ClearColor[] = { 0.0, 0.0f, 0.0f, 1.0f };
		ActiveDeviceContext->ClearRenderTargetView(ActiveRenderTargetView, ClearColor);
	}

	if (ActiveDepthStencilView != nullptr)
	{
		ActiveDeviceContext->ClearDepthStencilView(ActiveDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	}

	ActiveDeviceContext->ClearState();

	D3D11_VIEWPORT Viewport = {};
	if (HasFrameViewportOverride)
	{
		Viewport = FrameViewportOverride;
	}
	else
	{
		Viewport.Width = static_cast<float>(GetScreenWidth());
		Viewport.Height = static_cast<float>(GetScreenHeight());
		Viewport.TopLeftX = 0.0f;
		Viewport.TopLeftY = 0.0f;
		Viewport.MinDepth = 0.0f;
		Viewport.MaxDepth = 1.0f;
	}
	ActiveDeviceContext->RSSetViewports(1, &Viewport);
}

void SceneViewportSubsystem::EndFrame()
{
	ID3D11DeviceContext* ActiveDeviceContext = GetDeviceContext();
	if (ActiveDeviceContext != nullptr)
	{
		ActiveDeviceContext->OMSetRenderTargets(0, nullptr, nullptr);
	}
}

void SceneViewportSubsystem::RenderFrame(
	const RenderFrameContext& NewRenderFrameContext,
	const GameRenderTargetOverride* OverrideRenderTarget,
	const D3D11_VIEWPORT* OverrideViewport,
	bool NewFramePresentEnabled,
	const std::function<void()>& BeforeSceneRenderCallback)
{
	SetExternalRenderFrameContext(NewRenderFrameContext);
	if (OverrideRenderTarget != nullptr)
	{
		SetFrameRenderTargetOverride(
			OverrideRenderTarget->RenderTargetView,
			OverrideRenderTarget->DepthStencilView,
			OverrideRenderTarget->Width,
			OverrideRenderTarget->Height);
	}
	if (OverrideViewport != nullptr)
	{
		SetFrameViewportOverride(*OverrideViewport);
	}
	SetFramePresentEnabled(NewFramePresentEnabled);
	BeginFrame(NewRenderFrameContext.TotalTimeSeconds);
	BeginDearImGuiFrame();
	if (BeforeSceneRenderCallback)
	{
		BeforeSceneRenderCallback();
	}
	RenderSceneFrame();
	ClearExternalRenderFrameContext();
	ClearFrameRenderTargetOverride();
	ClearFrameViewportOverride();
	SetFramePresentEnabled(true);
}

ID3D11Device* SceneViewportSubsystem::GetDevice() const
{
	RenderRuntimeGameInstanceSubsystem* RenderRuntimeSubsystem = ResolveRenderRuntimeSubsystem();
	if (RenderRuntimeSubsystem != nullptr)
	{
		return RenderRuntimeSubsystem->GetDevice();
	}
	if (UseExternalRenderFrameContext)
	{
		return ExternalRenderFrameContext.Device;
	}
	return nullptr;
}

ID3D11DeviceContext* SceneViewportSubsystem::GetDeviceContext() const
{
	RenderRuntimeGameInstanceSubsystem* RenderRuntimeSubsystem = ResolveRenderRuntimeSubsystem();
	if (RenderRuntimeSubsystem != nullptr)
	{
		return RenderRuntimeSubsystem->GetDeviceContext();
	}
	if (UseExternalRenderFrameContext)
	{
		return ExternalRenderFrameContext.DeviceContext;
	}
	return nullptr;
}

ID3D11RenderTargetView* SceneViewportSubsystem::GetRenderTargetView() const
{
	if (FrameRenderTargetOverrideView != nullptr)
	{
		return FrameRenderTargetOverrideView;
	}
	if (UseExternalRenderFrameContext)
	{
		return ExternalRenderFrameContext.BackBufferRenderTargetView;
	}
	return nullptr;
}

ID3D11DepthStencilView* SceneViewportSubsystem::GetDepthStencilView() const
{
	if (FrameDepthStencilOverrideView != nullptr)
	{
		return FrameDepthStencilOverrideView;
	}
	if (UseExternalRenderFrameContext)
	{
		return ExternalRenderFrameContext.BackBufferDepthStencilView;
	}
	return nullptr;
}

DeferredRenderer* SceneViewportSubsystem::GetDeferredRenderer() const
{
	return DeferredRendererInstance.get();
}

HWND SceneViewportSubsystem::GetWindowHandle() const
{
	RenderRuntimeGameInstanceSubsystem* RenderRuntimeSubsystem = ResolveRenderRuntimeSubsystem();
	if (RenderRuntimeSubsystem != nullptr)
	{
		HWND WindowHandle = RenderRuntimeSubsystem->GetWindowHandle();
		if (WindowHandle != nullptr)
		{
			return WindowHandle;
		}
	}

	return LastKnownWindowHandle;
}

int SceneViewportSubsystem::GetScreenWidth() const
{
	if (FrameOverrideWidth > 0)
	{
		return FrameOverrideWidth;
	}
	if (UseExternalRenderFrameContext)
	{
		return ExternalRenderFrameContext.ScreenWidth;
	}
	Game* GameInstance = GetOwningGame();
	if (GameInstance == nullptr)
	{
		return 0;
	}

	return GameInstance->GetScreenWidth();
}

int SceneViewportSubsystem::GetScreenHeight() const
{
	if (FrameOverrideHeight > 0)
	{
		return FrameOverrideHeight;
	}
	if (UseExternalRenderFrameContext)
	{
		return ExternalRenderFrameContext.ScreenHeight;
	}
	Game* GameInstance = GetOwningGame();
	if (GameInstance == nullptr)
	{
		return 0;
	}

	return GameInstance->GetScreenHeight();
}

DirectX::XMMATRIX SceneViewportSubsystem::GetViewMatrix() const
{
	return DirectX::XMLoadFloat4x4(&ViewMatrixStorage);
}

DirectX::XMMATRIX SceneViewportSubsystem::GetProjectionMatrix() const
{
	return DirectX::XMLoadFloat4x4(&ProjectionMatrixStorage);
}

DirectX::XMFLOAT3 SceneViewportSubsystem::GetCameraWorldPosition() const
{
	return CameraWorldPosition;
}

DirectX::XMFLOAT3 SceneViewportSubsystem::GetDirectionalLightDirection() const
{
	return DirectionalLightDirection;
}

DirectX::XMFLOAT4 SceneViewportSubsystem::GetDirectionalLightColor() const
{
	return DirectionalLightColor;
}

float SceneViewportSubsystem::GetDirectionalLightIntensity() const
{
	return DirectionalLightIntensity;
}

const std::vector<DeferredPointLightData>& SceneViewportSubsystem::GetPointLights() const
{
	return PointLights;
}

const std::vector<DeferredSpotLightData>& SceneViewportSubsystem::GetSpotLights() const
{
	return SpotLights;
}

float SceneViewportSubsystem::GetUseFullBrightnessWithoutLighting() const
{
	return UseFullBrightnessWithoutLighting;
}

bool SceneViewportSubsystem::GetIsShadowRenderingEnabled() const
{
	return IsShadowRenderingEnabled;
}

int SceneViewportSubsystem::GetShadowCascadeCountSetting() const
{
	return ShadowCascadeCountSetting;
}

float SceneViewportSubsystem::GetShadowMaximumDistanceSetting() const
{
	return ShadowMaximumDistanceSetting;
}

bool SceneViewportSubsystem::GetUseShadowedAlbedoTextureWithoutShadowDimming() const
{
	return UseShadowedAlbedoTextureWithoutShadowDimming;
}

void SceneViewportSubsystem::SetFrameCameraData(const DirectX::XMMATRIX& NewViewMatrix, const DirectX::XMMATRIX& NewProjectionMatrix, const DirectX::XMFLOAT3& NewCameraWorldPosition)
{
	DirectX::XMStoreFloat4x4(&ViewMatrixStorage, NewViewMatrix);
	DirectX::XMStoreFloat4x4(&ProjectionMatrixStorage, NewProjectionMatrix);
	CameraWorldPosition = NewCameraWorldPosition;
}

void SceneViewportSubsystem::SetDirectionalLightData(const DirectX::XMFLOAT3& NewLightDirection, const DirectX::XMFLOAT4& NewLightColor, float NewLightIntensity, float NewUseFullBrightnessWithoutLighting)
{
	DirectionalLightDirection = NewLightDirection;
	DirectionalLightColor = NewLightColor;
	DirectionalLightIntensity = NewLightIntensity;
	UseFullBrightnessWithoutLighting = NewUseFullBrightnessWithoutLighting;
}

void SceneViewportSubsystem::SetIsShadowRenderingEnabled(bool NewIsShadowRenderingEnabled)
{
	IsShadowRenderingEnabled = NewIsShadowRenderingEnabled;
}

void SceneViewportSubsystem::SetShadowCascadeSettings(int NewShadowCascadeCount, float NewShadowMaximumDistance)
{
	const int ClampedShadowCascadeCount = (std::max)(1, (std::min)(NewShadowCascadeCount, 4));
	const float ClampedShadowMaximumDistance = (std::max)(10.0f, NewShadowMaximumDistance);

	ShadowCascadeCountSetting = ClampedShadowCascadeCount;
	ShadowMaximumDistanceSetting = ClampedShadowMaximumDistance;

	if (DeferredRendererInstance != nullptr)
	{
		DeferredRendererInstance->SetShadowCascadeSettings(ShadowCascadeCountSetting, ShadowMaximumDistanceSetting);
	}
}

void SceneViewportSubsystem::SetUseShadowedAlbedoTextureWithoutShadowDimming(bool NewUseShadowedAlbedoTextureWithoutShadowDimming)
{
	UseShadowedAlbedoTextureWithoutShadowDimming = NewUseShadowedAlbedoTextureWithoutShadowDimming;
}

void SceneViewportSubsystem::SetRenderPipelineType(RenderPipelineType NewRenderPipelineType)
{
	CurrentRenderPipelineType = NewRenderPipelineType;
}

RenderPipelineType SceneViewportSubsystem::GetRenderPipelineType() const
{
	return CurrentRenderPipelineType;
}

void SceneViewportSubsystem::SetDeferredDebugBufferViewMode(DeferredDebugBufferViewMode NewDeferredDebugBufferViewMode)
{
	CurrentDeferredDebugBufferViewMode = NewDeferredDebugBufferViewMode;
}

DeferredDebugBufferViewMode SceneViewportSubsystem::GetDeferredDebugBufferViewMode() const
{
	return CurrentDeferredDebugBufferViewMode;
}

bool SceneViewportSubsystem::IsDeferredRenderingEnabled() const
{
	return CurrentRenderPipelineType == RenderPipelineType::Deferred;
}

bool SceneViewportSubsystem::GetParticleDistanceSortEnabled() const
{
	return ParticleDistanceSortEnabled;
}

void SceneViewportSubsystem::SetParticleDistanceSortEnabled(bool NewParticleDistanceSortEnabled)
{
	ParticleDistanceSortEnabled = NewParticleDistanceSortEnabled;
}

void SceneViewportSubsystem::RenderSceneFrame()
{
	if (!EnsureRenderingResourcesInitialized())
	{
		EndDearImGuiFrame();
		EndFrame();
		return;
	}

	Game* GameInstance = GetOwningGame();
	if (GameInstance == nullptr)
	{
		return;
	}

	std::vector<RenderingComponent*> RenderingComponents;
	std::vector<LightComponent*> LightComponents;
	const std::vector<Actor*> AllActors = GameInstance->GetAllActorsByClass<Actor>();
	for (Actor* ExistingActor : AllActors)
	{
		if (ExistingActor == nullptr)
		{
			continue;
		}

		const std::vector<std::unique_ptr<ActorComponent>>& ActorComponents = ExistingActor->GetComponents();
		for (const std::unique_ptr<ActorComponent>& ExistingComponent : ActorComponents)
		{
			RenderingComponent* ExistingRenderingComponent = dynamic_cast<RenderingComponent*>(ExistingComponent.get());
			if (ExistingRenderingComponent != nullptr && ExistingRenderingComponent->GetIsActive())
			{
				RenderingComponents.push_back(ExistingRenderingComponent);
			}

			LightComponent* ExistingLightComponent = dynamic_cast<LightComponent*>(ExistingComponent.get());
			if (ExistingLightComponent != nullptr && ExistingLightComponent->GetIsActive())
			{
				LightComponents.push_back(ExistingLightComponent);
			}
		}
	}

	bool HasDirectionalLight = false;
	PointLights.clear();
	SpotLights.clear();
	PointLights.reserve(MaximumDeferredPointLightCount);
	SpotLights.reserve(MaximumDeferredSpotLightCount);
	for (LightComponent* ExistingLightComponent : LightComponents)
	{
		const LightType ExistingLightType = ExistingLightComponent->GetLightType();
		if (ExistingLightType == LightType::Directional && HasDirectionalLight == false)
		{
			SetDirectionalLightData(
				ExistingLightComponent->GetDirection(),
				ExistingLightComponent->GetColor(),
				ExistingLightComponent->GetIntensity(),
				0.0f);
			HasDirectionalLight = true;
		}

		if (ExistingLightType == LightType::Point)
		{
			if (static_cast<int>(PointLights.size()) >= MaximumDeferredPointLightCount)
			{
				continue;
			}

			const Transform ExistingLightTransform = ExistingLightComponent->GetWorldTransform();
			DeferredPointLightData PointLightData = {};
			PointLightData.Position = ExistingLightTransform.Position;
			PointLightData.Intensity = ExistingLightComponent->GetIntensity();
			PointLightData.Color = ExistingLightComponent->GetColor();
			PointLightData.Range = ExistingLightComponent->GetRange();
			PointLights.push_back(PointLightData);
			continue;
		}

		if (ExistingLightType == LightType::Spot)
		{
			if (static_cast<int>(SpotLights.size()) >= MaximumDeferredSpotLightCount)
			{
				continue;
			}

			const Transform ExistingLightTransform = ExistingLightComponent->GetWorldTransform();
			DeferredSpotLightData SpotLightData = {};
			SpotLightData.Position = ExistingLightTransform.Position;
			SpotLightData.Intensity = ExistingLightComponent->GetIntensity();
			SpotLightData.Color = ExistingLightComponent->GetColor();
			SpotLightData.Direction = ExistingLightComponent->GetDirection();
			SpotLightData.Range = ExistingLightComponent->GetRange();
			SpotLightData.InnerConeAngleCosine = std::cos(DirectX::XMConvertToRadians(ExistingLightComponent->GetSpotInnerConeAngleDegrees()));
			SpotLightData.OuterConeAngleCosine = std::cos(DirectX::XMConvertToRadians(ExistingLightComponent->GetSpotOuterConeAngleDegrees()));
			SpotLights.push_back(SpotLightData);
		}
	}
	if (HasDirectionalLight == false)
	{
		SetDirectionalLightData(
			DirectX::XMFLOAT3(0.0f, -1.0f, 0.0f),
			DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
			0.0f,
			1.0f);
	}

	std::stable_sort(
		RenderingComponents.begin(),
		RenderingComponents.end(),
		[](const RenderingComponent* LeftRenderingComponent, const RenderingComponent* RightRenderingComponent)
		{
			return LeftRenderingComponent->GetRenderOrder() < RightRenderingComponent->GetRenderOrder();
		});

	AbstractRenderPipeline* ActiveRenderPipeline = nullptr;
	if (CurrentRenderPipelineType == RenderPipelineType::Deferred)
	{
		ActiveRenderPipeline = DeferredRenderPipelineInstance.get();
	}
	else
	{
		ActiveRenderPipeline = ForwardRenderPipelineInstance.get();
	}

	if (ActiveRenderPipeline != nullptr)
	{
		ActiveRenderPipeline->RenderFrame(this, RenderingComponents);
	}

	EndDearImGuiFrame();
	EndFrame();
}
void SceneViewportSubsystem::BeginDearImGuiFrame()
{
	RenderRuntimeGameInstanceSubsystem* RenderRuntimeSubsystem = ResolveRenderRuntimeSubsystem();
	if (RenderRuntimeSubsystem == nullptr)
	{
		return;
	}

	RenderRuntimeSubsystem->BeginDearImGuiFrame();
}

void SceneViewportSubsystem::EndDearImGuiFrame()
{
	RenderRuntimeGameInstanceSubsystem* RenderRuntimeSubsystem = ResolveRenderRuntimeSubsystem();
	if (RenderRuntimeSubsystem == nullptr)
	{
		return;
	}

	RenderRuntimeSubsystem->EndDearImGuiFrame();
}

bool SceneViewportSubsystem::HandleDearImGuiMessage(HWND WindowHandle, UINT Message, WPARAM WParam, LPARAM LParam)
{
	RenderRuntimeGameInstanceSubsystem* RenderRuntimeSubsystem = ResolveRenderRuntimeSubsystem();
	if (RenderRuntimeSubsystem == nullptr)
	{
		return false;
	}

	return RenderRuntimeSubsystem->HandleDearImGuiMessage(WindowHandle, Message, WParam, LParam);
}

bool SceneViewportSubsystem::GetIsDearImGuiInitialized() const
{
	RenderRuntimeGameInstanceSubsystem* RenderRuntimeSubsystem = ResolveRenderRuntimeSubsystem();
	if (RenderRuntimeSubsystem == nullptr)
	{
		return false;
	}

	return RenderRuntimeSubsystem->GetIsDearImGuiInitialized();
}

ID3D11ShaderResourceView* SceneViewportSubsystem::GetDearImGuiBackBufferCopyShaderResourceView() const
{
	RenderRuntimeGameInstanceSubsystem* RenderRuntimeSubsystem = ResolveRenderRuntimeSubsystem();
	if (RenderRuntimeSubsystem == nullptr)
	{
		return nullptr;
	}

	return RenderRuntimeSubsystem->GetDearImGuiBackBufferCopyShaderResourceView();
}

bool SceneViewportSubsystem::EnsureRenderingResourcesInitialized()
{
	ID3D11Device* DeviceInstance = GetDevice();
	if (DeviceInstance == nullptr)
	{
		return false;
	}
	if (DeferredRendererInstance == nullptr)
	{
		DeferredRendererInstance = std::make_unique<DeferredRenderer>();
		DeferredRendererInstance->Initialize(DeviceInstance);
	}
	if (ForwardRenderPipelineInstance == nullptr)
	{
		ForwardRenderPipelineInstance = std::make_unique<ForwardRenderPipeline>();
	}
	if (DeferredRenderPipelineInstance == nullptr)
	{
		DeferredRenderPipelineInstance = std::make_unique<DeferredRenderPipeline>();
	}
	if (DeferredRendererInstance != nullptr)
	{
		DeferredRendererInstance->EnsureTargets(DeviceInstance, GetScreenWidth(), GetScreenHeight());
		DeferredRendererInstance->SetShadowCascadeSettings(ShadowCascadeCountSetting, ShadowMaximumDistanceSetting);
	}
	return true;
}

void SceneViewportSubsystem::SetFrameRenderTargetOverride(
	ID3D11RenderTargetView* NewRenderTargetView,
	ID3D11DepthStencilView* NewDepthStencilView,
	int NewWidth,
	int NewHeight)
{
	FrameRenderTargetOverrideView = NewRenderTargetView;
	FrameDepthStencilOverrideView = NewDepthStencilView;
	FrameOverrideWidth = NewWidth;
	FrameOverrideHeight = NewHeight;
}

void SceneViewportSubsystem::ClearFrameRenderTargetOverride()
{
	FrameRenderTargetOverrideView = nullptr;
	FrameDepthStencilOverrideView = nullptr;
	FrameOverrideWidth = 0;
	FrameOverrideHeight = 0;
}

void SceneViewportSubsystem::SetFrameViewportOverride(const D3D11_VIEWPORT& NewViewport)
{
	FrameViewportOverride = NewViewport;
	HasFrameViewportOverride = true;
}

void SceneViewportSubsystem::ClearFrameViewportOverride()
{
	HasFrameViewportOverride = false;
}

void SceneViewportSubsystem::SetFramePresentEnabled(bool NewFramePresentEnabled)
{
	FramePresentEnabled = NewFramePresentEnabled;
}

RenderRuntimeGameInstanceSubsystem* SceneViewportSubsystem::ResolveRenderRuntimeSubsystem() const
{
	if (CachedRenderRuntimeSubsystem != nullptr)
	{
		return CachedRenderRuntimeSubsystem;
	}

	Game* OwningGame = GetOwningGame();
	if (OwningGame != nullptr)
	{
		GameInstance* OwningGameInstance = OwningGame->GetOwningGameInstance();
		if (OwningGameInstance != nullptr)
		{
			CachedRenderRuntimeSubsystem = OwningGameInstance->GetSubsystem<RenderRuntimeGameInstanceSubsystem>();
			if (CachedRenderRuntimeSubsystem != nullptr)
			{
				return CachedRenderRuntimeSubsystem;
			}
		}
	}

	if (GlobalGameInstance != nullptr)
	{
		CachedRenderRuntimeSubsystem = GlobalGameInstance->GetSubsystem<RenderRuntimeGameInstanceSubsystem>();
	}

	return CachedRenderRuntimeSubsystem;
}
