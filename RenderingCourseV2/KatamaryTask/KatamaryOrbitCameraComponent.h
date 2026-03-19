#pragma once

#include "Abstracts/Components/CameraComponent.h"

class Actor;
class InputDevice;

class KatamaryOrbitCameraComponent : public CameraComponent
{
public:
	KatamaryOrbitCameraComponent();
	~KatamaryOrbitCameraComponent() override;

	void SetOrbitTargetActor(Actor* NewOrbitTargetActor);
	Actor* GetOrbitTargetActor() const;
	virtual void Update(float DeltaTime) override;
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
