#pragma once

#include "Abstracts/Core/GameConfigurator.h"
#include "Abstracts/Core/PlayerObject.h"
#include "Abstracts/Core/PlayerRenderTargetService.h"
#include "Abstracts/Core/RenderTypes.h"
#include <memory>
#include <vector>

class Game;
class RenderRuntimeGameInstanceSubsystem;

struct SessionGameView
{
	int GameIdentifier;
	Game* GameInstance;
};

class MultiGameSession
{
public:
	MultiGameSession(int NewSessionIdentifier, const GameConfigurator& NewGameConfigurator);
	~MultiGameSession();

	int GetSessionIdentifier() const;
	const std::wstring& GetSessionName() const;
	bool GetReceiveInputWhenInactive() const;
	void Initialize();
	void TickFrame(float DeltaTime, bool IsSessionActive, const std::vector<SessionGameView>& SessionGames);
	void RenderFrame(
		RenderRuntimeGameInstanceSubsystem* RenderRuntimeSubsystem,
		int ScreenWidth,
		int ScreenHeight,
		const std::vector<SessionGameView>& SessionGames,
		std::vector<PlayerRenderTargetCompositeCommand>& OutCompositeCommands);
	bool HandleMessage(HWND WindowHandle, UINT Message, WPARAM WParam, LPARAM LParam, bool IsSessionActive, const std::vector<SessionGameView>& SessionGames);
	bool BuildGameDefinitions(std::vector<GameDefinition>& OutGameDefinitions) const;
	void SetPlayerActiveGameIdentifier(int PlayerIdentifier, int NewActiveGameIdentifier);
	void SetActivePlayerIdentifier(int NewActivePlayerIdentifier);
	int GetActivePlayerIdentifier() const;
	void Shutdown();

private:
	D3D11_VIEWPORT BuildPlayerViewport(int ScreenWidth, int ScreenHeight, const ViewportRectangleNormalized& ViewportRectangle) const;
	Game* FindGameByIdentifier(const std::vector<SessionGameView>& SessionGames, int GameIdentifier) const;
	int ResolveInputTargetGameIdentifier() const;

	int SessionIdentifier;
	GameConfigurator SessionConfigurator;
	std::wstring SessionName;
	std::vector<std::unique_ptr<PlayerObject>> Players;
	int ActivePlayerIdentifier;
};
