#pragma once

#include "Abstracts/Core/SessionManager.h"
#include <memory>
#include <vector>
#include <chrono>

class GameInstanceSubsystem;

class GameInstance
{
public:
	GameInstance();
	~GameInstance();

	void Initialize(LPCWSTR ApplicationName, int ScreenWidth, int ScreenHeight);
	int OpenGame(const GameConfigurator& NewGameConfigurator);
	void OpenMultipleGames(const std::vector<GameConfigurator>& NewGameConfigurators);
	void CloseGame(int SessionIdentifier);
	void SetPlayerActiveGameIdentifier(int SessionIdentifier, int PlayerIdentifier, int GameIdentifier);
	void QueueSessionReplacement(int SessionIdentifier, const GameConfigurator& NewGameConfigurator);
	void Run();
	LRESULT MessageHandler(HWND WindowHandle, UINT Message, WPARAM WParam, LPARAM LParam);
	void AddSubsystem(std::unique_ptr<GameInstanceSubsystem> NewSubsystem);

	template<typename TSubsystem>
	TSubsystem* GetSubsystem() const
	{
		for (const std::unique_ptr<GameInstanceSubsystem>& ExistingSubsystem : Subsystems)
		{
			TSubsystem* TypedSubsystem = dynamic_cast<TSubsystem*>(ExistingSubsystem.get());
			if (TypedSubsystem != nullptr)
			{
				return TypedSubsystem;
			}
		}

		return nullptr;
	}

private:
	struct PendingSessionReplacement
	{
		int SessionIdentifier;
		GameConfigurator NewGameConfigurator;
	};

	LPCWSTR ApplicationName;
	int ScreenWidth;
	int ScreenHeight;
	SessionManager SessionManagerInstance;
	std::vector<std::unique_ptr<GameInstanceSubsystem>> Subsystems;
	std::vector<PendingSessionReplacement> PendingSessionReplacements;
	bool IsExitRequested;
	std::chrono::time_point<std::chrono::steady_clock> PreviousTime;
	float TotalRunTimeSeconds;
};

extern GameInstance* GlobalGameInstance;
