#include "Engine/Core/Runtime/Abstract/Subsystems/SceneViewportSubsystem.h"
#include "Engine/Core/Runtime/Abstract/Subsystems/DisplayWin32.h"
#include "Engine/Core/Runtime/Abstract/Core/Game.h"
#include <iostream>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")

SceneViewportSubsystem::SceneViewportSubsystem()
	: Context(nullptr)
	, SwapChain(nullptr)
	, BackBuffer(nullptr)
	, RenderView(nullptr)
{
}

SceneViewportSubsystem::~SceneViewportSubsystem()
{
	Shutdown();
}

void SceneViewportSubsystem::Initialize()
{
	Subsystem::Initialize();

	Game* GameInstance = GetOwningGame();
	HINSTANCE InstanceHandle = GetModuleHandle(nullptr);

	Display = std::make_unique<DisplayWin32>(
		GameInstance->GetApplicationName(),
		InstanceHandle,
		GameInstance->GetScreenWidth(),
		GameInstance->GetScreenHeight(),
		[GameInstance](HWND WindowHandle, UINT Message, WPARAM WParam, LPARAM LParam) -> LRESULT
		{
			return GameInstance->MessageHandler(WindowHandle, Message, WParam, LParam);
		});

	D3D_FEATURE_LEVEL FeatureLevels[] = { D3D_FEATURE_LEVEL_11_1 };
	DXGI_SWAP_CHAIN_DESC SwapDescription = {};
	SwapDescription.BufferCount = 2;
	SwapDescription.BufferDesc.Width = static_cast<UINT>(GameInstance->GetScreenWidth());
	SwapDescription.BufferDesc.Height = static_cast<UINT>(GameInstance->GetScreenHeight());
	SwapDescription.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	SwapDescription.BufferDesc.RefreshRate.Numerator = 60;
	SwapDescription.BufferDesc.RefreshRate.Denominator = 1;
	SwapDescription.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	SwapDescription.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	SwapDescription.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	SwapDescription.OutputWindow = Display->GetWindowHandle();
	SwapDescription.Windowed = true;
	SwapDescription.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	SwapDescription.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	SwapDescription.SampleDesc.Count = 1;
	SwapDescription.SampleDesc.Quality = 0;

	HRESULT Result = D3D11CreateDeviceAndSwapChain(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		D3D11_CREATE_DEVICE_DEBUG,
		FeatureLevels,
		1,
		D3D11_SDK_VERSION,
		&SwapDescription,
		&SwapChain,
		&Device,
		nullptr,
		&Context);

	if (FAILED(Result))
	{
		std::cerr << "Failed to create D3D11 device and swap chain." << std::endl;
		return;
	}

	CreateBackBuffer();
}

void SceneViewportSubsystem::Shutdown()
{
	DestroyResources();
	Subsystem::Shutdown();
}

void SceneViewportSubsystem::BeginFrame(float TotalTimeSeconds)
{
	if (Context == nullptr || RenderView == nullptr)
	{
		return;
	}

	Context->ClearState();
	ClearWindowTarget(TotalTimeSeconds);
	RestoreWindowTargetsAndViewport();
}

void SceneViewportSubsystem::EndFrame()
{
	if (Context != nullptr)
	{
		Context->OMSetRenderTargets(0, nullptr, nullptr);
	}

	if (SwapChain != nullptr)
	{
		SwapChain->Present(1, 0);
	}
}

void SceneViewportSubsystem::ActivateWindowRenderTarget()
{
	RestoreWindowTargetsAndViewport();
}

ID3D11Device* SceneViewportSubsystem::GetDevice() const
{
	return Device.Get();
}

ID3D11DeviceContext* SceneViewportSubsystem::GetDeviceContext() const
{
	return Context;
}

IDXGISwapChain* SceneViewportSubsystem::GetSwapChain() const
{
	return SwapChain;
}

DisplayWin32* SceneViewportSubsystem::GetDisplay() const
{
	return Display.get();
}

int SceneViewportSubsystem::GetScreenWidth() const
{
	Game* GameInstance = GetOwningGame();
	if (GameInstance == nullptr)
	{
		return 0;
	}

	return GameInstance->GetScreenWidth();
}

int SceneViewportSubsystem::GetScreenHeight() const
{
	Game* GameInstance = GetOwningGame();
	if (GameInstance == nullptr)
	{
		return 0;
	}

	return GameInstance->GetScreenHeight();
}

void SceneViewportSubsystem::CreateBackBuffer()
{
	if (SwapChain == nullptr || Device.Get() == nullptr)
	{
		return;
	}

	SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&BackBuffer));
	Device->CreateRenderTargetView(BackBuffer, nullptr, &RenderView);
}

void SceneViewportSubsystem::RestoreWindowTargetsAndViewport()
{
	if (Context == nullptr || RenderView == nullptr)
	{
		return;
	}

	Context->OMSetRenderTargets(1, &RenderView, nullptr);
	SetViewportSize(static_cast<float>(GetScreenWidth()), static_cast<float>(GetScreenHeight()));
}

void SceneViewportSubsystem::SetViewportSize(float Width, float Height)
{
	if (Context == nullptr)
	{
		return;
	}

	D3D11_VIEWPORT Viewport = {};
	Viewport.Width = Width;
	Viewport.Height = Height;
	Viewport.TopLeftX = 0.0f;
	Viewport.TopLeftY = 0.0f;
	Viewport.MinDepth = 0.0f;
	Viewport.MaxDepth = 1.0f;
	Context->RSSetViewports(1, &Viewport);
}

void SceneViewportSubsystem::ClearWindowTarget(float TotalTimeSeconds)
{
	if (Context == nullptr || RenderView == nullptr)
	{
		return;
	}

	if (bDisplayChangedColor)
	{
		float ColorCycle = TotalTimeSeconds;
		while (ColorCycle > 1.0f)
		{
			ColorCycle -= 1.0f;
		}

		float ClearColor[] = { ColorCycle, 0.1f, 0.1f, 1.0f };
		Context->ClearRenderTargetView(RenderView, ClearColor);
	}
	else
	{
		float ClearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		Context->ClearRenderTargetView(RenderView, ClearColor);
	}
}

void SceneViewportSubsystem::DestroyResources()
{
	if (RenderView != nullptr)
	{
		RenderView->Release();
		RenderView = nullptr;
	}

	if (BackBuffer != nullptr)
	{
		BackBuffer->Release();
		BackBuffer = nullptr;
	}

	if (SwapChain != nullptr)
	{
		SwapChain->Release();
		SwapChain = nullptr;
	}

	if (Context != nullptr)
	{
		Context->Release();
		Context = nullptr;
	}

	Display.reset();
}

