#pragma once

#include "Abstracts/Core/RenderTypes.h"
#include "Abstracts/Subsystems/Subsystem.h"
#include "Abstracts/Rendering/DeferredRenderer.h"
#include <windows.h>
#include <d3d11.h>
#include <memory>
#include <vector>
#include <functional>
#include <directxmath.h>

class RenderingComponent;
class AbstractRenderPipeline;
class RenderRuntimeGameInstanceSubsystem;

enum class RenderPipelineType
{
	Forward,
	Deferred
};

enum class DeferredDebugBufferViewMode
{
	FinalLighting = 0,
	Albedo = 1,
	Normal = 2,
	Material = 3,
	Depth = 4,
	ShadowVisibility = 5
};

class SceneViewportSubsystem : public Subsystem
{
public:
	SceneViewportSubsystem();
	~SceneViewportSubsystem() override;

	void Initialize() override;
	void Shutdown() override;
	void SetExternalRenderFrameContext(const RenderFrameContext& NewRenderFrameContext);
	void ClearExternalRenderFrameContext();

	void BeginFrame(float TotalTimeSeconds);
	void EndFrame();
	void RenderFrame(
		const RenderFrameContext& NewRenderFrameContext,
		const GameRenderTargetOverride* OverrideRenderTarget = nullptr,
		const D3D11_VIEWPORT* OverrideViewport = nullptr,
		bool NewFramePresentEnabled = false,
		const std::function<void()>& BeforeSceneRenderCallback = std::function<void()>());

	ID3D11Device* GetDevice() const;
	ID3D11DeviceContext* GetDeviceContext() const;
	ID3D11RenderTargetView* GetRenderTargetView() const;
	ID3D11DepthStencilView* GetDepthStencilView() const;
	DeferredRenderer* GetDeferredRenderer() const;
	HWND GetWindowHandle() const;
	int GetScreenWidth() const;
	int GetScreenHeight() const;
	DirectX::XMMATRIX GetViewMatrix() const;
	DirectX::XMMATRIX GetProjectionMatrix() const;
	DirectX::XMFLOAT3 GetCameraWorldPosition() const;
	DirectX::XMFLOAT3 GetDirectionalLightDirection() const;
	DirectX::XMFLOAT4 GetDirectionalLightColor() const;
	float GetDirectionalLightIntensity() const;
	const std::vector<DeferredPointLightData>& GetPointLights() const;
	const std::vector<DeferredSpotLightData>& GetSpotLights() const;
	float GetUseFullBrightnessWithoutLighting() const;
	bool GetIsShadowRenderingEnabled() const;
	int GetShadowCascadeCountSetting() const;
	float GetShadowMaximumDistanceSetting() const;
	bool GetUseShadowedAlbedoTextureWithoutShadowDimming() const;

	void SetFrameCameraData(const DirectX::XMMATRIX& NewViewMatrix, const DirectX::XMMATRIX& NewProjectionMatrix, const DirectX::XMFLOAT3& NewCameraWorldPosition);
	void SetDirectionalLightData(const DirectX::XMFLOAT3& NewLightDirection, const DirectX::XMFLOAT4& NewLightColor, float NewLightIntensity, float NewUseFullBrightnessWithoutLighting);
	void SetIsShadowRenderingEnabled(bool NewIsShadowRenderingEnabled);
	void SetShadowCascadeSettings(int NewShadowCascadeCount, float NewShadowMaximumDistance);
	void SetUseShadowedAlbedoTextureWithoutShadowDimming(bool NewUseShadowedAlbedoTextureWithoutShadowDimming);
	void SetRenderPipelineType(RenderPipelineType NewRenderPipelineType);
	RenderPipelineType GetRenderPipelineType() const;
	void SetDeferredDebugBufferViewMode(DeferredDebugBufferViewMode NewDeferredDebugBufferViewMode);
	DeferredDebugBufferViewMode GetDeferredDebugBufferViewMode() const;
	bool IsDeferredRenderingEnabled() const;
	bool GetParticleDistanceSortEnabled() const;
	void SetParticleDistanceSortEnabled(bool NewParticleDistanceSortEnabled);
	void RenderSceneFrame();
	void BeginDearImGuiFrame();
	void EndDearImGuiFrame();
	bool HandleDearImGuiMessage(HWND WindowHandle, UINT Message, WPARAM WParam, LPARAM LParam);
	bool GetIsDearImGuiInitialized() const;
	ID3D11ShaderResourceView* GetDearImGuiBackBufferCopyShaderResourceView() const;
	void SetFrameRenderTargetOverride(
		ID3D11RenderTargetView* NewRenderTargetView,
		ID3D11DepthStencilView* NewDepthStencilView,
		int NewWidth,
		int NewHeight);
	void ClearFrameRenderTargetOverride();
	void SetFrameViewportOverride(const D3D11_VIEWPORT& NewViewport);
	void ClearFrameViewportOverride();
	void SetFramePresentEnabled(bool NewFramePresentEnabled);

	bool bDisplayChangedColor = false;

private:
	RenderRuntimeGameInstanceSubsystem* ResolveRenderRuntimeSubsystem() const;
	bool EnsureRenderingResourcesInitialized();

	DirectX::XMFLOAT4X4 ViewMatrixStorage;
	DirectX::XMFLOAT4X4 ProjectionMatrixStorage;
	DirectX::XMFLOAT3 CameraWorldPosition;
	DirectX::XMFLOAT3 DirectionalLightDirection;
	DirectX::XMFLOAT4 DirectionalLightColor;
	float DirectionalLightIntensity;
	std::vector<DeferredPointLightData> PointLights;
	std::vector<DeferredSpotLightData> SpotLights;
	float UseFullBrightnessWithoutLighting;
	bool IsShadowRenderingEnabled;
	int ShadowCascadeCountSetting;
	float ShadowMaximumDistanceSetting;
	bool UseShadowedAlbedoTextureWithoutShadowDimming;
	DeferredDebugBufferViewMode CurrentDeferredDebugBufferViewMode;
	RenderPipelineType CurrentRenderPipelineType;
	bool ParticleDistanceSortEnabled;
	std::unique_ptr<DeferredRenderer> DeferredRendererInstance;
	std::unique_ptr<AbstractRenderPipeline> ForwardRenderPipelineInstance;
	std::unique_ptr<AbstractRenderPipeline> DeferredRenderPipelineInstance;
	ID3D11RenderTargetView* FrameRenderTargetOverrideView;
	ID3D11DepthStencilView* FrameDepthStencilOverrideView;
	int FrameOverrideWidth;
	int FrameOverrideHeight;
	bool HasFrameViewportOverride;
	D3D11_VIEWPORT FrameViewportOverride;
	bool FramePresentEnabled;
	bool UseExternalRenderFrameContext;
	RenderFrameContext ExternalRenderFrameContext;
	HWND LastKnownWindowHandle;
	mutable RenderRuntimeGameInstanceSubsystem* CachedRenderRuntimeSubsystem;
};
