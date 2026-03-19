#include "KatamaryTask/KatamaryOrbitCameraInputHandler.h"
#include "KatamaryTask/KatamaryOrbitCameraComponent.h"
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

	ActiveOrbitCameraComponent->HandleOrbitInput(Input, DeltaTime);
}
