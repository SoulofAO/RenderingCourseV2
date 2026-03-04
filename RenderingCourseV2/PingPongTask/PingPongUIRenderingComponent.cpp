#include "PingPongUIRenderingComponent.h"
#include "PingPongGame.h"
#include <algorithm>
#include <directxmath.h>

#include "imgui.h"

PingPongUIRenderingComponent::PingPongUIRenderingComponent()
	: UIRenderingComponent()
{
}

PingPongUIRenderingComponent::~PingPongUIRenderingComponent() = default;

void PingPongUIRenderingComponent::RenderUI()
{
	PingPongGame* OwningPingPongGame = GetOwningPingPongGame();
	if (OwningPingPongGame == nullptr)
	{
		return;
	}

	ImGui::SetNextWindowPos(ImVec2(20.0f, 20.0f), ImGuiCond_Once);
	ImGui::SetNextWindowSize(ImVec2(260.0f, 120.0f), ImGuiCond_Once);
	ImGui::Begin("PingPong Debug");
	if (ImGui::Button("Reset Ball"))
	{
		OwningPingPongGame->ResetBallFromUI();
	}

	bool IsBounceAccelerationEnabled = OwningPingPongGame->GetBounceAccelerationEnabled();
	if (ImGui::Checkbox("Increase speed every bounce", &IsBounceAccelerationEnabled))
	{
		OwningPingPongGame->SetBounceAccelerationEnabledFromUI(IsBounceAccelerationEnabled);
	}
	ImGui::End();

	const ImVec2 DisplaySize = ImGui::GetIO().DisplaySize;
	const float ScoreWindowWidth = 200.0f;
	const float ScoreWindowHeight = 80.0f;

	ImGui::SetNextWindowPos(ImVec2(20.0f, 160.0f), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(ScoreWindowWidth, ScoreWindowHeight), ImGuiCond_Always);
	ImGui::Begin("Player Score", nullptr, ImGuiWindowFlags_NoResize);
	ImGui::Text("Player");
	ImGui::Separator();
	ImGui::Text("%d", OwningPingPongGame->GetPlayerVictoryCount());
	ImGui::End();

	const float RightWindowPositionX = (std::max)(20.0f, DisplaySize.x - ScoreWindowWidth - 20.0f);
	ImGui::SetNextWindowPos(ImVec2(RightWindowPositionX, 160.0f), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(ScoreWindowWidth, ScoreWindowHeight), ImGuiCond_Always);
	ImGui::Begin("Computer Score", nullptr, ImGuiWindowFlags_NoResize);
	ImGui::Text("Computer");
	ImGui::Separator();
	ImGui::Text("%d", OwningPingPongGame->GetComputerVictoryCount());
	ImGui::End();
}

PingPongGame* PingPongUIRenderingComponent::GetOwningPingPongGame() const
{
	Game* OwningGame = GetOwningGame();
	if (OwningGame == nullptr)
	{
		return nullptr;
	}

	return dynamic_cast<PingPongGame*>(OwningGame);
}
