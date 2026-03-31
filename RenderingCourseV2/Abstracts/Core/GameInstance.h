#pragma once

#include "Abstracts/Core/SessionManager.h"
#include "Abstracts/Core/PlayerRenderTargetService.h"
#include "Abstracts/Subsystems/SceneViewportSubsystem.h"
#include <memory>
#include <vector>
#include <chrono>

class GameInstance
{
public:
	GameInstance();
	~GameInstance();

	void Initialize(LPCWSTR ApplicationName, int ScreenWidth, int ScreenHeight);
	int OpenGame(const GameConfigurator& NewGameConfigurator);
	void OpenMultipleGames(const std::vector<GameConfigurator>& NewGameConfigurators);
	void CloseGame(int SessionIdentifier);
	void Run();
	LRESULT MessageHandler(HWND WindowHandle, UINT Message, WPARAM WParam, LPARAM LParam);

private:
	LPCWSTR ApplicationName;
	int ScreenWidth;
	int ScreenHeight;
	SessionManager SessionManagerInstance;
	std::unique_ptr<SceneViewportSubsystem> SceneViewportSubsystemInstance;
	PlayerRenderTargetService PlayerRenderTargetServiceInstance;
	bool IsExitRequested;
	std::chrono::time_point<std::chrono::steady_clock> PreviousTime;
	float TotalRunTimeSeconds;
};
