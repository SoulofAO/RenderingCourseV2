#include "Abstracts/Core/SelectionGame.h"
#include "Abstracts/Components/SelectionUIRenderingComponent.h"
#include "Abstracts/Core/Actor.h"
#include "Abstracts/Core/GameInstance.h"

#include <memory>

SelectionGame::SelectionGame(LPCWSTR ApplicationName, int ScreenWidth, int ScreenHeight)
	: Game(ApplicationName, ScreenWidth, ScreenHeight)
	, SessionIdentifier(1)
	, PlayerIdentifier(1)
	, SelectionUIRenderingComponentInstance(nullptr)
{
}

SelectionGame::~SelectionGame() = default;

void SelectionGame::SetSelectionContext(int NewSessionIdentifier, int NewPlayerIdentifier)
{
	SessionIdentifier = NewSessionIdentifier;
	PlayerIdentifier = NewPlayerIdentifier;
}

void SelectionGame::SetAvailableGameSelections(const std::vector<SelectionGameEntry>& NewAvailableGameSelections)
{
	AvailableGameSelections = NewAvailableGameSelections;
}

const std::vector<SelectionGameEntry>& SelectionGame::GetAvailableGameSelections() const
{
	return AvailableGameSelections;
}

void SelectionGame::RequestOpenGameBySelectionIndex(size_t SelectionIndex)
{
	GameInstance* OwningGameInstance = GetOwningGameInstance();
	if (OwningGameInstance == nullptr)
	{
		return;
	}
	if (SelectionIndex >= AvailableGameSelections.size())
	{
		return;
	}

	OwningGameInstance->QueueSessionReplacement(
		SessionIdentifier,
		AvailableGameSelections[SelectionIndex].GameConfigurator);
}

void SelectionGame::BeginPlay()
{
	Game::BeginPlay();

	SetGridFloorEnabled(false);
	SetDefaultCameraSettingsWindowVisible(false);
	SetMouseInputMode(MouseInputMode::UIOnly);

	if (SelectionUIRenderingComponentInstance == nullptr)
	{
		std::unique_ptr<Actor> NewUserInterfaceActor = std::make_unique<Actor>();
		std::unique_ptr<SelectionUIRenderingComponent> NewSelectionUIRenderingComponent = std::make_unique<SelectionUIRenderingComponent>();
		SelectionUIRenderingComponentInstance = NewSelectionUIRenderingComponent.get();
		NewUserInterfaceActor->AddComponent(std::move(NewSelectionUIRenderingComponent));
		AddActor(std::move(NewUserInterfaceActor));
	}
}
