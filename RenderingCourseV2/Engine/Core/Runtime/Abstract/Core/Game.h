#pragma once

#include "Engine/Core/Runtime/Abstract/Subsystems/Subsystem.h"
#include <windows.h>
#include <vector>
#include <memory>
#include <chrono>

class InputDevice;
class Actor;
class SceneViewportSubsystem;

class Game
{
public:
	Game(LPCWSTR ApplicationName, int ScreenWidth, int ScreenHeight);
	virtual ~Game();

	Game(const Game&) = delete;
	Game& operator=(const Game&) = delete;

	void Initialize();
	void Run();

	LPCWSTR GetApplicationName() const;
	InputDevice* GetInputDevice() const;
	int GetScreenWidth() const;
	int GetScreenHeight() const;
	float GetTotalRunTimeSeconds() const;

	void AddSubsystem(std::unique_ptr<Subsystem> NewSubsystem);
	void AddActor(std::unique_ptr<Actor> NewActor);

	template<typename TSubsystem>
	TSubsystem* GetSubsystem() const
	{
		for (const std::unique_ptr<Subsystem>& ExistingSubsystem : Subsystems)
		{
			TSubsystem* TypedSubsystem = dynamic_cast<TSubsystem*>(ExistingSubsystem.get());
			if (TypedSubsystem != nullptr)
			{
				return TypedSubsystem;
			}
		}

		return nullptr;
	}

	virtual LRESULT MessageHandler(HWND WindowHandle, UINT Message, WPARAM WParam, LPARAM LParam);

protected:
	virtual void BeginPlay();
	virtual void Update(float DeltaTime);
	virtual void Draw();

	LPCWSTR Name;
	int ScreenWidth;
	int ScreenHeight;

	std::unique_ptr<InputDevice> Input;
	std::vector<std::unique_ptr<Subsystem>> Subsystems;
	std::vector<std::unique_ptr<Actor>> Actors;

	std::chrono::time_point<std::chrono::steady_clock> StartTime;
	std::chrono::time_point<std::chrono::steady_clock> PreviousTime;
	float TotalRunTimeSeconds;
	unsigned int FrameCount;
	bool IsExitRequested;
};

