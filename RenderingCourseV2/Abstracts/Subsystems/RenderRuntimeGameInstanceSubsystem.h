#pragma once

#include "Abstracts/Subsystems/GameInstanceSubsystem.h"
#include "Abstracts/Core/PlayerRenderTargetService.h"
#include "Abstracts/Core/RenderTypes.h"
#include "Abstracts/Subsystems/DisplayWin32.h"
#include <d3d11.h>
#include <windows.h>
#include <wrl.h>
#include <memory>
#include <functional>
#include <vector>

class RenderRuntimeGameInstanceSubsystem : public GameInstanceSubsystem
{
public:
	RenderRuntimeGameInstanceSubsystem();
	~RenderRuntimeGameInstanceSubsystem() override;

	void InitializeRuntime(
		LPCWSTR ApplicationName,
		int ScreenWidth,
		int ScreenHeight,
		std::function<LRESULT(HWND, UINT, WPARAM, LPARAM)> MessageCallback);
	void Shutdown() override;
	void BeginRuntimeFrame(float TotalTimeSeconds);
	void EndRuntimeFrame();
	RenderFrameContext BuildFrameContext(float TotalTimeSeconds) const;
	bool AcquirePlayerRenderTarget(
		const PlayerRenderTargetIdentifier& Identifier,
		int Width,
		int Height,
		GameRenderTargetOverride& OutRenderTargetOverride);
	void CompositePlayerTargets(const std::vector<PlayerRenderTargetCompositeCommand>& CompositeCommands);
	HWND GetWindowHandle() const;
	ID3D11Device* GetDevice() const;
	ID3D11DeviceContext* GetDeviceContext() const;
	ID3D11Texture2D* GetBackBufferTexture() const;
	int GetScreenWidth() const;
	int GetScreenHeight() const;

private:
	void CreateBackBuffer();
	void DestroyResources();

	std::unique_ptr<DisplayWin32> Display;
	Microsoft::WRL::ComPtr<ID3D11Device> Device;
	ID3D11DeviceContext* Context;
	IDXGISwapChain* SwapChain;
	ID3D11Texture2D* BackBuffer;
	ID3D11RenderTargetView* BackBufferRenderView;
	ID3D11Texture2D* BackBufferDepthTexture;
	ID3D11DepthStencilView* BackBufferDepthStencilView;
	int ScreenWidth;
	int ScreenHeight;
	PlayerRenderTargetService PlayerRenderTargetServiceInstance;
};
