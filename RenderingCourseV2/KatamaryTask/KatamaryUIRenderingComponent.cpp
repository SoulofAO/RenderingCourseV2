#include "KatamaryTask/KatamaryUIRenderingComponent.h"
#include "KatamaryTask/KatamaryGame.h"
#include <algorithm>

#include <imgui.h>

KatamaryUIRenderingComponent::KatamaryUIRenderingComponent()
	: UIRenderingComponent()
	, UseWeldCollectMode(false)
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
	UseWeldCollectMode = OwningKatamaryGame->GetUseWeldCollectMode();

	ImGui::SetNextWindowPos(ImVec2(20.0f, 20.0f), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(320.0f, 170.0f), ImGuiCond_Always);
	ImGui::Begin("Katamary HUD", nullptr, ImGuiWindowFlags_NoResize);
	ImGui::Text("Time Left: %.1f s", RemainingTimeSecondsValue);
	ImGui::Text("Collected: %d", CollectedItemCount);
	if (ImGui::Checkbox("Use Weld Collect Mode", &UseWeldCollectMode))
	{
		OwningKatamaryGame->SetUseWeldCollectMode(UseWeldCollectMode);
	}
	if (UseWeldCollectMode)
	{
		ImGui::TextUnformatted("Player sphere growth is disabled.");
	}
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
