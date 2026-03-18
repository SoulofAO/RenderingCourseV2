#pragma once

#include "Abstracts/Components/UIRenderingComponent.h"

class FPSSpectateCameraComponent;
class PlanetsGame;

class FPSSpectateCameraComponentSettingsUI : public UIRenderingComponent
{
public:
	FPSSpectateCameraComponentSettingsUI();
	~FPSSpectateCameraComponentSettingsUI() override;
	void SetTargetFPSSpectateCameraComponent(FPSSpectateCameraComponent* NewTargetFPSSpectateCameraComponent);

protected:
	void RenderUI() override;

private:
	PlanetsGame* GetOwningPlanetsGame() const;

	FPSSpectateCameraComponent* TargetFPSSpectateCameraComponent;
	bool UseOrthographicProjection;
	float OrthographicWidth;
	float OrthographicHeight;
	float FieldOfViewDegrees;
};
