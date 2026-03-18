#pragma once

#include "Abstracts/Components/UIRenderingComponent.h"

class DefaultCameraSettingsUIRenderingComponent : public UIRenderingComponent
{
public:
	DefaultCameraSettingsUIRenderingComponent();
	~DefaultCameraSettingsUIRenderingComponent() override;

protected:
	void RenderUI() override;
};
