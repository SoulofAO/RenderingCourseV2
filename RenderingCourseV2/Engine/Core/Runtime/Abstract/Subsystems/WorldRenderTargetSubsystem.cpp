#include "Engine/Core/Runtime/Abstract/Subsystems/WorldRenderTargetSubsystem.h"
#include "Engine/Core/Runtime/Abstract/Core/Game.h"
#include "Engine/Core/Runtime/Abstract/Core/World.h"
#include "Engine/Core/Runtime/Abstract/Subsystems/SceneViewportSubsystem.h"

WorldRenderTargetSubsystem::WorldRenderTargetSubsystem()
	: Subsystem(SubsystemCategory::World)
	, RenderTexture(nullptr)
	, RenderView(nullptr)
	, ShaderResourceView(nullptr)
{
}

WorldRenderTargetSubsystem::~WorldRenderTargetSubsystem()
{
	Shutdown();
}

void WorldRenderTargetSubsystem::Initialize()
{
	if (GetIsInitialized())
	{
		return;
	}

	Subsystem::Initialize();
}

void WorldRenderTargetSubsystem::Shutdown()
{
	if (GetIsInitialized() == false)
	{
		return;
	}

	DestroyResources();
	Subsystem::Shutdown();
}

void WorldRenderTargetSubsystem::BeginRenderPass(SceneViewportSubsystem* SceneViewport, float TotalTimeSeconds)
{
	if (SceneViewport == nullptr || SceneViewport->GetDevice() == nullptr || SceneViewport->GetDeviceContext() == nullptr)
	{
		return;
	}

	if (RenderView == nullptr || RenderTexture == nullptr || ShaderResourceView == nullptr)
	{
		CreateResources(SceneViewport);
	}

	if (RenderView == nullptr)
	{
		return;
	}

	ID3D11DeviceContext* DeviceContext = SceneViewport->GetDeviceContext();
	DeviceContext->OMSetRenderTargets(1, &RenderView, nullptr);

	D3D11_VIEWPORT Viewport = {};
	Viewport.Width = static_cast<float>(SceneViewport->GetScreenWidth());
	Viewport.Height = static_cast<float>(SceneViewport->GetScreenHeight());
	Viewport.TopLeftX = 0.0f;
	Viewport.TopLeftY = 0.0f;
	Viewport.MinDepth = 0.0f;
	Viewport.MaxDepth = 1.0f;
	DeviceContext->RSSetViewports(1, &Viewport);

	float ColorCycle = TotalTimeSeconds;
	while (ColorCycle > 1.0f)
	{
		ColorCycle -= 1.0f;
	}

	float ClearColor[] = { 0.02f + ColorCycle * 0.05f, 0.02f, 0.02f, 1.0f };
	DeviceContext->ClearRenderTargetView(RenderView, ClearColor);
}

void WorldRenderTargetSubsystem::EndRenderPass(SceneViewportSubsystem* SceneViewport)
{
	(void)SceneViewport;
}

ID3D11ShaderResourceView* WorldRenderTargetSubsystem::GetShaderResourceView() const
{
	return ShaderResourceView;
}

void WorldRenderTargetSubsystem::CreateResources(SceneViewportSubsystem* SceneViewport)
{
	ID3D11Device* Device = SceneViewport->GetDevice();
	if (Device == nullptr)
	{
		return;
	}

	D3D11_TEXTURE2D_DESC TextureDescription = {};
	TextureDescription.Width = static_cast<UINT>(SceneViewport->GetScreenWidth());
	TextureDescription.Height = static_cast<UINT>(SceneViewport->GetScreenHeight());
	TextureDescription.MipLevels = 1;
	TextureDescription.ArraySize = 1;
	TextureDescription.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	TextureDescription.SampleDesc.Count = 1;
	TextureDescription.SampleDesc.Quality = 0;
	TextureDescription.Usage = D3D11_USAGE_DEFAULT;
	TextureDescription.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

	if (FAILED(Device->CreateTexture2D(&TextureDescription, nullptr, &RenderTexture)))
	{
		RenderTexture = nullptr;
		return;
	}

	if (FAILED(Device->CreateRenderTargetView(RenderTexture, nullptr, &RenderView)))
	{
		DestroyResources();
		return;
	}

	if (FAILED(Device->CreateShaderResourceView(RenderTexture, nullptr, &ShaderResourceView)))
	{
		DestroyResources();
	}
}

void WorldRenderTargetSubsystem::DestroyResources()
{
	if (ShaderResourceView != nullptr)
	{
		ShaderResourceView->Release();
		ShaderResourceView = nullptr;
	}

	if (RenderView != nullptr)
	{
		RenderView->Release();
		RenderView = nullptr;
	}

	if (RenderTexture != nullptr)
	{
		RenderTexture->Release();
		RenderTexture = nullptr;
	}
}
