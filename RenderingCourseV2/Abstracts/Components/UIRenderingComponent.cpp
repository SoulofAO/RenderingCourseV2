#include "Abstracts/Components/UIRenderingComponent.h"
#include "Abstracts/Core/Game.h"
#include "Abstracts/Subsystems/DisplayWin32.h"
#include "Abstracts/Subsystems/SceneViewportSubsystem.h"

#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND WindowHandle, UINT Message, WPARAM WParam, LPARAM LParam);

UIRenderingComponent::UIRenderingComponent()
	: RenderingComponent()
	, IsDearImGuiInitialized(false)
{
	SetRenderOrder(1000);
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

	Game* OwningGame = GetOwningGame();
	if (OwningGame == nullptr)
	{
		return;
	}

	SceneViewportSubsystem* SceneViewport = OwningGame->GetSubsystem<SceneViewportSubsystem>();
	if (SceneViewport == nullptr)
	{
		return;
	}

	DisplayWin32* Display = SceneViewport->GetDisplay();
	if (Display == nullptr || SceneViewport->GetDevice() == nullptr || SceneViewport->GetDeviceContext() == nullptr)
	{
		return;
	}

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(Display->GetWindowHandle());
	ImGui_ImplDX11_Init(SceneViewport->GetDevice(), SceneViewport->GetDeviceContext());
	IsDearImGuiInitialized = true;
}

void UIRenderingComponent::Update(float DeltaTime)
{
	RenderingComponent::Update(DeltaTime);
}

void UIRenderingComponent::Render(SceneViewportSubsystem* SceneViewport)
{
	if (SceneViewport == nullptr || IsDearImGuiInitialized == false)
	{
		return;
	}

	ImGui_ImplWin32_NewFrame();
	ImGui_ImplDX11_NewFrame();
	ImGui::NewFrame();

	RenderUI();

	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

void UIRenderingComponent::Shutdown()
{
	if (GetIsInitialized() == false)
	{
		return;
	}

	if (IsDearImGuiInitialized)
	{
		ImGui_ImplWin32_Shutdown();
		ImGui_ImplDX11_Shutdown();
		ImGui::DestroyContext();
		IsDearImGuiInitialized = false;
	}

	RenderingComponent::Shutdown();
}

bool UIRenderingComponent::HandleMessage(HWND WindowHandle, UINT Message, WPARAM WParam, LPARAM LParam)
{
	if (IsDearImGuiInitialized)
	{
		if (ImGui_ImplWin32_WndProcHandler(WindowHandle, Message, WParam, LParam))
		{
			return true;
		}
	}

	return false;
}
