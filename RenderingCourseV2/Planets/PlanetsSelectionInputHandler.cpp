#include "Planets/PlanetsSelectionInputHandler.h"
#include "Planets/PlanetsGame.h"
#include "Planets/OrbitCameraComponent.h"
#include "Abstracts/Components/CameraComponent.h"
#include "Abstracts/Core/Game.h"
#include "Abstracts/Subsystems/CameraSubsystem.h"
#include "Abstracts/Subsystems/InputDevice.h"
#include <windows.h>

void PlanetsSelectionInputHandler::HandleInput(Game* OwningGame, InputDevice* Input, float DeltaTime)
{
	if (OwningGame == nullptr || Input == nullptr || DeltaTime < 0.0f)
	{
		return;
	}

	if (Input->WasKeyPressedThisFrame(VK_LBUTTON) == false)
	{
		return;
	}

	CameraSubsystem* CameraSystem = OwningGame->GetSubsystem<CameraSubsystem>();
	if (CameraSystem == nullptr)
	{
		return;
	}

	CameraComponent* ActiveCamera = CameraSystem->GetActiveCamera();
	OrbitCameraComponent* ActiveOrbitCamera = dynamic_cast<OrbitCameraComponent*>(ActiveCamera);
	if (ActiveOrbitCamera == nullptr)
	{
		return;
	}

	PlanetsGame* PlanetsGameInstance = dynamic_cast<PlanetsGame*>(OwningGame);
	if (PlanetsGameInstance == nullptr)
	{
		return;
	}

	const int MousePositionX = Input->GetMousePositionX();
	const int MousePositionY = Input->GetMousePositionY();
	PlanetsGameInstance->HandleCelestialBodySelectionFromInputDevice(MousePositionX, MousePositionY);
}
