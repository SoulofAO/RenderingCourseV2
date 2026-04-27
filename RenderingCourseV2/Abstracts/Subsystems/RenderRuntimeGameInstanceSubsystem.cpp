#include "Abstracts/Subsystems/RenderRuntimeGameInstanceSubsystem.h"
#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>
#include <iostream>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND WindowHandle, UINT Message, WPARAM WParam, LPARAM LParam);

RenderRuntimeGameInstanceSubsystem::RenderRuntimeGameInstanceSubsystem()
	: Context(nullptr)
	, SwapChain(nullptr)
	, BackBuffer(nullptr)
	, BackBufferRenderView(nullptr)
	, BackBufferDepthTexture(nullptr)
	, BackBufferDepthStencilView(nullptr)
	, DearImGuiBackBufferCopyTexture(nullptr)
	, DearImGuiBackBufferCopyShaderResourceView(nullptr)
	, ScreenWidth(0)
	, ScreenHeight(0)
	, IsWindowMinimized(false)
	, IsDearImGuiInitialized(false)
{
}

RenderRuntimeGameInstanceSubsystem::~RenderRuntimeGameInstanceSubsystem()
{
	Shutdown();
}

void RenderRuntimeGameInstanceSubsystem::InitializeRuntime(
	LPCWSTR ApplicationName,
	int ScreenWidth,
	int ScreenHeight,
	std::function<LRESULT(HWND, UINT, WPARAM, LPARAM)> MessageCallback)
{
	this->ScreenWidth = ScreenWidth;
	this->ScreenHeight = ScreenHeight;
	HINSTANCE InstanceHandle = GetModuleHandle(nullptr);
	Display = std::make_unique<DisplayWin32>(
		ApplicationName,
		InstanceHandle,
		ScreenWidth,
		ScreenHeight,
		MessageCallback);

	D3D_FEATURE_LEVEL FeatureLevels[] = { D3D_FEATURE_LEVEL_11_1 };
	DXGI_SWAP_CHAIN_DESC SwapDescription = {};
	SwapDescription.BufferCount = 2;
	SwapDescription.BufferDesc.Width = static_cast<UINT>(ScreenWidth);
	SwapDescription.BufferDesc.Height = static_cast<UINT>(ScreenHeight);
	SwapDescription.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	SwapDescription.BufferDesc.RefreshRate.Numerator = 60;
	SwapDescription.BufferDesc.RefreshRate.Denominator = 1;
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

void RenderRuntimeGameInstanceSubsystem::Shutdown()
{
	ShutdownDearImGui();
	PlayerRenderTargetServiceInstance.ReleaseAll();
	DestroyResources();
	GameInstanceSubsystem::Shutdown();
}

void RenderRuntimeGameInstanceSubsystem::BeginRuntimeFrame(float TotalTimeSeconds)
{
	if (IsWindowMinimized || Context == nullptr || BackBufferRenderView == nullptr)
	{
		return;
	}

	const float ClearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	Context->ClearRenderTargetView(BackBufferRenderView, ClearColor);
	if (BackBufferDepthStencilView != nullptr)
	{
		Context->ClearDepthStencilView(BackBufferDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	}
	Context->ClearState();
	D3D11_VIEWPORT FullscreenViewport = {};
	FullscreenViewport.Width = static_cast<float>(ScreenWidth);
	FullscreenViewport.Height = static_cast<float>(ScreenHeight);
	FullscreenViewport.MinDepth = 0.0f;
	FullscreenViewport.MaxDepth = 1.0f;
	Context->RSSetViewports(1, &FullscreenViewport);
}

void RenderRuntimeGameInstanceSubsystem::EndRuntimeFrame()
{
	if (IsWindowMinimized)
	{
		return;
	}

	UpdateDearImGuiBackBufferCopyFromBackBuffer();

	if (Context != nullptr)
	{
		Context->OMSetRenderTargets(0, nullptr, nullptr);
	}
	if (SwapChain != nullptr)
	{
		SwapChain->Present(1, 0);
	}
}

RenderFrameContext RenderRuntimeGameInstanceSubsystem::BuildFrameContext(float TotalTimeSeconds) const
{
	RenderFrameContext FrameContext = {};
	FrameContext.Device = Device.Get();
	FrameContext.DeviceContext = Context;
	FrameContext.BackBufferTexture = BackBuffer;
	FrameContext.BackBufferRenderTargetView = BackBufferRenderView;
	FrameContext.BackBufferDepthStencilView = BackBufferDepthStencilView;
	FrameContext.WindowHandle = GetWindowHandle();
	FrameContext.ScreenWidth = ScreenWidth;
	FrameContext.ScreenHeight = ScreenHeight;
	FrameContext.TotalTimeSeconds = TotalTimeSeconds;
	return FrameContext;
}

bool RenderRuntimeGameInstanceSubsystem::AcquirePlayerRenderTarget(
	const PlayerRenderTargetIdentifier& Identifier,
	int Width,
	int Height,
	GameRenderTargetOverride& OutRenderTargetOverride)
{
	if (!PlayerRenderTargetServiceInstance.EnsurePlayerRenderTarget(Device.Get(), Identifier, Width, Height))
	{
		return false;
	}
	return PlayerRenderTargetServiceInstance.GetPlayerRenderTargetOverride(Identifier, OutRenderTargetOverride);
}

void RenderRuntimeGameInstanceSubsystem::CompositePlayerTargets(const std::vector<PlayerRenderTargetCompositeCommand>& CompositeCommands)
{
	PlayerRenderTargetServiceInstance.CompositeToBackBuffer(Context, BackBuffer, CompositeCommands);
}

void RenderRuntimeGameInstanceSubsystem::EnsureDearImGuiInitialized()
{
	if (IsDearImGuiInitialized)
	{
		return;
	}

	HWND WindowHandle = GetWindowHandle();
	if (WindowHandle == nullptr || Device.Get() == nullptr || Context == nullptr)
	{
		return;
	}

	IMGUI_CHECKVERSION();
	if (ImGui::GetCurrentContext() == nullptr)
	{
		ImGui::CreateContext();
		ImGui::StyleColorsDark();
	}

	ImGuiIO& DearImGuiIO = ImGui::GetIO();
	if (DearImGuiIO.BackendPlatformUserData == nullptr)
	{
		ImGui_ImplWin32_Init(WindowHandle);
	}
	if (DearImGuiIO.BackendRendererUserData == nullptr)
	{
		ImGui_ImplDX11_Init(Device.Get(), Context);
	}

	IsDearImGuiInitialized = DearImGuiIO.BackendPlatformUserData != nullptr && DearImGuiIO.BackendRendererUserData != nullptr;
}

void RenderRuntimeGameInstanceSubsystem::ShutdownDearImGui()
{
	if (IsDearImGuiInitialized == false)
	{
		return;
	}

	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	if (ImGui::GetCurrentContext() != nullptr)
	{
		ImGui::DestroyContext();
	}
	IsDearImGuiInitialized = false;
}

void RenderRuntimeGameInstanceSubsystem::BeginDearImGuiFrame()
{
	EnsureDearImGuiInitialized();
	if (IsDearImGuiInitialized == false)
	{
		return;
	}

	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void RenderRuntimeGameInstanceSubsystem::EndDearImGuiFrame()
{
	if (IsDearImGuiInitialized == false)
	{
		return;
	}

	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

bool RenderRuntimeGameInstanceSubsystem::HandleDearImGuiMessage(HWND WindowHandle, UINT Message, WPARAM WParam, LPARAM LParam)
{
	if (IsDearImGuiInitialized == false)
	{
		return false;
	}

	return ImGui_ImplWin32_WndProcHandler(WindowHandle, Message, WParam, LParam) != 0;
}

bool RenderRuntimeGameInstanceSubsystem::GetIsDearImGuiInitialized() const
{
	return IsDearImGuiInitialized;
}

ID3D11ShaderResourceView* RenderRuntimeGameInstanceSubsystem::GetDearImGuiBackBufferCopyShaderResourceView() const
{
	return DearImGuiBackBufferCopyShaderResourceView;
}

HWND RenderRuntimeGameInstanceSubsystem::GetWindowHandle() const
{
	if (Display == nullptr)
	{
		return nullptr;
	}
	return Display->GetWindowHandle();
}

ID3D11Device* RenderRuntimeGameInstanceSubsystem::GetDevice() const
{
	return Device.Get();
}

ID3D11DeviceContext* RenderRuntimeGameInstanceSubsystem::GetDeviceContext() const
{
	return Context;
}

ID3D11Texture2D* RenderRuntimeGameInstanceSubsystem::GetBackBufferTexture() const
{
	return BackBuffer;
}

int RenderRuntimeGameInstanceSubsystem::GetScreenWidth() const
{
	return ScreenWidth;
}

int RenderRuntimeGameInstanceSubsystem::GetScreenHeight() const
{
	return ScreenHeight;
}

void RenderRuntimeGameInstanceSubsystem::SetIsWindowMinimized(bool NewIsWindowMinimized)
{
	IsWindowMinimized = NewIsWindowMinimized;
}

bool RenderRuntimeGameInstanceSubsystem::GetIsWindowMinimized() const
{
	return IsWindowMinimized;
}

void RenderRuntimeGameInstanceSubsystem::CreateBackBuffer()
{
	if (SwapChain == nullptr || Device.Get() == nullptr)
	{
		return;
	}

	SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&BackBuffer));
	Device->CreateRenderTargetView(BackBuffer, nullptr, &BackBufferRenderView);

	D3D11_TEXTURE2D_DESC DepthDescription = {};
	DepthDescription.Width = static_cast<UINT>(ScreenWidth);
	DepthDescription.Height = static_cast<UINT>(ScreenHeight);
	DepthDescription.MipLevels = 1;
	DepthDescription.ArraySize = 1;
	DepthDescription.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	DepthDescription.SampleDesc.Count = 1;
	DepthDescription.Usage = D3D11_USAGE_DEFAULT;
	DepthDescription.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	Device->CreateTexture2D(&DepthDescription, nullptr, &BackBufferDepthTexture);
	Device->CreateDepthStencilView(BackBufferDepthTexture, nullptr, &BackBufferDepthStencilView);
}

void RenderRuntimeGameInstanceSubsystem::DestroyResources()
{
	ReleaseDearImGuiBackBufferCopyResources();

	if (BackBufferDepthStencilView != nullptr)
	{
		BackBufferDepthStencilView->Release();
		BackBufferDepthStencilView = nullptr;
	}
	if (BackBufferDepthTexture != nullptr)
	{
		BackBufferDepthTexture->Release();
		BackBufferDepthTexture = nullptr;
	}
	if (BackBufferRenderView != nullptr)
	{
		BackBufferRenderView->Release();
		BackBufferRenderView = nullptr;
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

bool RenderRuntimeGameInstanceSubsystem::EnsureDearImGuiBackBufferCopyResources()
{
	if (Device.Get() == nullptr || BackBuffer == nullptr || ScreenWidth <= 0 || ScreenHeight <= 0)
	{
		return false;
	}

	if (DearImGuiBackBufferCopyTexture != nullptr && DearImGuiBackBufferCopyShaderResourceView != nullptr)
	{
		return true;
	}

	ReleaseDearImGuiBackBufferCopyResources();

	D3D11_TEXTURE2D_DESC BackBufferCopyTextureDescription = {};
	BackBufferCopyTextureDescription.Width = static_cast<UINT>(ScreenWidth);
	BackBufferCopyTextureDescription.Height = static_cast<UINT>(ScreenHeight);
	BackBufferCopyTextureDescription.MipLevels = 1;
	BackBufferCopyTextureDescription.ArraySize = 1;
	BackBufferCopyTextureDescription.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	BackBufferCopyTextureDescription.SampleDesc.Count = 1;
	BackBufferCopyTextureDescription.Usage = D3D11_USAGE_DEFAULT;
	BackBufferCopyTextureDescription.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	if (FAILED(Device->CreateTexture2D(&BackBufferCopyTextureDescription, nullptr, &DearImGuiBackBufferCopyTexture)))
	{
		return false;
	}
	if (FAILED(Device->CreateShaderResourceView(DearImGuiBackBufferCopyTexture, nullptr, &DearImGuiBackBufferCopyShaderResourceView)))
	{
		ReleaseDearImGuiBackBufferCopyResources();
		return false;
	}

	return true;
}

void RenderRuntimeGameInstanceSubsystem::ReleaseDearImGuiBackBufferCopyResources()
{
	if (DearImGuiBackBufferCopyShaderResourceView != nullptr)
	{
		DearImGuiBackBufferCopyShaderResourceView->Release();
		DearImGuiBackBufferCopyShaderResourceView = nullptr;
	}
	if (DearImGuiBackBufferCopyTexture != nullptr)
	{
		DearImGuiBackBufferCopyTexture->Release();
		DearImGuiBackBufferCopyTexture = nullptr;
	}
}

void RenderRuntimeGameInstanceSubsystem::UpdateDearImGuiBackBufferCopyFromBackBuffer()
{
	if (Context == nullptr || BackBuffer == nullptr)
	{
		return;
	}
	if (EnsureDearImGuiBackBufferCopyResources() == false)
	{
		return;
	}

	Context->CopyResource(DearImGuiBackBufferCopyTexture, BackBuffer);
}
