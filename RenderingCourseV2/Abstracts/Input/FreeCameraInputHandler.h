#pragma once

#include "Abstracts/Input/GameInputHandler.h"

class FreeCameraInputHandler : public GameInputHandler
{
public:
	FreeCameraInputHandler();

	void SetMovementSpeed(float NewMovementSpeed);
	void SetLookSensitivity(float NewLookSensitivity);

	void HandleInput(Game* OwningGame, InputDevice* Input, float DeltaTime) override;

private:
	float MovementSpeed;
	float LookSensitivity;
};
