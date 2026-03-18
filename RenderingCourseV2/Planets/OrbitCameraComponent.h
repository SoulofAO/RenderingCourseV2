#pragma once

#include "Abstracts/Components/CameraComponent.h"

class Actor;
class InputDevice;

class OrbitCameraComponent : public CameraComponent
{
public:
	OrbitCameraComponent();
	~OrbitCameraComponent() override;

	void SetOrbitTargetActor(Actor* NewOrbitTargetActor);
	Actor* GetOrbitTargetActor() const;
	void HandleOrbitInput(InputDevice* Input, float DeltaTime);

private:
	void ApplyOrbitTransform();

	Actor* OrbitTargetActor;
	float OrbitYawRadians;
	float OrbitPitchRadians;
	float OrbitDistance;
	float RotationSensitivity;
	float ZoomStep;
};
