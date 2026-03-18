#include "Abstracts/Input/FreeCameraInputHandler.h"
#include "Abstracts/Components/CameraComponent.h"
#include "Abstracts/Components/FPSSpectateCameraComponent.h"
#include "Abstracts/Core/Actor.h"
#include "Abstracts/Core/Game.h"
#include "Abstracts/Subsystems/CameraSubsystem.h"
#include "Abstracts/Subsystems/InputDevice.h"
#include <algorithm>
#include <directxmath.h>
#include <windows.h>

FreeCameraInputHandler::FreeCameraInputHandler()
	: MovementSpeed(6.0f)
	, LookSensitivity(0.0025f)
{
}

void FreeCameraInputHandler::SetMovementSpeed(float NewMovementSpeed)
{
	MovementSpeed = (std::max)(0.0f, NewMovementSpeed);
}

void FreeCameraInputHandler::SetLookSensitivity(float NewLookSensitivity)
{
	LookSensitivity = (std::max)(0.00001f, NewLookSensitivity);
}

void FreeCameraInputHandler::HandleInput(Game* OwningGame, InputDevice* Input, float DeltaTime)
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
	if (ActiveCamera == nullptr)
	{
		return;
	}

	Actor* CameraActor = ActiveCamera->GetOwningActor();
	if (CameraActor == nullptr)
	{
		return;
	}

	Transform CameraTransform = CameraActor->GetLocalTransform();
	const float MouseDeltaX = static_cast<float>(Input->GetMouseDeltaX());
	const float MouseDeltaY = static_cast<float>(Input->GetMouseDeltaY());

	CameraTransform.RotationEuler.y += MouseDeltaX * LookSensitivity;
	CameraTransform.RotationEuler.x += MouseDeltaY * LookSensitivity;
	CameraTransform.RotationEuler.x = (std::clamp)(CameraTransform.RotationEuler.x, -1.55334f, 1.55334f);

	DirectX::XMMATRIX RotationMatrix = DirectX::XMMatrixRotationRollPitchYaw(
		CameraTransform.RotationEuler.x,
		CameraTransform.RotationEuler.y,
		CameraTransform.RotationEuler.z);

	DirectX::XMVECTOR ForwardDirection = DirectX::XMVector3TransformNormal(DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), RotationMatrix);
	DirectX::XMVECTOR RightDirection = DirectX::XMVector3TransformNormal(DirectX::XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f), RotationMatrix);
	DirectX::XMVECTOR UpDirection = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	DirectX::XMVECTOR DesiredMovementDirection = DirectX::XMVectorZero();

	if (Input->IsKeyDown('W'))
	{
		DesiredMovementDirection = DirectX::XMVectorAdd(DesiredMovementDirection, ForwardDirection);
	}
	if (Input->IsKeyDown('S'))
	{
		DesiredMovementDirection = DirectX::XMVectorSubtract(DesiredMovementDirection, ForwardDirection);
	}
	if (Input->IsKeyDown('D'))
	{
		DesiredMovementDirection = DirectX::XMVectorAdd(DesiredMovementDirection, RightDirection);
	}
	if (Input->IsKeyDown('A'))
	{
		DesiredMovementDirection = DirectX::XMVectorSubtract(DesiredMovementDirection, RightDirection);
	}
	if (Input->IsKeyDown('E'))
	{
		DesiredMovementDirection = DirectX::XMVectorAdd(DesiredMovementDirection, UpDirection);
	}
	if (Input->IsKeyDown('Q'))
	{
		DesiredMovementDirection = DirectX::XMVectorSubtract(DesiredMovementDirection, UpDirection);
	}

	const float NormalizedMovementLength = DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(DesiredMovementDirection));
	if (NormalizedMovementLength > 0.0f)
	{
		DesiredMovementDirection = DirectX::XMVector3Normalize(DesiredMovementDirection);
	}

	float EffectiveMovementSpeed = MovementSpeed;
	FPSSpectateCameraComponent* ActiveFPSSpectateCameraComponent = dynamic_cast<FPSSpectateCameraComponent*>(ActiveCamera);
	if (ActiveFPSSpectateCameraComponent != nullptr)
	{
		EffectiveMovementSpeed *= ActiveFPSSpectateCameraComponent->GetMovementSpeedScale();
	}

	if (Input->IsKeyDown(VK_SHIFT))
	{
		EffectiveMovementSpeed *= 2.0f;
	}

	DirectX::XMVECTOR CurrentPosition = DirectX::XMVectorSet(
		CameraTransform.Position.x,
		CameraTransform.Position.y,
		CameraTransform.Position.z,
		1.0f);
	DirectX::XMVECTOR ScaledMovement = DirectX::XMVectorScale(DesiredMovementDirection, EffectiveMovementSpeed * DeltaTime);
	CurrentPosition = DirectX::XMVectorAdd(CurrentPosition, ScaledMovement);
	DirectX::XMStoreFloat3(&CameraTransform.Position, CurrentPosition);
	CameraActor->SetTransform(CameraTransform);
}
