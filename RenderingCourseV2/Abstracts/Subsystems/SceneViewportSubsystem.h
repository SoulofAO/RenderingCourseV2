#pragma once

#include "Abstracts/Subsystems/Subsystem.h"
#include "Abstracts/Rendering/DeferredRenderer.h"
#include <windows.h>
#include <wrl.h>
#include <d3d11.h>
#include <memory>
#include <directxmath.h>

class DisplayWin32;

enum class RenderPipelineType
{
	Forward,
	Deferred
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
	DisplayWin32* GetDisplay() const;
	int GetScreenWidth() const;
	int GetScreenHeight() const;
	DirectX::XMMATRIX GetViewMatrix() const;
	DirectX::XMMATRIX GetProjectionMatrix() const;
	DirectX::XMFLOAT3 GetCameraWorldPosition() const;
	DirectX::XMFLOAT3 GetDirectionalLightDirection() const;
	DirectX::XMFLOAT4 GetDirectionalLightColor() const;
	float GetDirectionalLightIntensity() const;
	float GetUseFullBrightnessWithoutLighting() const;

	void SetFrameCameraData(const DirectX::XMMATRIX& NewViewMatrix, const DirectX::XMMATRIX& NewProjectionMatrix, const DirectX::XMFLOAT3& NewCameraWorldPosition);
	void SetDirectionalLightData(const DirectX::XMFLOAT3& NewLightDirection, const DirectX::XMFLOAT4& NewLightColor, float NewLightIntensity, float NewUseFullBrightnessWithoutLighting);
	void SetRenderPipelineType(RenderPipelineType NewRenderPipelineType);
	RenderPipelineType GetRenderPipelineType() const;
	bool IsDeferredRenderingEnabled() const;
	void BeginGeometryPass();
	void EndGeometryPass();
	void ExecuteDeferredLightingPass();

	bool bDisplayChangedColor = false;

private:
	void CreateBackBuffer();
	void RestoreTargets();
	void DestroyResources();

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
	float UseFullBrightnessWithoutLighting;
	RenderPipelineType CurrentRenderPipelineType;
	std::unique_ptr<DeferredRenderer> DeferredRendererInstance;
};
