#include "Abstracts/Rendering/RenderProxy/ParticleDeferredRenderProxyObject.h"
#include "Abstracts/Components/ParticleRenderingComponent.h"

ParticleDeferredRenderProxyObject::ParticleDeferredRenderProxyObject(ParticleRenderingComponent* NewOwnerComponent)
	: OwnerComponent(NewOwnerComponent)
{
}

void ParticleDeferredRenderProxyObject::RenderDeferredGeometryPass(
	const DeferredGeometryRenderPassState& DeferredGeometryRenderPassStateValue)
{
}

void ParticleDeferredRenderProxyObject::RenderDeferredShadowPass(
	const DeferredShadowRenderPassState& DeferredShadowRenderPassStateValue)
{
}
