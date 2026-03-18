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

	
	DirectX::XMFLOAT3 StartPosition = OrbitTargetActor->GetTransform().Position;
	StartPosition.z = OrbitTargetActor->GetTransform().Position.z + OrbitDistance;
	OwningActor->SetPosition(StartPosition);
	OwningActor->SetRotation(MainMathLibrary::DirectionToRotationEuler(DirectX::XMFLOAT3(0,0,-1)));
}
