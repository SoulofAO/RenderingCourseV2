#pragma once

#include "Abstracts/Components/ActorComponent.h"

class SceneViewportSubsystem;

class RenderingComponent : public ActorComponent
{
public:
	RenderingComponent();
	~RenderingComponent() override;

	virtual void Render(SceneViewportSubsystem* SceneViewport) = 0;
};
