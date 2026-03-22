#pragma once

#include "Abstracts/Components/UIRenderingComponent.h"

class ParticleTestUIRenderingComponent : public UIRenderingComponent
{
public:
	ParticleTestUIRenderingComponent();
	~ParticleTestUIRenderingComponent() override;

protected:
	void RenderUI() override;
};
