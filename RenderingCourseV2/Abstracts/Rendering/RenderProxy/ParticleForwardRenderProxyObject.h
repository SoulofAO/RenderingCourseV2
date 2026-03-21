#pragma once

#include "Abstracts/Rendering/RenderProxy/ForwardRendererProxyObject.h"

class ParticleRenderingComponent;

class ParticleForwardRenderProxyObject : public ForwardRendererProxyObject
{
public:
	explicit ParticleForwardRenderProxyObject(ParticleRenderingComponent* NewOwnerComponent);

	void RenderForwardMainPass(const ForwardMainRenderPassState& ForwardMainRenderPassStateValue) override;

private:
	ParticleRenderingComponent* OwnerComponent;
};
