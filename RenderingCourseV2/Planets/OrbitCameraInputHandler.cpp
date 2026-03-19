#include "Planets/OrbitCameraInputHandler.h"
#include "Planets/OrbitCameraComponent.h"
#include "Abstracts/Components/CameraComponent.h"
#include "Abstracts/Core/Game.h"
#include "Abstracts/Subsystems/CameraSubsystem.h"
#include "Abstracts/Subsystems/InputDevice.h"

void OrbitCameraInputHandler::HandleInput(Game* OwningGame, InputDevice* Input, float DeltaTime)
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
	OrbitCameraComponent* ActiveOrbitCameraComponent = dynamic_cast<OrbitCameraComponent*>(ActiveCamera);
	if (ActiveOrbitCameraComponent == nullptr)
	{
		return;
	}

	const float MouseDeltaX = static_cast<float>(Input->GetMouseDeltaX());
	const float MouseDeltaY = static_cast<float>(Input->GetMouseDeltaY());
	const int MouseWheelDelta = Input->GetMouseWheelDelta();
	ActiveOrbitCameraComponent->ApplyOrbitInput(MouseDeltaX, MouseDeltaY, MouseWheelDelta, DeltaTime);
}
