#include "Abstracts/Subsystems/SceneViewportSubsystem.h"
#include "Abstracts/Subsystems/DisplayWin32.h"
#include "Abstracts/Core/Game.h"
#include <iostream>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")

SceneViewportSubsystem::SceneViewportSubsystem()
	: Context(nullptr)
	, SwapChain(nullptr)
	, BackBuffer(nullptr)
	, RenderView(nullptr)
	, DepthTexture(nullptr)
	, DepthStencilView(nullptr)
	, CameraWorldPosition(0.0f, 0.0f, 0.0f)
	, DirectionalLightDirection(0.0f, -1.0f, 0.0f)
	, DirectionalLightColor(1.0f, 1.0f, 1.0f, 1.0f)
	, DirectionalLightIntensity(1.0f)
	, UseFullBrightnessWithoutLighting(0.0f)
	, CurrentRenderPipelineType(RenderPipelineType::Forward)
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

	DeferredRendererInstance = std::make_unique<DeferredRenderer>();
	DeferredRendererInstance->Initialize(Device.Get());
	DeferredRendererInstance->EnsureTargets(Device.Get(), GetScreenWidth(), GetScreenHeight());
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
		float ClearColor[] = { 0.0, 0.0f, 0.0f, 1.0f };
		Context->ClearRenderTargetView(RenderView, ClearColor);
	}

	if (DepthStencilView != nullptr)
	{
		Context->ClearDepthStencilView(DepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	}

	Context->ClearState();
	if (CurrentRenderPipelineType == RenderPipelineType::Forward)
	{
		RestoreTargets();
	}
	else if (DeferredRendererInstance != nullptr)
	{
		DeferredRendererInstance->EnsureTargets(Device.Get(), GetScreenWidth(), GetScreenHeight());
	}

	D3D11_VIEWPORT Viewport = {};
	Viewport.Width = static_cast<float>(GetOwningGame()->GetScreenWidth());
	Viewport.Height = static_cast<float>(GetOwningGame()->GetScreenHeight());
	Viewport.TopLeftX = 0.0f;
	Viewport.TopLeftY = 0.0f;
	Viewport.MinDepth = 0.0f;
	Viewport.MaxDepth = 1.0f;
	Context->RSSetViewports(1, &Viewport);
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

float SceneViewportSubsystem::GetUseFullBrightnessWithoutLighting() const
{
	return UseFullBrightnessWithoutLighting;
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

void SceneViewportSubsystem::SetRenderPipelineType(RenderPipelineType NewRenderPipelineType)
{
	CurrentRenderPipelineType = NewRenderPipelineType;
}

RenderPipelineType SceneViewportSubsystem::GetRenderPipelineType() const
{
	return CurrentRenderPipelineType;
}

bool SceneViewportSubsystem::IsDeferredRenderingEnabled() const
{
	return CurrentRenderPipelineType == RenderPipelineType::Deferred;
}

void SceneViewportSubsystem::BeginGeometryPass()
{
	if (CurrentRenderPipelineType == RenderPipelineType::Deferred && DeferredRendererInstance != nullptr)
	{
		DeferredRendererInstance->BeginGeometryPass(Context);
	}
	else
	{
		RestoreTargets();
	}
}

void SceneViewportSubsystem::EndGeometryPass()
{
	if (CurrentRenderPipelineType == RenderPipelineType::Deferred && DeferredRendererInstance != nullptr)
	{
		DeferredRendererInstance->EndGeometryPass(Context);
	}
}

void SceneViewportSubsystem::ExecuteDeferredLightingPass()
{
	if (CurrentRenderPipelineType != RenderPipelineType::Deferred || DeferredRendererInstance == nullptr)
	{
		return;
	}

	const DirectX::XMMATRIX ViewMatrix = GetViewMatrix();
	const DirectX::XMMATRIX ProjectionMatrix = GetProjectionMatrix();
	const DirectX::XMMATRIX ViewProjectionMatrix = ViewMatrix * ProjectionMatrix;
	const DirectX::XMMATRIX InverseViewProjectionMatrix = DirectX::XMMatrixInverse(nullptr, ViewProjectionMatrix);

	DeferredRendererInstance->RenderLightingPass(
		Context,
		RenderView,
		InverseViewProjectionMatrix,
		CameraWorldPosition,
		DirectionalLightDirection,
		DirectionalLightColor,
		DirectionalLightIntensity,
		UseFullBrightnessWithoutLighting);
}

void SceneViewportSubsystem::CreateBackBuffer()
{
	if (SwapChain == nullptr || Device.Get() == nullptr)
	{
		return;
	}

	SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&BackBuffer));
	Device->CreateRenderTargetView(BackBuffer, nullptr, &RenderView);

	D3D11_TEXTURE2D_DESC DepthDescription = {};
	DepthDescription.Width = static_cast<UINT>(GetScreenWidth());
	DepthDescription.Height = static_cast<UINT>(GetScreenHeight());
	DepthDescription.MipLevels = 1;
	DepthDescription.ArraySize = 1;
	DepthDescription.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	DepthDescription.SampleDesc.Count = 1;
	DepthDescription.Usage = D3D11_USAGE_DEFAULT;
	DepthDescription.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	Device->CreateTexture2D(&DepthDescription, nullptr, &DepthTexture);
	Device->CreateDepthStencilView(DepthTexture, nullptr, &DepthStencilView);
}

void SceneViewportSubsystem::RestoreTargets()
{
	if (Context == nullptr || RenderView == nullptr)
	{
		return;
	}

	Context->OMSetRenderTargets(1, &RenderView, DepthStencilView);
}

void SceneViewportSubsystem::DestroyResources()
{
	if (DepthStencilView != nullptr)
	{
		DepthStencilView->Release();
		DepthStencilView = nullptr;
	}

	if (DepthTexture != nullptr)
	{
		DepthTexture->Release();
		DepthTexture = nullptr;
	}

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

	if (DeferredRendererInstance != nullptr)
	{
		DeferredRendererInstance->Shutdown();
		DeferredRendererInstance.reset();
	}
}
