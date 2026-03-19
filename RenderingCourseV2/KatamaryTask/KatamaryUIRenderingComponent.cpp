#include "KatamaryTask/KatamaryUIRenderingComponent.h"
#include "KatamaryTask/KatamaryGame.h"
#include <algorithm>

#include <imgui.h>

KatamaryUIRenderingComponent::KatamaryUIRenderingComponent()
	: UIRenderingComponent()
{
}

KatamaryUIRenderingComponent::~KatamaryUIRenderingComponent() = default;

void KatamaryUIRenderingComponent::RenderUI()
{
	KatamaryGame* OwningKatamaryGame = GetOwningKatamaryGame();
	if (OwningKatamaryGame == nullptr)
	{
		return;
	}

	const float RemainingTimeSecondsValue = (std::max)(0.0f, OwningKatamaryGame->GetRemainingTimeSeconds());
	const int CollectedItemCount = OwningKatamaryGame->GetCollectedItemCount();
	const bool IsRoundFinished = OwningKatamaryGame->GetIsRoundFinished();

	ImGui::SetNextWindowPos(ImVec2(20.0f, 20.0f), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(280.0f, 120.0f), ImGuiCond_Always);
	ImGui::Begin("Katamary HUD", nullptr, ImGuiWindowFlags_NoResize);
	ImGui::Text("Time Left: %.1f s", RemainingTimeSecondsValue);
	ImGui::Text("Collected: %d", CollectedItemCount);
	if (IsRoundFinished)
	{
		ImGui::Separator();
		ImGui::Text("Round Finished");
	}
	ImGui::End();
}

KatamaryGame* KatamaryUIRenderingComponent::GetOwningKatamaryGame() const
{
	Game* OwningGame = GetOwningGame();
	if (OwningGame == nullptr)
	{
		return nullptr;
	}

	return dynamic_cast<KatamaryGame*>(OwningGame);
}
