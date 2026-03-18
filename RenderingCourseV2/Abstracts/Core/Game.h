#pragma once

#include "Abstracts/Subsystems/Subsystem.h"
#include <windows.h>
#include <vector>
#include <memory>
#include <chrono>

class InputDevice;
class Actor;
class SceneViewportSubsystem;
class ResourceManager;
class GameInputHandler;
class CameraComponent;

enum class MouseInputMode
{
	GameOnly,
	GameAndUI,
	UIOnly
};

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
	ResourceManager* GetResourceManager() const;

	void AddSubsystem(std::unique_ptr<Subsystem> NewSubsystem);
	void AddActor(std::unique_ptr<Actor> NewActor);
	void RegisterInputHandler(std::unique_ptr<GameInputHandler> NewInputHandler);
	void UnregisterInputHandler(GameInputHandler* ExistingInputHandler);
	void SetMouseInputMode(MouseInputMode NewMouseInputMode);
	void ToggleMouseInputMode();
	MouseInputMode GetMouseInputMode() const;
	void SetDefaultCameraSettingsWindowVisible(bool NewDefaultCameraSettingsWindowVisible);
	void ToggleDefaultCameraSettingsWindowVisible();
	bool GetDefaultCameraSettingsWindowVisible() const;
	bool GetIsFallbackCameraPossessed() const;

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
	void ApplyMouseInputMode();
	void UpdateMouseInputModeState();
	void UpdateInputHandlerActivationState();

	LPCWSTR Name;
	int ScreenWidth;
	int ScreenHeight;

	std::unique_ptr<InputDevice> Input;
	std::unique_ptr<ResourceManager> Resources;
	std::vector<std::unique_ptr<Subsystem>> Subsystems;
	std::vector<std::unique_ptr<Actor>> Actors;
	std::vector<std::unique_ptr<GameInputHandler>> InputHandlers;
	Actor* FallbackCameraActor;
	CameraComponent* FallbackCameraComponentInstance;

	std::chrono::time_point<std::chrono::steady_clock> StartTime;
	std::chrono::time_point<std::chrono::steady_clock> PreviousTime;
	float TotalRunTimeSeconds;
	unsigned int FrameCount;
	bool IsExitRequested;
	MouseInputMode CurrentMouseInputMode;
	bool DefaultCameraSettingsWindowVisible;
};
