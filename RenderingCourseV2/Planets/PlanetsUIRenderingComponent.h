#pragma once

#include "Abstracts/Components/UIRenderingComponent.h"

class PlanetsGame;

class PlanetsUIRenderingComponent : public UIRenderingComponent
{
public:
	PlanetsUIRenderingComponent();
	~PlanetsUIRenderingComponent() override;

protected:
	void RenderUI() override;

private:
	PlanetsGame* GetOwningPlanetsGame() const;
};
