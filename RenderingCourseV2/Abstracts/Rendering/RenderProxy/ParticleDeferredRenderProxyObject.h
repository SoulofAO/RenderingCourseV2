#pragma once

#include "Abstracts/Rendering/RenderProxy/DeferredRendererProxyObject.h"

class ParticleRenderingComponent;

class ParticleDeferredRenderProxyObject : public DeferredRendererProxyObject
{
public:
	explicit ParticleDeferredRenderProxyObject(ParticleRenderingComponent* NewOwnerComponent);

	void RenderDeferredGeometryPass(const DeferredGeometryRenderPassState& DeferredGeometryRenderPassStateValue) override;
	void RenderDeferredShadowPass(const DeferredShadowRenderPassState& DeferredShadowRenderPassStateValue) override;

private:
	ParticleRenderingComponent* OwnerComponent;
};
