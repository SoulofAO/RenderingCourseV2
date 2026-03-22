#pragma once

#include "Abstracts/Subsystems/Subsystem.h"
#include "Abstracts/Rendering/DeferredRenderer.h"
#include <windows.h>
#include <wrl.h>
#include <d3d11.h>
#include <memory>
#include <vector>
#include <directxmath.h>

class DisplayWin32;
class RenderingComponent;
class AbstractRenderPipeline;

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

	void BeginFrame(float TotalTimeSeconds);
	void EndFrame();

	ID3D11Device* GetDevice() const;
	ID3D11DeviceContext* GetDeviceContext() const;
	IDXGISwapChain* GetSwapChain() const;
	ID3D11RenderTargetView* GetRenderTargetView() const;
	ID3D11DepthStencilView* GetDepthStencilView() const;
	DeferredRenderer* GetDeferredRenderer() const;
	DisplayWin32* GetDisplay() const;
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

	void SetFrameCameraData(const DirectX::XMMATRIX& NewViewMatrix, const DirectX::XMMATRIX& NewProjectionMatrix, const DirectX::XMFLOAT3& NewCameraWorldPosition);
	void SetDirectionalLightData(const DirectX::XMFLOAT3& NewLightDirection, const DirectX::XMFLOAT4& NewLightColor, float NewLightIntensity, float NewUseFullBrightnessWithoutLighting);
	void SetIsShadowRenderingEnabled(bool NewIsShadowRenderingEnabled);
	void SetShadowCascadeSettings(int NewShadowCascadeCount, float NewShadowMaximumDistance);
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

	bool bDisplayChangedColor = false;

private:
	void CreateBackBuffer();
	void RestoreTargets();
	void DestroyResources();
	void InitializeDearImGui();
	void ShutdownDearImGui();

	std::unique_ptr<DisplayWin32> Display;
	Microsoft::WRL::ComPtr<ID3D11Device> Device;
	ID3D11DeviceContext* Context;
	IDXGISwapChain* SwapChain;
	ID3D11Texture2D* BackBuffer;
	ID3D11RenderTargetView* RenderView;
	ID3D11Texture2D* DepthTexture;
	ID3D11DepthStencilView* DepthStencilView;
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
	DeferredDebugBufferViewMode CurrentDeferredDebugBufferViewMode;
	RenderPipelineType CurrentRenderPipelineType;
	bool ParticleDistanceSortEnabled;
	std::unique_ptr<DeferredRenderer> DeferredRendererInstance;
	std::unique_ptr<AbstractRenderPipeline> ForwardRenderPipelineInstance;
	std::unique_ptr<AbstractRenderPipeline> DeferredRenderPipelineInstance;
	bool IsDearImGuiInitialized;
};
