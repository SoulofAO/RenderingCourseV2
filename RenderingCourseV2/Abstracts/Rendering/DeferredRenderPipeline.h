#pragma once

#include "Abstracts/Rendering/AbstractRenderPipeline.h"

class DeferredRenderPipeline : public AbstractRenderPipeline
{
public:
	void RenderFrame(
		SceneViewportSubsystem* SceneViewport,
		const std::vector<RenderingComponent*>& RenderingComponents) override;
};
