#pragma once

#include "Abstracts/Components/CameraComponent.h"

class EngineHotkeyInputHandler;
class DefaultCameraSettingsUIRenderingComponent;

class FPSSpectateCameraComponent : public CameraComponent
{
public:
	FPSSpectateCameraComponent();
	~FPSSpectateCameraComponent() override;

	void Initialize() override;
	void Shutdown() override;
	void Posses() override;
	void Unposses() override;

private:
	EngineHotkeyInputHandler* EngineHotkeyInputHandlerInstance;
	DefaultCameraSettingsUIRenderingComponent* DefaultCameraSettingsUIRenderingComponentInstance;
	bool IsPossessed;
};
