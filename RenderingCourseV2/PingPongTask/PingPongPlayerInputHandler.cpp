#include "PingPongPlayerInputHandler.h"
#include "PingPongGame.h"
#include "Abstracts/Core/Game.h"
#include "Abstracts/Subsystems/InputDevice.h"
#include <windows.h>

void PingPongPlayerInputHandler::HandleInput(Game* OwningGame, InputDevice* Input, float DeltaTime)
{
	if (OwningGame == nullptr || Input == nullptr || DeltaTime <= 0.0f)
	{
		return;
	}

	PingPongGame* PingPongGameInstance = dynamic_cast<PingPongGame*>(OwningGame);
	if (PingPongGameInstance == nullptr)
	{
		return;
	}

	float MovementDirection = 0.0f;
	if (Input->IsKeyDown('W') || Input->IsKeyDown(VK_UP))
	{
		MovementDirection += 1.0f;
	}

	if (Input->IsKeyDown('S') || Input->IsKeyDown(VK_DOWN))
	{
		MovementDirection -= 1.0f;
	}

	PingPongGameInstance->ApplyPlayerMovementInput(DeltaTime, MovementDirection);
}
