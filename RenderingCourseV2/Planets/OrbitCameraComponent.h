#pragma once

#include "Abstracts/Components/CameraComponent.h"

class Actor;
class OrbitCameraInputHandler;
class PlanetsSelectionInputHandler;

class OrbitCameraComponent : public CameraComponent
{
public:
	OrbitCameraComponent();
	~OrbitCameraComponent() override;

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
	OrbitCameraInputHandler* OrbitCameraInputHandlerInstance;
	PlanetsSelectionInputHandler* PlanetsSelectionInputHandlerInstance;
};
