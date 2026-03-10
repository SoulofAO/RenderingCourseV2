#pragma once

#include "Engine/Core/Runtime/Abstract/Components/ActorComponent.h"

class SceneViewportSubsystem;

class RenderingComponent : public ActorComponent
{
public:
	RenderingComponent();
	~RenderingComponent() override;

	void SetRenderOrder(int NewRenderOrder);
	int GetRenderOrder() const;

	virtual void Render(SceneViewportSubsystem* SceneViewport) = 0;

private:
	int RenderOrder;
};

