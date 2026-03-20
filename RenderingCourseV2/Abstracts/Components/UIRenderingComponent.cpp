#include "Abstracts/Components/UIRenderingComponent.h"
#include "Abstracts/Core/Game.h"
#include "Abstracts/Rendering/RenderProxy/UIRenderingDeferredRendererProxyObject.h"
#include "Abstracts/Rendering/RenderProxy/UIRenderingForwardRendererProxyObject.h"
#include "Abstracts/Subsystems/SceneViewportSubsystem.h"
#include <memory>

UIRenderingComponent::UIRenderingComponent()
	: RenderingComponent()
{
	SetRenderOrder(1000);
	SetForwardRendererProxyObject(std::make_unique<UIRenderingForwardRendererProxyObject>(this));
	SetDeferredRendererProxyObject(std::make_unique<UIRenderingDeferredRendererProxyObject>(this));
}

UIRenderingComponent::~UIRenderingComponent()
{
	Shutdown();
}

void UIRenderingComponent::Initialize()
{
	if (GetIsInitialized())
	{
		return;
	}

	RenderingComponent::Initialize();
}

void UIRenderingComponent::Update(float DeltaTime)
{
	RenderingComponent::Update(DeltaTime);
}

void UIRenderingComponent::Shutdown()
{
	if (GetIsInitialized() == false)
	{
		return;
	}

	RenderingComponent::Shutdown();
}

bool UIRenderingComponent::HandleMessage(HWND WindowHandle, UINT Message, WPARAM WParam, LPARAM LParam)
{
	Game* OwningGame = GetOwningGame();
	if (OwningGame == nullptr)
	{
		return false;
	}

	SceneViewportSubsystem* SceneViewport = OwningGame->GetSubsystem<SceneViewportSubsystem>();
	if (SceneViewport == nullptr)
	{
		return false;
	}

	return SceneViewport->HandleDearImGuiMessage(WindowHandle, Message, WParam, LParam);
}
