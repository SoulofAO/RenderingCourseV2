#include "Planets/PlanetsUIRenderingComponent.h"
#include "Planets/PlanetsGame.h"
#include "Abstracts/Core/Game.h"

#include <imgui.h>

PlanetsUIRenderingComponent::PlanetsUIRenderingComponent()
	: UIRenderingComponent()
{
}

PlanetsUIRenderingComponent::~PlanetsUIRenderingComponent() = default;

void PlanetsUIRenderingComponent::RenderUI()
{
	PlanetsGame* OwningPlanetsGame = GetOwningPlanetsGame();
	if (OwningPlanetsGame == nullptr)
	{
		return;
	}

	ImGui::SetNextWindowPos(ImVec2(20.0f, 20.0f), ImGuiCond_Once);
	ImGui::SetNextWindowSize(ImVec2(360.0f, 280.0f), ImGuiCond_Once);
	ImGui::Begin("Planets Controls");

	bool UseOrthographicProjectionForActiveCamera = OwningPlanetsGame->GetUseOrthographicProjectionForActiveCamera();
	if (ImGui::Checkbox("Use Orthographic Projection", &UseOrthographicProjectionForActiveCamera))
	{
		OwningPlanetsGame->SetUseOrthographicProjectionForActiveCamera(UseOrthographicProjectionForActiveCamera);
	}

	float PlanetOrbitSpeedScale = OwningPlanetsGame->GetPlanetOrbitSpeedScale();
	if (ImGui::SliderFloat("Planet Orbit Speed", &PlanetOrbitSpeedScale, 0.0f, 4.0f))
	{
		OwningPlanetsGame->SetPlanetOrbitSpeedScale(PlanetOrbitSpeedScale);
	}

	float MoonOrbitSpeedScale = OwningPlanetsGame->GetMoonOrbitSpeedScale();
	if (ImGui::SliderFloat("Moon Orbit Speed", &MoonOrbitSpeedScale, 0.0f, 6.0f))
	{
		OwningPlanetsGame->SetMoonOrbitSpeedScale(MoonOrbitSpeedScale);
	}

	float PlanetOrbitRadiusScale = OwningPlanetsGame->GetPlanetOrbitRadiusScale();
	if (ImGui::SliderFloat("Planet Orbit Radius", &PlanetOrbitRadiusScale, 0.2f, 3.0f))
	{
		OwningPlanetsGame->SetPlanetOrbitRadiusScale(PlanetOrbitRadiusScale);
	}

	float MoonOrbitRadiusScale = OwningPlanetsGame->GetMoonOrbitRadiusScale();
	if (ImGui::SliderFloat("Moon Orbit Radius", &MoonOrbitRadiusScale, 0.2f, 3.0f))
	{
		OwningPlanetsGame->SetMoonOrbitRadiusScale(MoonOrbitRadiusScale);
	}

	ImGui::Spacing();
	ImGui::Text("Hotkeys:");
	ImGui::BulletText("C - Switch camera (FPS / Orbit)");
	ImGui::BulletText("K - Toggle fallback camera settings");
	ImGui::BulletText("WASD + Mouse - Move FPS camera");
	ImGui::BulletText("Q/E - Vertical move for FPS camera");

	ImGui::End();
}

PlanetsGame* PlanetsUIRenderingComponent::GetOwningPlanetsGame() const
{
	Game* OwningGame = GetOwningGame();
	if (OwningGame == nullptr)
	{
		return nullptr;
	}

	return dynamic_cast<PlanetsGame*>(OwningGame);
}
