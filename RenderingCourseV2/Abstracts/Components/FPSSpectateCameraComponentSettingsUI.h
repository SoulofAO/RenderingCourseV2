#pragma once

#include "Abstracts/Components/UIRenderingComponent.h"

class FPSSpectateCameraComponent;

class FPSSpectateCameraComponentSettingsUI : public UIRenderingComponent
{
public:
	FPSSpectateCameraComponentSettingsUI();
	~FPSSpectateCameraComponentSettingsUI() override;
	void SetTargetFPSSpectateCameraComponent(FPSSpectateCameraComponent* NewTargetFPSSpectateCameraComponent);

protected:
	void RenderUI() override;

private:
	FPSSpectateCameraComponent* TargetFPSSpectateCameraComponent;
};
