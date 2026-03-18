#include "Abstracts/Input/EngineHotkeyInputHandler.h"
#include "Abstracts/Core/Game.h"
#include "Abstracts/Subsystems/CameraSubsystem.h"
#include "Abstracts/Subsystems/InputDevice.h"
#include "Abstracts/Subsystems/SceneViewportSubsystem.h"

void EngineHotkeyInputHandler::HandleInput(Game* OwningGame, InputDevice* Input, float DeltaTime)
{
	if (OwningGame == nullptr || Input == nullptr)
	{
		return;
	}

	SceneViewportSubsystem* SceneViewport = OwningGame->GetSubsystem<SceneViewportSubsystem>();
	if (SceneViewport != nullptr && Input->WasKeyPressedThisFrame('P'))
	{
		if (SceneViewport->GetRenderPipelineType() == RenderPipelineType::Forward)
		{
			SceneViewport->SetRenderPipelineType(RenderPipelineType::Deferred);
		}
		else
		{
			SceneViewport->SetRenderPipelineType(RenderPipelineType::Forward);
		}
	}

	CameraSubsystem* CameraSystem = OwningGame->GetSubsystem<CameraSubsystem>();
	if (CameraSystem != nullptr && Input->WasKeyPressedThisFrame('C'))
	{
		CameraSystem->CycleActiveCamera();
	}

	if (Input->WasKeyPressedThisFrame('M'))
	{
		OwningGame->ToggleMouseInputMode();
	}

	if (Input->WasKeyPressedThisFrame('K'))
	{
		OwningGame->ToggleDefaultCameraSettingsWindowVisible();
	}
}
