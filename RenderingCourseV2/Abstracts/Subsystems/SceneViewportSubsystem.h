#pragma once

#include "Abstracts/Subsystems/Subsystem.h"
#include <windows.h>
#include <wrl.h>
#include <d3d11.h>
#include <memory>

class DisplayWin32;

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
};
