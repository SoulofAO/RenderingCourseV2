#pragma once

#include "Abstracts/Core/GameConfigurator.h"
#include "Abstracts/Core/Game.h"
#include <string>
#include <vector>

class SelectionUIRenderingComponent;

struct SelectionGameEntry
{
	std::string GameDisplayName;
	GameConfigurator GameConfigurator;
};

class SelectionGame : public Game
{
public:
	SelectionGame(LPCWSTR ApplicationName, int ScreenWidth, int ScreenHeight);
	~SelectionGame() override;

	void SetSelectionContext(int NewSessionIdentifier, int NewPlayerIdentifier);
	void SetAvailableGameSelections(const std::vector<SelectionGameEntry>& NewAvailableGameSelections);
	const std::vector<SelectionGameEntry>& GetAvailableGameSelections() const;
	void RequestOpenGameBySelectionIndex(size_t SelectionIndex);

protected:
	void BeginPlay() override;

private:
	int SessionIdentifier;
	int PlayerIdentifier;
	std::vector<SelectionGameEntry> AvailableGameSelections;
	SelectionUIRenderingComponent* SelectionUIRenderingComponentInstance;
};
