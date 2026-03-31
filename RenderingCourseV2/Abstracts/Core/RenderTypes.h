#pragma once

#include <d3d11.h>
#include <windows.h>

struct GameRenderTargetOverride
{
	ID3D11RenderTargetView* RenderTargetView;
	ID3D11DepthStencilView* DepthStencilView;
	int Width;
	int Height;
};

struct RenderFrameContext
{
	ID3D11Device* Device;
	ID3D11DeviceContext* DeviceContext;
	ID3D11Texture2D* BackBufferTexture;
	ID3D11RenderTargetView* BackBufferRenderTargetView;
	ID3D11DepthStencilView* BackBufferDepthStencilView;
	HWND WindowHandle;
	int ScreenWidth;
	int ScreenHeight;
	float TotalTimeSeconds;
};
