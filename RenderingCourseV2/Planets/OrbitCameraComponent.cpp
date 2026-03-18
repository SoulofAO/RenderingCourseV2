#include "Planets/OrbitCameraComponent.h"
#include "Abstracts/Core/Actor.h"
#include "Abstracts/Core/Transform.h"
#include "Abstracts/Subsystems/InputDevice.h"
#include <algorithm>
#include <cmath>
#include "Abstracts/Others/MainMathLibrary.h"

OrbitCameraComponent::OrbitCameraComponent()
	: CameraComponent()
	, OrbitTargetActor(nullptr)
	, OrbitYawRadians(3.14159265358979323846f)
	, OrbitPitchRadians(0.25f)
	, OrbitDistance(14.0f)
	, RotationSensitivity(0.0025f)
	, ZoomStep(1.0f)
{
}

OrbitCameraComponent::~OrbitCameraComponent() = default;

void OrbitCameraComponent::SetOrbitTargetActor(Actor* NewOrbitTargetActor)
{
	OrbitTargetActor = NewOrbitTargetActor;

	ApplyOrbitTransform();
}

Actor* OrbitCameraComponent::GetOrbitTargetActor() const
{
	return OrbitTargetActor;
}

void OrbitCameraComponent::Update(float DeltaTime)
{
	CameraComponent::Update(DeltaTime);
	ApplyOrbitTransform();
}

void OrbitCameraComponent::HandleOrbitInput(InputDevice* Input, float DeltaTime)
{
	if (Input == nullptr || OrbitTargetActor == nullptr || DeltaTime <= 0.0f)
	{
		return;
	}

	const float MouseDeltaX = static_cast<float>(Input->GetMouseDeltaX());
	const float MouseDeltaY = static_cast<float>(Input->GetMouseDeltaY());
	OrbitYawRadians += MouseDeltaX * RotationSensitivity;
	OrbitPitchRadians += MouseDeltaY * RotationSensitivity;
	OrbitPitchRadians = (std::clamp)(OrbitPitchRadians, -1.3f, 1.3f);

	const int MouseWheelDelta = Input->GetMouseWheelDelta();
	if (MouseWheelDelta != 0)
	{
		const float MouseWheelStep = static_cast<float>(MouseWheelDelta) / 120.0f;
		OrbitDistance -= MouseWheelStep * ZoomStep;
		OrbitDistance = (std::clamp)(OrbitDistance, 2.0f, 80.0f);
	}

	ApplyOrbitTransform();
}

void OrbitCameraComponent::ApplyOrbitTransform()
{
	if (OrbitTargetActor == nullptr)
	{
		return;
	}

	Actor* OwningActor = GetOwningActor();
	if (OwningActor == nullptr)
	{
		return;
	}

	const DirectX::XMFLOAT3 OrbitTargetPosition = OrbitTargetActor->GetTransform(ETransformSpace::World).Position;
	const DirectX::XMFLOAT3 OrbitRotationEuler = DirectX::XMFLOAT3(
		OrbitPitchRadians,
		OrbitYawRadians,
		0.0f);
	const DirectX::XMFLOAT3 OrbitLookDirection = MainMathLibrary::RotationEulerToForwardVector(OrbitRotationEuler);

	DirectX::XMFLOAT3 CameraPosition = OrbitTargetPosition;
	CameraPosition.x -= OrbitLookDirection.x * OrbitDistance;
	CameraPosition.y -= OrbitLookDirection.y * OrbitDistance;
	CameraPosition.z -= OrbitLookDirection.z * OrbitDistance;

	OwningActor->SetLocation(CameraPosition, ETransformSpace::World);
	OwningActor->SetRotation(MainMathLibrary::DirectionToRotationEuler(OrbitLookDirection), ETransformSpace::World);
}
