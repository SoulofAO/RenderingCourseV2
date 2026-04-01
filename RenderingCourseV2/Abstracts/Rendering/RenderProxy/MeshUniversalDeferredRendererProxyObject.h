#pragma once

#include "Abstracts/Rendering/RenderProxy/DeferredRendererProxyObject.h"

class MeshUniversalComponent;

class MeshUniversalDeferredRendererProxyObject : public DeferredRendererProxyObject
{
public:
	explicit MeshUniversalDeferredRendererProxyObject(MeshUniversalComponent* NewOwnerComponent);

	void RenderDeferredGeometryPass(const DeferredGeometryRenderPassState& DeferredGeometryRenderPassStateValue) override;
	void RenderDeferredShadowPass(const DeferredShadowRenderPassState& DeferredShadowRenderPassStateValue) override;
	void RenderDeferredStencilShadowVolumePass(const DeferredStencilShadowRenderPassState& DeferredStencilShadowRenderPassStateValue) override;

private:
	MeshUniversalComponent* OwnerComponent;
};
