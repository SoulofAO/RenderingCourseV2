#include "Abstracts/Components/FPSSpectateCameraComponentSettingsUI.h"
#include "Abstracts/Components/FPSSpectateCameraComponent.h"
#include "Abstracts/Core/Game.h"
#include "Planets/PlanetsGame.h"

#include <imgui.h>

FPSSpectateCameraComponentSettingsUI::FPSSpectateCameraComponentSettingsUI()
	: UIRenderingComponent()
	, TargetFPSSpectateCameraComponent(nullptr)
	, UseOrthographicProjection(false)
	, OrthographicWidth(8.0f)
	, OrthographicHeight(6.0f)
	, FieldOfViewDegrees(60.0f)
{
}

FPSSpectateCameraComponentSettingsUI::~FPSSpectateCameraComponentSettingsUI() = default;

void FPSSpectateCameraComponentSettingsUI::SetTargetFPSSpectateCameraComponent(FPSSpectateCameraComponent* NewTargetFPSSpectateCameraComponent)
{
	TargetFPSSpectateCameraComponent = NewTargetFPSSpectateCameraComponent;
}

void FPSSpectateCameraComponentSettingsUI::RenderUI()
{
	Game* OwningGame = GetOwningGame();
	if (OwningGame == nullptr)
	{
		return;
	}

	if (TargetFPSSpectateCameraComponent == nullptr || TargetFPSSpectateCameraComponent->GetIsPossessed() == false)
	{
		return;
	}

	if (OwningGame->GetDefaultCameraSettingsWindowVisible() == false)
	{
		return;
	}

	const ImVec2 DisplaySize = ImGui::GetIO().DisplaySize;
	const ImVec2 DefaultCameraWindowSize = ImVec2(360.0f, 220.0f);
	const ImVec2 DefaultCameraWindowPosition = ImVec2(
		DisplaySize.x - DefaultCameraWindowSize.x - 20.0f,
		DisplaySize.y - DefaultCameraWindowSize.y - 20.0f);

	ImGui::SetNextWindowPos(DefaultCameraWindowPosition, ImGuiCond_Always);
	ImGui::SetNextWindowSize(DefaultCameraWindowSize, ImGuiCond_Always);
	ImGui::Begin("Default Camera", nullptr, ImGuiWindowFlags_NoResize);

	float DefaultCameraMovementSpeedScale = TargetFPSSpectateCameraComponent->GetMovementSpeedScale();
	if (ImGui::SliderFloat("Movement Speed Scale", &DefaultCameraMovementSpeedScale, 0.1f, 5.0f))
	{
		TargetFPSSpectateCameraComponent->SetMovementSpeedScale(DefaultCameraMovementSpeedScale);
	}

	PlanetsGame* OwningPlanetsGame = GetOwningPlanetsGame();
	if (OwningPlanetsGame != nullptr)
	{
		UseOrthographicProjection = OwningPlanetsGame->GetUseOrthographicProjectionForActiveCamera();
		if (ImGui::Checkbox("Use Orthographic Projection", &UseOrthographicProjection))
		{
			OwningPlanetsGame->SetUseOrthographicProjectionForActiveCamera(UseOrthographicProjection);
		}
	}

	OrthographicWidth = TargetFPSSpectateCameraComponent->GetOrthographicWidth();
	OrthographicHeight = TargetFPSSpectateCameraComponent->GetOrthographicHeight();
	if (ImGui::SliderFloat("Orthographic Width", &OrthographicWidth, 0.1f, 200.0f))
	{
		TargetFPSSpectateCameraComponent->SetOrthographicSize(OrthographicWidth, OrthographicHeight);
	}

	if (ImGui::SliderFloat("Orthographic Height", &OrthographicHeight, 0.1f, 200.0f))
	{
		TargetFPSSpectateCameraComponent->SetOrthographicSize(OrthographicWidth, OrthographicHeight);
	}

	FieldOfViewDegrees = TargetFPSSpectateCameraComponent->GetFieldOfViewDegrees();
	if (ImGui::SliderFloat("FOV", &FieldOfViewDegrees, 5.0f, 175.0f))
	{
		TargetFPSSpectateCameraComponent->SetFieldOfViewDegrees(FieldOfViewDegrees);
	}

	ImGui::End();

}

PlanetsGame* FPSSpectateCameraComponentSettingsUI::GetOwningPlanetsGame() const
{
	Game* OwningGame = GetOwningGame();
	if (OwningGame == nullptr)
	{
		return nullptr;
	}

	return dynamic_cast<PlanetsGame*>(OwningGame);
}
