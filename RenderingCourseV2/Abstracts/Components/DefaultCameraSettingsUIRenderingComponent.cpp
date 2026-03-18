#include "Abstracts/Components/DefaultCameraSettingsUIRenderingComponent.h"
#include "Abstracts/Core/Game.h"

#include <imgui.h>

DefaultCameraSettingsUIRenderingComponent::DefaultCameraSettingsUIRenderingComponent()
	: UIRenderingComponent()
{
}

DefaultCameraSettingsUIRenderingComponent::~DefaultCameraSettingsUIRenderingComponent() = default;

void DefaultCameraSettingsUIRenderingComponent::RenderUI()
{
	Game* OwningGame = GetOwningGame();
	if (OwningGame == nullptr)
	{
		return;
	}

	if (OwningGame->GetIsFallbackCameraPossessed() == false)
	{
		return;
	}

	if (OwningGame->GetDefaultCameraSettingsWindowVisible() == false)
	{
		return;
	}

	const ImVec2 DisplaySize = ImGui::GetIO().DisplaySize;
	const ImVec2 DefaultCameraWindowSize = ImVec2(320.0f, 110.0f);
	const ImVec2 DefaultCameraWindowPosition = ImVec2(
		DisplaySize.x - DefaultCameraWindowSize.x - 20.0f,
		DisplaySize.y - DefaultCameraWindowSize.y - 20.0f);

	bool IsDefaultCameraSettingsWindowVisible = OwningGame->GetDefaultCameraSettingsWindowVisible();
	ImGui::SetNextWindowPos(DefaultCameraWindowPosition, ImGuiCond_Always);
	ImGui::SetNextWindowSize(DefaultCameraWindowSize, ImGuiCond_Always);
	ImGui::Begin("Default Camera", &IsDefaultCameraSettingsWindowVisible, ImGuiWindowFlags_NoResize);

	float DefaultCameraMovementSpeedScale = OwningGame->GetDefaultCameraMovementSpeedScale();
	if (ImGui::SliderFloat("Movement Speed Scale", &DefaultCameraMovementSpeedScale, 0.1f, 5.0f))
	{
		OwningGame->SetDefaultCameraMovementSpeedScale(DefaultCameraMovementSpeedScale);
	}

	ImGui::End();

	if (OwningGame->GetDefaultCameraSettingsWindowVisible() != IsDefaultCameraSettingsWindowVisible)
	{
		OwningGame->SetDefaultCameraSettingsWindowVisible(IsDefaultCameraSettingsWindowVisible);
	}
}
