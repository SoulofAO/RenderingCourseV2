#include "Abstracts/Core/MultiGameSession.h"
#include "Abstracts/Core/Game.h"
#include "Abstracts/Subsystems/CameraSubsystem.h"
#include <algorithm>
#include <set>

MultiGameSession::MultiGameSession(int NewSessionIdentifier, const GameConfigurator& NewGameConfigurator)
	: SessionIdentifier(NewSessionIdentifier)
	, SessionConfigurator(NewGameConfigurator)
	, SessionName(NewGameConfigurator.SessionName)
	, ActivePlayerIdentifier(-1)
{
}

MultiGameSession::~MultiGameSession()
{
	Shutdown();
}

int MultiGameSession::GetSessionIdentifier() const
{
	return SessionIdentifier;
}

const std::wstring& MultiGameSession::GetSessionName() const
{
	return SessionName;
}

bool MultiGameSession::GetReceiveInputWhenInactive() const
{
	return SessionConfigurator.ReceiveInputWhenSessionIsInactive;
}

void MultiGameSession::Initialize()
{
	Players.clear();
	ActivePlayerIdentifier = -1;
	if (SessionConfigurator.Players.empty())
	{
		return;
	}

	for (const PlayerBinding& ExistingPlayerBinding : SessionConfigurator.Players)
	{
		bool IsReferencedGameValid = false;
		for (const GameDefinition& ExistingGameDefinition : SessionConfigurator.Games)
		{
			if (ExistingGameDefinition.GameIdentifier == ExistingPlayerBinding.ActiveGameIdentifier)
			{
				IsReferencedGameValid = true;
				break;
			}
		}

		if (!IsReferencedGameValid)
		{
			continue;
		}

		std::unique_ptr<PlayerObject> NewPlayer = std::make_unique<PlayerObject>();
		NewPlayer->SetPlayerIdentifier(ExistingPlayerBinding.PlayerIdentifier);
		NewPlayer->SetInputSourceIdentifier(ExistingPlayerBinding.InputSourceIdentifier);
		NewPlayer->SetPreferredCameraIndex(ExistingPlayerBinding.PreferredCameraIndex);
		NewPlayer->SetViewportRectangle(ExistingPlayerBinding.ViewportRectangle);
		NewPlayer->SetActiveGameIdentifier(ExistingPlayerBinding.ActiveGameIdentifier);
		Players.push_back(std::move(NewPlayer));
		if (ActivePlayerIdentifier < 0)
		{
			ActivePlayerIdentifier = ExistingPlayerBinding.PlayerIdentifier;
		}
	}
}

void MultiGameSession::TickFrame(float DeltaTime, bool IsSessionActive, const std::vector<SessionGameView>& SessionGames)
{
	if (!IsSessionActive && !GetReceiveInputWhenInactive())
	{
		return;
	}

	for (const SessionGameView& ExistingSessionGame : SessionGames)
	{
		if (ExistingSessionGame.GameInstance != nullptr)
		{
			ExistingSessionGame.GameInstance->TickFrame(DeltaTime);
		}
	}
}

void MultiGameSession::RenderFrame(
	SceneViewportSubsystem* SceneViewportSubsystemInstance,
	PlayerRenderTargetService* PlayerRenderTargetServiceInstance,
	int ScreenWidth,
	int ScreenHeight,
	const std::vector<SessionGameView>& SessionGames,
	std::vector<PlayerRenderTargetCompositeCommand>& OutCompositeCommands)
{
	if (SceneViewportSubsystemInstance == nullptr || PlayerRenderTargetServiceInstance == nullptr || ScreenWidth <= 0 || ScreenHeight <= 0)
	{
		return;
	}

	for (const std::unique_ptr<PlayerObject>& ExistingPlayer : Players)
	{
		if (ExistingPlayer == nullptr)
		{
			continue;
		}

		Game* AssignedGame = FindGameByIdentifier(SessionGames, ExistingPlayer->GetActiveGameIdentifier());
		if (AssignedGame == nullptr)
		{
			continue;
		}

		const ViewportRectangleNormalized& PlayerViewportRectangle = ExistingPlayer->GetViewportRectangle();
		const int TargetWidth = (std::max)(1, static_cast<int>(static_cast<float>(ScreenWidth) * PlayerViewportRectangle.Width));
		const int TargetHeight = (std::max)(1, static_cast<int>(static_cast<float>(ScreenHeight) * PlayerViewportRectangle.Height));
		PlayerRenderTargetIdentifier Identifier = {};
		Identifier.SessionIdentifier = SessionIdentifier;
		Identifier.PlayerIdentifier = ExistingPlayer->GetPlayerIdentifier();
		if (!PlayerRenderTargetServiceInstance->EnsurePlayerRenderTarget(SceneViewportSubsystemInstance->GetDevice(), Identifier, TargetWidth, TargetHeight))
		{
			continue;
		}

		GameRenderTargetOverride RenderTargetOverride = {};
		if (!PlayerRenderTargetServiceInstance->GetPlayerRenderTargetOverride(Identifier, RenderTargetOverride))
		{
			continue;
		}

		CameraSubsystem* CameraSubsystemInstance = AssignedGame->GetSubsystem<CameraSubsystem>();
		if (CameraSubsystemInstance != nullptr && ExistingPlayer->GetPreferredCameraIndex() >= 0)
		{
			CameraSubsystemInstance->SetActiveCameraIndex(ExistingPlayer->GetPreferredCameraIndex());
		}

		D3D11_VIEWPORT PlayerViewport = BuildPlayerViewport(TargetWidth, TargetHeight, ViewportRectangleNormalized{ 0.0f, 0.0f, 1.0f, 1.0f });
		AssignedGame->RenderFrame(SceneViewportSubsystemInstance, &RenderTargetOverride, &PlayerViewport, false);

		PlayerRenderTargetCompositeCommand CompositeCommand = {};
		CompositeCommand.Identifier = Identifier;
		CompositeCommand.DestinationPixelX = (std::max)(0, static_cast<int>(PlayerViewportRectangle.Left * static_cast<float>(ScreenWidth)));
		CompositeCommand.DestinationPixelY = (std::max)(0, static_cast<int>(PlayerViewportRectangle.Top * static_cast<float>(ScreenHeight)));
		CompositeCommand.DestinationWidth = TargetWidth;
		CompositeCommand.DestinationHeight = TargetHeight;
		OutCompositeCommands.push_back(CompositeCommand);
	}
}

bool MultiGameSession::HandleMessage(HWND WindowHandle, UINT Message, WPARAM WParam, LPARAM LParam, bool IsSessionActive, const std::vector<SessionGameView>& SessionGames)
{
	if (!IsSessionActive && !GetReceiveInputWhenInactive())
	{
		return false;
	}

	const int InputTargetGameIdentifier = ResolveInputTargetGameIdentifier();
	Game* InputTargetGame = FindGameByIdentifier(SessionGames, InputTargetGameIdentifier);
	if (InputTargetGame == nullptr)
	{
		return false;
	}

	return InputTargetGame->MessageHandler(WindowHandle, Message, WParam, LParam) != 0;
}

bool MultiGameSession::BuildGameDefinitions(std::vector<GameDefinition>& OutGameDefinitions) const
{
	OutGameDefinitions.clear();
	if (SessionConfigurator.Games.empty())
	{
		return true;
	}

	std::set<int> UniqueGameIdentifiers;
	for (const GameDefinition& ExistingGameDefinition : SessionConfigurator.Games)
	{
		if (!ExistingGameDefinition.GameFactory)
		{
			return false;
		}
		if (UniqueGameIdentifiers.find(ExistingGameDefinition.GameIdentifier) != UniqueGameIdentifiers.end())
		{
			return false;
		}
		UniqueGameIdentifiers.insert(ExistingGameDefinition.GameIdentifier);
		OutGameDefinitions.push_back(ExistingGameDefinition);
	}

	return true;
}

void MultiGameSession::SetPlayerActiveGameIdentifier(int PlayerIdentifier, int NewActiveGameIdentifier)
{
	bool IsDefinedGameIdentifier = false;
	for (const GameDefinition& ExistingGameDefinition : SessionConfigurator.Games)
	{
		if (ExistingGameDefinition.GameIdentifier == NewActiveGameIdentifier)
		{
			IsDefinedGameIdentifier = true;
			break;
		}
	}
	if (!IsDefinedGameIdentifier)
	{
		return;
	}

	for (std::unique_ptr<PlayerObject>& ExistingPlayer : Players)
	{
		if (ExistingPlayer == nullptr || ExistingPlayer->GetPlayerIdentifier() != PlayerIdentifier)
		{
			continue;
		}

		ExistingPlayer->SetActiveGameIdentifier(NewActiveGameIdentifier);
		return;
	}
}

void MultiGameSession::SetActivePlayerIdentifier(int NewActivePlayerIdentifier)
{
	for (const std::unique_ptr<PlayerObject>& ExistingPlayer : Players)
	{
		if (ExistingPlayer != nullptr && ExistingPlayer->GetPlayerIdentifier() == NewActivePlayerIdentifier)
		{
			ActivePlayerIdentifier = NewActivePlayerIdentifier;
			return;
		}
	}
}

int MultiGameSession::GetActivePlayerIdentifier() const
{
	return ActivePlayerIdentifier;
}

void MultiGameSession::Shutdown()
{
	Players.clear();
	ActivePlayerIdentifier = -1;
}

D3D11_VIEWPORT MultiGameSession::BuildPlayerViewport(int ScreenWidth, int ScreenHeight, const ViewportRectangleNormalized& ViewportRectangle) const
{
	D3D11_VIEWPORT PlayerViewport = {};
	PlayerViewport.TopLeftX = ViewportRectangle.Left * static_cast<float>(ScreenWidth);
	PlayerViewport.TopLeftY = ViewportRectangle.Top * static_cast<float>(ScreenHeight);
	PlayerViewport.Width = (std::max)(1.0f, ViewportRectangle.Width * static_cast<float>(ScreenWidth));
	PlayerViewport.Height = (std::max)(1.0f, ViewportRectangle.Height * static_cast<float>(ScreenHeight));
	PlayerViewport.MinDepth = 0.0f;
	PlayerViewport.MaxDepth = 1.0f;
	return PlayerViewport;
}

Game* MultiGameSession::FindGameByIdentifier(const std::vector<SessionGameView>& SessionGames, int GameIdentifier) const
{
	for (const SessionGameView& ExistingSessionGame : SessionGames)
	{
		if (ExistingSessionGame.GameIdentifier == GameIdentifier)
		{
			return ExistingSessionGame.GameInstance;
		}
	}

	return nullptr;
}

int MultiGameSession::ResolveInputTargetGameIdentifier() const
{
	for (const std::unique_ptr<PlayerObject>& ExistingPlayer : Players)
	{
		if (ExistingPlayer != nullptr && ExistingPlayer->GetPlayerIdentifier() == ActivePlayerIdentifier)
		{
			return ExistingPlayer->GetActiveGameIdentifier();
		}
	}

	if (!Players.empty() && Players.front() != nullptr)
	{
		return Players.front()->GetActiveGameIdentifier();
	}

	return -1;
}

