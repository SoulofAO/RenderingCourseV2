#pragma once

#include "Abstracts/Components/CameraComponent.h"

class FPSSpectateCameraComponentSettingsUI;
class FreeCameraInputHandler;

class FPSSpectateCameraComponent : public CameraComponent
{
public:
	FPSSpectateCameraComponent();
	~FPSSpectateCameraComponent() override;

	void Initialize() override;
	void Shutdown() override;
	void Posses() override;
	void Unposses() override;
	void SetMovementSpeedScale(float NewMovementSpeedScale);
	float GetMovementSpeedScale() const;
	bool GetIsPossessed() const;

private:
	FPSSpectateCameraComponentSettingsUI* FPSSpectateCameraComponentSettingsUIInstance;
	FreeCameraInputHandler* FreeCameraInputHandlerInstance;
	float MovementSpeedScale;
	bool IsPossessed;
};
