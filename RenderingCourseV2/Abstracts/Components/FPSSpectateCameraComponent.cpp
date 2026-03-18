#include "Abstracts/Components/FPSSpectateCameraComponent.h"
#include "Abstracts/Components/DefaultCameraSettingsUIRenderingComponent.h"
#include "Abstracts/Core/Actor.h"
#include "Abstracts/Core/Game.h"
#include "Abstracts/Input/EngineHotkeyInputHandler.h"
#include <memory>

FPSSpectateCameraComponent::FPSSpectateCameraComponent()
	: CameraComponent()
	, EngineHotkeyInputHandlerInstance(nullptr)
	, DefaultCameraSettingsUIRenderingComponentInstance(nullptr)
	, IsPossessed(false)
{
}

FPSSpectateCameraComponent::~FPSSpectateCameraComponent() = default;

void FPSSpectateCameraComponent::Initialize()
{
	CameraComponent::Initialize();
}

void FPSSpectateCameraComponent::Shutdown()
{
	if (IsPossessed)
	{
		Unposses();
	}

	CameraComponent::Shutdown();
}

void FPSSpectateCameraComponent::Posses()
{
	if (IsPossessed)
	{
		return;
	}

	Game* OwningGame = GetOwningGame();
	if (OwningGame == nullptr)
	{
		return;
	}

	if (EngineHotkeyInputHandlerInstance == nullptr)
	{
		std::unique_ptr<EngineHotkeyInputHandler> NewEngineHotkeyInputHandler = std::make_unique<EngineHotkeyInputHandler>();
		EngineHotkeyInputHandlerInstance = NewEngineHotkeyInputHandler.get();
		OwningGame->RegisterInputHandler(std::move(NewEngineHotkeyInputHandler));
	}

	if (DefaultCameraSettingsUIRenderingComponentInstance == nullptr)
	{
		std::unique_ptr<Actor> DefaultCameraUIActor = std::make_unique<Actor>();
		std::unique_ptr<DefaultCameraSettingsUIRenderingComponent> NewDefaultCameraSettingsUIRenderingComponent = std::make_unique<DefaultCameraSettingsUIRenderingComponent>();
		DefaultCameraSettingsUIRenderingComponentInstance = NewDefaultCameraSettingsUIRenderingComponent.get();
		DefaultCameraUIActor->AddComponent(std::move(NewDefaultCameraSettingsUIRenderingComponent));
		OwningGame->AddActor(std::move(DefaultCameraUIActor));
	}

	IsPossessed = true;
}

void FPSSpectateCameraComponent::Unposses()
{
	if (IsPossessed == false)
	{
		return;
	}

	Game* OwningGame = GetOwningGame();
	if (OwningGame != nullptr)
	{
		if (EngineHotkeyInputHandlerInstance != nullptr)
		{
			OwningGame->UnregisterInputHandler(EngineHotkeyInputHandlerInstance);
			EngineHotkeyInputHandlerInstance = nullptr;
		}

		OwningGame->SetDefaultCameraSettingsWindowVisible(false);
	}

	IsPossessed = false;
}
