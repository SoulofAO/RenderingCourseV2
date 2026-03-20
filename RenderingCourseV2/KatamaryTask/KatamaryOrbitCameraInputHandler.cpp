#include "KatamaryTask/KatamaryOrbitCameraInputHandler.h"
#include "KatamaryTask/KatamaryOrbitCameraComponent.h"
#include "KatamaryTask/KatamaryGame.h"
#include "Abstracts/Components/CameraComponent.h"
#include "Abstracts/Core/Game.h"
#include "Abstracts/Subsystems/CameraSubsystem.h"
#include "Abstracts/Subsystems/InputDevice.h"

void KatamaryOrbitCameraInputHandler::HandleInput(Game* OwningGame, InputDevice* Input, float DeltaTime)
{
	if (OwningGame == nullptr || Input == nullptr || DeltaTime <= 0.0f)
	{
		return;
	}

	CameraSubsystem* CameraSystem = OwningGame->GetSubsystem<CameraSubsystem>();
	if (CameraSystem == nullptr)
	{
		return;
	}

	CameraComponent* ActiveCamera = CameraSystem->GetActiveCamera();
	KatamaryOrbitCameraComponent* ActiveOrbitCameraComponent = dynamic_cast<KatamaryOrbitCameraComponent*>(ActiveCamera);
	if (ActiveOrbitCameraComponent == nullptr)
	{
		return;
	}

	const float MouseDeltaX = static_cast<float>(Input->GetMouseDeltaX());
	const float MouseDeltaY = static_cast<float>(Input->GetMouseDeltaY());
	const int MouseWheelDelta = Input->GetMouseWheelDelta();
	ActiveOrbitCameraComponent->ApplyOrbitInput(MouseDeltaX, MouseDeltaY, MouseWheelDelta, DeltaTime);

	float MovementInputForward = 0.0f;
	float MovementInputRight = 0.0f;
	if (Input->IsKeyDown('W'))
	{
		MovementInputForward += 1.0f;
	}
	if (Input->IsKeyDown('S'))
	{
		MovementInputForward -= 1.0f;
	}
	if (Input->IsKeyDown('D'))
	{
		MovementInputRight += 1.0f;
	}
	if (Input->IsKeyDown('A'))
	{
		MovementInputRight -= 1.0f;
	}

	KatamaryGame* KatamaryGameInstance = dynamic_cast<KatamaryGame*>(OwningGame);
	if (KatamaryGameInstance != nullptr)
	{
		KatamaryGameInstance->HandlePlayerMovementInput(DeltaTime, MovementInputForward, MovementInputRight);
		if (Input->WasKeyPressedThisFrame(VK_SPACE))
		{
			KatamaryGameInstance->HandlePlayerJumpInput();
		}
	}
}
