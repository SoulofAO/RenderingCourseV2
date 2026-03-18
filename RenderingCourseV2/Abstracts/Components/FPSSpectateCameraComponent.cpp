#include "Abstracts/Components/FPSSpectateCameraComponent.h"
#include "Abstracts/Components/FPSSpectateCameraComponentSettingsUI.h"
#include "Abstracts/Core/Actor.h"
#include "Abstracts/Core/Game.h"
#include <algorithm>
#include <memory>

FPSSpectateCameraComponent::FPSSpectateCameraComponent()
	: CameraComponent()
	, FPSSpectateCameraComponentSettingsUIInstance(nullptr)
	, MovementSpeedScale(1.0f)
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

	if (FPSSpectateCameraComponentSettingsUIInstance == nullptr)
	{
		std::unique_ptr<Actor> DefaultCameraUIActor = std::make_unique<Actor>();
		std::unique_ptr<FPSSpectateCameraComponentSettingsUI> NewFPSSpectateCameraComponentSettingsUI = std::make_unique<FPSSpectateCameraComponentSettingsUI>();
		NewFPSSpectateCameraComponentSettingsUI->SetTargetFPSSpectateCameraComponent(this);
		FPSSpectateCameraComponentSettingsUIInstance = NewFPSSpectateCameraComponentSettingsUI.get();
		DefaultCameraUIActor->AddComponent(std::move(NewFPSSpectateCameraComponentSettingsUI));
		OwningGame->AddActor(std::move(DefaultCameraUIActor));
	}
	else
	{
		FPSSpectateCameraComponentSettingsUIInstance->SetTargetFPSSpectateCameraComponent(this);
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
		if (FPSSpectateCameraComponentSettingsUIInstance != nullptr)
		{
			FPSSpectateCameraComponentSettingsUIInstance->SetTargetFPSSpectateCameraComponent(nullptr);
		}
	}

	IsPossessed = false;
}

void FPSSpectateCameraComponent::SetMovementSpeedScale(float NewMovementSpeedScale)
{
	MovementSpeedScale = (std::clamp)(NewMovementSpeedScale, 0.05f, 5.0f);
}

float FPSSpectateCameraComponent::GetMovementSpeedScale() const
{
	return MovementSpeedScale;
}

bool FPSSpectateCameraComponent::GetIsPossessed() const
{
	return IsPossessed;
}
