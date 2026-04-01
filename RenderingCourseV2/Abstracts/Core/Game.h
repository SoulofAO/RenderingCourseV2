#pragma once

#include "Abstracts/Core/RenderTypes.h"
#include "Abstracts/Subsystems/Subsystem.h"
#include "Abstracts/Core/Actor.h"
#include <windows.h>
#include <directxmath.h>
#include <d3d11.h>
#include <vector>
#include <memory>
#include <chrono>
#include <functional>

class InputDevice;
class SceneViewportSubsystem;
class ResourceManager;
class GameInputHandler;
class CameraComponent;
class LightComponent;
class GameInstance;

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
	void StartEmbeddedPlay();
	void TickFrame(float DeltaTime);
	void RenderFrame(const RenderFrameContext& RenderFrameContextValue,
		const GameRenderTargetOverride* OverrideRenderTarget = nullptr,
		const D3D11_VIEWPORT* OverrideViewport = nullptr);
	void SetCreateDefaultSceneViewportSubsystem(bool NewCreateDefaultSceneViewportSubsystem);
	void SetExternalMessageHandler(std::function<LRESULT(HWND, UINT, WPARAM, LPARAM)> NewExternalMessageHandler);
	void SetOwningGameInstance(GameInstance* NewOwningGameInstance);
	GameInstance* GetOwningGameInstance() const;

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
	void SetWorldBoundarySphereEnabled(bool NewIsEnabled);
	bool GetWorldBoundarySphereEnabled() const;
	void SetWorldBoundarySphereCenter(const DirectX::XMFLOAT3& NewWorldBoundarySphereCenter);
	const DirectX::XMFLOAT3& GetWorldBoundarySphereCenter() const;
	void SetWorldBoundarySphereRadius(float NewWorldBoundarySphereRadius);
	float GetWorldBoundarySphereRadius() const;
	void SetWorldBoundarySphereSettings(
		bool NewIsEnabled,
		const DirectX::XMFLOAT3& NewWorldBoundarySphereCenter,
		float NewWorldBoundarySphereRadius);

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

	template<typename ActorType>
	std::vector<ActorType*> GetAllActorsByClass() const
	{
		std::vector<ActorType*> FoundActors;

		for (const std::unique_ptr<Actor>& ExistingActor : Actors)
		{
			if (ExistingActor == nullptr)
			{
				continue;
			}

			ActorType* FoundActor = dynamic_cast<ActorType*>(ExistingActor.get());
			if (FoundActor != nullptr)
			{
				FoundActors.push_back(FoundActor);
			}
		}

		return FoundActors;
	}

	template<typename ComponentType>
	ComponentType* GetFirstComponentByClass(bool RequireActiveComponent = false) const
	{
		for (const std::unique_ptr<Actor>& ExistingActor : Actors)
		{
			if (ExistingActor == nullptr)
			{
				continue;
			}

			ComponentType* FoundComponent = ExistingActor->GetFirstComponentByClass<ComponentType>(RequireActiveComponent);
			if (FoundComponent != nullptr)
			{
				return FoundComponent;
			}
		}

		return nullptr;
	}

	template<typename ComponentType>
	std::vector<Actor*> GetAllActorsWithComponentByClass(bool RequireActiveComponent = false) const
	{
		std::vector<Actor*> FoundActors;

		for (const std::unique_ptr<Actor>& ExistingActor : Actors)
		{
			if (ExistingActor == nullptr)
			{
				continue;
			}

			ComponentType* FoundComponent = ExistingActor->GetFirstComponentByClass<ComponentType>(RequireActiveComponent);
			if (FoundComponent != nullptr)
			{
				FoundActors.push_back(ExistingActor.get());
			}
		}

		return FoundActors;
	}

	virtual LRESULT MessageHandler(HWND WindowHandle, UINT Message, WPARAM WParam, LPARAM LParam);

protected:
	virtual void BeginPlay();
	virtual void Update(float DeltaTime);
	virtual void Draw();
	void ApplyMouseInputMode();
	void UpdateMouseInputModeState();
	void UpdateInputHandlerActivationState();
	void DrawCameraPossessionUserInterface();
	void ToggleCameraPossessionFromUserInterface();
	void UpdateSelectedActorFromMouseClick();
	Actor* FindActorUnderMouseCursor() const;
	void DrawActorTranslationGizmo();
	bool ForceRebuildInputResourcesAndReinitializeScene();
	LightComponent* FindFirstDirectionalLightComponent() const;
	void ApplyWorldBoundarySphereSettings();

	LPCWSTR Name;
	int ScreenWidth;
	int ScreenHeight;

	std::unique_ptr<InputDevice> Input;
	std::unique_ptr<ResourceManager> Resources;
	std::vector<std::unique_ptr<Subsystem>> Subsystems;
	std::vector<std::unique_ptr<Actor>> Actors;
	std::vector<std::unique_ptr<GameInputHandler>> InputHandlers;
	Actor* FallbackCameraActor;
	Actor* SelectedActorForGizmo;
	CameraComponent* FallbackCameraComponentInstance;

	std::chrono::time_point<std::chrono::steady_clock> StartTime;
	std::chrono::time_point<std::chrono::steady_clock> PreviousTime;
	float TotalRunTimeSeconds;
	unsigned int FrameCount;
	bool IsExitRequested;
	MouseInputMode CurrentMouseInputMode;
	bool DefaultCameraSettingsWindowVisible;
	bool HasInputResourcesRebuildResult;
	bool LastInputResourcesRebuildSucceeded;
	bool IsWorldBoundarySphereEnabled;
	DirectX::XMFLOAT3 WorldBoundarySphereCenter;
	float WorldBoundarySphereRadius;
	bool IsEmbeddedPlayStarted;
	bool CreateDefaultSceneViewportSubsystem;
	std::function<LRESULT(HWND, UINT, WPARAM, LPARAM)> ExternalMessageHandler;
	GameInstance* OwningGameInstance;
};

extern Game* GlobalGame;
