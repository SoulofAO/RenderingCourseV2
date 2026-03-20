#pragma once

#include "Abstracts/Rendering/AbstractRenderPipeline.h"

class ForwardRenderPipeline : public AbstractRenderPipeline
{
public:
	void RenderFrame(
		SceneViewportSubsystem* SceneViewport,
		const std::vector<RenderingComponent*>& RenderingComponents) override;
};
