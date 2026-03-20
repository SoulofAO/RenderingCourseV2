#pragma once

#include "Abstracts/Rendering/RenderProxy/DeferredRendererProxyObject.h"

class TriangleComponent;

class TriangleDeferredRendererProxyObject : public DeferredRendererProxyObject
{
public:
	explicit TriangleDeferredRendererProxyObject(TriangleComponent* NewOwnerComponent);

	void RenderDeferredGeometryPass(const DeferredGeometryRenderPassState& DeferredGeometryRenderPassStateValue) override;
	void RenderDeferredShadowPass(const DeferredShadowRenderPassState& DeferredShadowRenderPassStateValue) override;

private:
	TriangleComponent* OwnerComponent;
};
