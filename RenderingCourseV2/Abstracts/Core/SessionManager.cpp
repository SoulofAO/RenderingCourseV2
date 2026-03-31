#include "Abstracts/Core/SessionManager.h"
#include "Abstracts/Core/MultiGameSession.h"
#include "Abstracts/Core/Game.h"
#include "Abstracts/Subsystems/SceneViewportSubsystem.h"
#include <algorithm>

SessionManager::SessionManager()
	: NextSessionIdentifier(1)
	, ActiveSessionIdentifier(-1)
{
}

SessionManager::~SessionManager() = default;

int SessionManager::OpenGame(const GameConfigurator& NewGameConfigurator)
{
	const int NewSessionIdentifier = NextSessionIdentifier;
	NextSessionIdentifier += 1;

	std::unique_ptr<MultiGameSession> NewSession = std::make_unique<MultiGameSession>(NewSessionIdentifier, NewGameConfigurator);
	NewSession->Initialize();
	std::vector<GameDefinition> GameDefinitions;
	if (!NewSession->BuildGameDefinitions(GameDefinitions))
	{
		return -1;
	}

	for (const GameDefinition& ExistingGameDefinition : GameDefinitions)
	{
		std::unique_ptr<Game> NewGame = ExistingGameDefinition.GameFactory();
		if (NewGame == nullptr)
		{
			continue;
		}

		NewGame->SetCreateDefaultSceneViewportSubsystem(false);
		NewGame->Initialize();
		NewGame->StartEmbeddedPlay();

		ManagedGameEntry ManagedGame = {};
		ManagedGame.SessionIdentifier = NewSessionIdentifier;
		ManagedGame.GameIdentifier = ExistingGameDefinition.GameIdentifier;
		ManagedGame.GameInstance = std::move(NewGame);
		ManagedGames.push_back(std::move(ManagedGame));
	}

	Sessions.push_back(std::move(NewSession));
	if (ActiveSessionIdentifier < 0)
	{
		ActiveSessionIdentifier = NewSessionIdentifier;
	}

	return NewSessionIdentifier;
}

void SessionManager::OpenMultipleGames(const std::vector<GameConfigurator>& NewGameConfigurators)
{
	for (const GameConfigurator& ExistingGameConfigurator : NewGameConfigurators)
	{
		OpenGame(ExistingGameConfigurator);
	}
}

void SessionManager::CloseGame(int SessionIdentifier)
{
	RemoveManagedGamesForSession(SessionIdentifier);

	for (size_t SessionIndex = 0; SessionIndex < Sessions.size(); ++SessionIndex)
	{
		if (Sessions[SessionIndex] == nullptr || Sessions[SessionIndex]->GetSessionIdentifier() != SessionIdentifier)
		{
			continue;
		}

		Sessions.erase(Sessions.begin() + static_cast<long long>(SessionIndex));
		break;
	}

	if (ActiveSessionIdentifier == SessionIdentifier)
	{
		if (Sessions.empty())
		{
			ActiveSessionIdentifier = -1;
		}
		else
		{
			ActiveSessionIdentifier = Sessions.front()->GetSessionIdentifier();
		}
	}
}

void SessionManager::SetActiveSessionIdentifier(int NewActiveSessionIdentifier)
{
	ActiveSessionIdentifier = NewActiveSessionIdentifier;
}

int SessionManager::GetActiveSessionIdentifier() const
{
	return ActiveSessionIdentifier;
}

void SessionManager::TickFrame(float DeltaTime)
{
	for (const std::unique_ptr<MultiGameSession>& ExistingSession : Sessions)
	{
		if (ExistingSession == nullptr)
		{
			continue;
		}

		const bool IsSessionActive = ExistingSession->GetSessionIdentifier() == ActiveSessionIdentifier;
		const std::vector<SessionGameView> SessionGameViews = BuildSessionGameViews(ExistingSession->GetSessionIdentifier());
		ExistingSession->TickFrame(DeltaTime, IsSessionActive, SessionGameViews);
	}
}

void SessionManager::RenderFrame(
	SceneViewportSubsystem* SceneViewportSubsystemInstance,
	PlayerRenderTargetService* PlayerRenderTargetServiceInstance,
	int ScreenWidth,
	int ScreenHeight)
{
	if (SceneViewportSubsystemInstance == nullptr || PlayerRenderTargetServiceInstance == nullptr)
	{
		return;
	}

	std::vector<PlayerRenderTargetCompositeCommand> CompositeCommands;
	for (const std::unique_ptr<MultiGameSession>& ExistingSession : Sessions)
	{
		if (ExistingSession == nullptr)
		{
			continue;
		}

		const std::vector<SessionGameView> SessionGameViews = BuildSessionGameViews(ExistingSession->GetSessionIdentifier());
		ExistingSession->RenderFrame(
			SceneViewportSubsystemInstance,
			PlayerRenderTargetServiceInstance,
			ScreenWidth,
			ScreenHeight,
			SessionGameViews,
			CompositeCommands);
	}

	PlayerRenderTargetServiceInstance->CompositeToBackBuffer(
		SceneViewportSubsystemInstance->GetDeviceContext(),
		SceneViewportSubsystemInstance->GetBackBufferTexture(),
		CompositeCommands);
}

bool SessionManager::HandleMessage(HWND WindowHandle, UINT Message, WPARAM WParam, LPARAM LParam)
{
	for (const std::unique_ptr<MultiGameSession>& ExistingSession : Sessions)
	{
		if (ExistingSession == nullptr)
		{
			continue;
		}

		const bool IsSessionActive = ExistingSession->GetSessionIdentifier() == ActiveSessionIdentifier;
		const std::vector<SessionGameView> SessionGameViews = BuildSessionGameViews(ExistingSession->GetSessionIdentifier());
		if (ExistingSession->HandleMessage(WindowHandle, Message, WParam, LParam, IsSessionActive, SessionGameViews))
		{
			return true;
		}
	}

	return false;
}

void SessionManager::SetSessionActivePlayerIdentifier(int SessionIdentifier, int PlayerIdentifier)
{
	for (const std::unique_ptr<MultiGameSession>& ExistingSession : Sessions)
	{
		if (ExistingSession == nullptr || ExistingSession->GetSessionIdentifier() != SessionIdentifier)
		{
			continue;
		}

		ExistingSession->SetActivePlayerIdentifier(PlayerIdentifier);
		return;
	}
}

void SessionManager::SetPlayerActiveGameIdentifier(int SessionIdentifier, int PlayerIdentifier, int GameIdentifier)
{
	for (const std::unique_ptr<MultiGameSession>& ExistingSession : Sessions)
	{
		if (ExistingSession == nullptr || ExistingSession->GetSessionIdentifier() != SessionIdentifier)
		{
			continue;
		}

		ExistingSession->SetPlayerActiveGameIdentifier(PlayerIdentifier, GameIdentifier);
		return;
	}
}

std::vector<SessionGameView> SessionManager::BuildSessionGameViews(int SessionIdentifier)
{
	std::vector<SessionGameView> SessionGames;
	for (ManagedGameEntry& ExistingManagedGame : ManagedGames)
	{
		if (ExistingManagedGame.SessionIdentifier != SessionIdentifier)
		{
			continue;
		}

		SessionGameView SessionGame = {};
		SessionGame.GameIdentifier = ExistingManagedGame.GameIdentifier;
		SessionGame.GameInstance = ExistingManagedGame.GameInstance.get();
		SessionGames.push_back(SessionGame);
	}

	return SessionGames;
}

void SessionManager::RemoveManagedGamesForSession(int SessionIdentifier)
{
	for (size_t ManagedGameIndex = 0; ManagedGameIndex < ManagedGames.size();)
	{
		if (ManagedGames[ManagedGameIndex].SessionIdentifier == SessionIdentifier)
		{
			ManagedGames.erase(ManagedGames.begin() + static_cast<long long>(ManagedGameIndex));
			continue;
		}

		ManagedGameIndex += 1;
	}
}
