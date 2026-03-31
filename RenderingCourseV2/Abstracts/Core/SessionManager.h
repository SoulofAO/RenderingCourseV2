#pragma once

#include "Abstracts/Core/GameConfigurator.h"
#include "Abstracts/Core/MultiGameSession.h"
#include <windows.h>
#include <memory>
#include <vector>

class Game;
class RenderRuntimeGameInstanceSubsystem;

class SessionManager
{
public:
	SessionManager();
	~SessionManager();

	int OpenGame(const GameConfigurator& NewGameConfigurator);
	void OpenMultipleGames(const std::vector<GameConfigurator>& NewGameConfigurators);
	void CloseGame(int SessionIdentifier);
	void SetActiveSessionIdentifier(int NewActiveSessionIdentifier);
	int GetActiveSessionIdentifier() const;
	void TickFrame(float DeltaTime);
	void RenderFrame(
		RenderRuntimeGameInstanceSubsystem* RenderRuntimeSubsystem,
		int ScreenWidth,
		int ScreenHeight);
	bool HandleMessage(HWND WindowHandle, UINT Message, WPARAM WParam, LPARAM LParam);
	void SetSessionActivePlayerIdentifier(int SessionIdentifier, int PlayerIdentifier);
	void SetPlayerActiveGameIdentifier(int SessionIdentifier, int PlayerIdentifier, int GameIdentifier);

private:
	struct ManagedGameEntry
	{
		int SessionIdentifier;
		int GameIdentifier;
		std::unique_ptr<Game> GameInstance;
	};

	std::vector<SessionGameView> BuildSessionGameViews(int SessionIdentifier);
	void RemoveManagedGamesForSession(int SessionIdentifier);

	int NextSessionIdentifier;
	int ActiveSessionIdentifier;
	std::vector<std::unique_ptr<MultiGameSession>> Sessions;
	std::vector<ManagedGameEntry> ManagedGames;
};
