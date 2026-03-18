#include "Planets/OrbitCameraComponent.h"
#include "Abstracts/Core/Actor.h"
#include "Abstracts/Core/Transform.h"
#include "Abstracts/Subsystems/InputDevice.h"
#include <algorithm>
#include <cmath>

OrbitCameraComponent::OrbitCameraComponent()
	: CameraComponent()
	, OrbitTargetActor(nullptr)
	, OrbitYawRadians(0.0f)
	, OrbitPitchRadians(0.25f)
	, OrbitDistance(8.0f)
	, RotationSensitivity(0.0025f)
	, ZoomStep(1.0f)
{
}

OrbitCameraComponent::~OrbitCameraComponent() = default;

void OrbitCameraComponent::SetOrbitTargetActor(Actor* NewOrbitTargetActor)
{
	OrbitTargetActor = NewOrbitTargetActor;

	Actor* OwningActor = GetOwningActor();
	if (OwningActor != nullptr)
	{
		OwningActor->AttachToActor(OrbitTargetActor);
	}

	ApplyOrbitTransform();
}

Actor* OrbitCameraComponent::GetOrbitTargetActor() const
{
	return OrbitTargetActor;
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

	Transform OrbitTransform = OwningActor->GetLocalTransform();
	const float CosinePitch = std::cos(OrbitPitchRadians);
	const float SinePitch = std::sin(OrbitPitchRadians);
	const float SineYaw = std::sin(OrbitYawRadians);
	const float CosineYaw = std::cos(OrbitYawRadians);

	OrbitTransform.Position.x = SineYaw * CosinePitch * OrbitDistance;
	OrbitTransform.Position.y = SinePitch * OrbitDistance;
	OrbitTransform.Position.z = -CosineYaw * CosinePitch * OrbitDistance;

	const float ForwardX = -OrbitTransform.Position.x;
	const float ForwardY = -OrbitTransform.Position.y;
	const float ForwardZ = -OrbitTransform.Position.z;
	const float ForwardLength = std::sqrt((ForwardX * ForwardX) + (ForwardY * ForwardY) + (ForwardZ * ForwardZ));
	if (ForwardLength > 0.00001f)
	{
		const float InverseForwardLength = 1.0f / ForwardLength;
		const float NormalizedForwardX = ForwardX * InverseForwardLength;
		const float NormalizedForwardY = ForwardY * InverseForwardLength;
		const float NormalizedForwardZ = ForwardZ * InverseForwardLength;
		OrbitTransform.RotationEuler.x = std::asin((std::clamp)(NormalizedForwardY, -1.0f, 1.0f));
		OrbitTransform.RotationEuler.y = std::atan2(NormalizedForwardX, NormalizedForwardZ);
		OrbitTransform.RotationEuler.z = 0.0f;
	}

	OwningActor->SetTransform(OrbitTransform);
}
