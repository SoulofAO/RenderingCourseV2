#pragma once

#include "Abstracts/Components/CameraComponent.h"

class Actor;
class KatamaryOrbitCameraInputHandler;

class KatamaryOrbitCameraComponent : public CameraComponent
{
public:
	KatamaryOrbitCameraComponent();
	~KatamaryOrbitCameraComponent() override;

	void SetOrbitTargetActor(Actor* NewOrbitTargetActor);
	Actor* GetOrbitTargetActor() const;
	void Posses() override;
	void Unposses() override;
	virtual void Update(float DeltaTime) override;
	void ApplyOrbitInput(float MouseDeltaX, float MouseDeltaY, int MouseWheelDelta, float DeltaTime);

private:
	void ApplyOrbitTransform();

	Actor* OrbitTargetActor;
	float OrbitYawRadians;
	float OrbitPitchRadians;
	float OrbitDistance;
	float RotationSensitivity;
	float ZoomStep;
	bool IsPossessed;
	KatamaryOrbitCameraInputHandler* KatamaryOrbitCameraInputHandlerInstance;
};
