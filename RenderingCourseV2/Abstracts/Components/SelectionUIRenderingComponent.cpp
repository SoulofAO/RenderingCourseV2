#include "Abstracts/Components/SelectionUIRenderingComponent.h"
#include "Abstracts/Core/SelectionGame.h"
#include "Abstracts/Core/Game.h"

#include <imgui.h>

SelectionUIRenderingComponent::SelectionUIRenderingComponent()
	: UIRenderingComponent()
{
}

SelectionUIRenderingComponent::~SelectionUIRenderingComponent() = default;

void SelectionUIRenderingComponent::RenderUI()
{
	SelectionGame* OwningSelectionGame = GetOwningSelectionGame();
	if (OwningSelectionGame == nullptr)
	{
		return;
	}

	const std::vector<SelectionGameEntry>& AvailableGameSelections = OwningSelectionGame->GetAvailableGameSelections();

	ImGuiViewport* MainViewport = ImGui::GetMainViewport();
	if (MainViewport != nullptr)
	{
		const ImVec2 SelectionWindowPosition(
			MainViewport->Pos.x + MainViewport->Size.x * 0.5f,
			MainViewport->Pos.y + MainViewport->Size.y * 0.5f);
		ImGui::SetNextWindowPos(SelectionWindowPosition, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
	}

	ImGui::SetNextWindowSize(ImVec2(420.0f, 280.0f), ImGuiCond_Once);
	const ImGuiWindowFlags WindowFlags =
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoResize;
	if (ImGui::Begin("Game Selection", nullptr, WindowFlags))
	{
		ImGui::TextUnformatted("Choose game to start");
		ImGui::Separator();

		for (size_t SelectionIndex = 0; SelectionIndex < AvailableGameSelections.size(); ++SelectionIndex)
		{
			const SelectionGameEntry& ExistingSelection = AvailableGameSelections[SelectionIndex];
			if (ImGui::Button(ExistingSelection.GameDisplayName.c_str(), ImVec2(-1.0f, 0.0f)))
			{
				OwningSelectionGame->RequestOpenGameBySelectionIndex(SelectionIndex);
			}
		}
	}
	ImGui::End();
}

SelectionGame* SelectionUIRenderingComponent::GetOwningSelectionGame() const
{
	Game* OwningGame = GetOwningGame();
	if (OwningGame == nullptr)
	{
		return nullptr;
	}

	return dynamic_cast<SelectionGame*>(OwningGame);
}
