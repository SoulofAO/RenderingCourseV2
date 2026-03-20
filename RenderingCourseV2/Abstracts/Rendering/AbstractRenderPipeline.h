#pragma once

#include <vector>

class SceneViewportSubsystem;
class RenderingComponent;

class AbstractRenderPipeline
{
public:
	virtual ~AbstractRenderPipeline() = default;

	virtual void RenderFrame(
		SceneViewportSubsystem* SceneViewport,
		const std::vector<RenderingComponent*>& RenderingComponents) = 0;
};
