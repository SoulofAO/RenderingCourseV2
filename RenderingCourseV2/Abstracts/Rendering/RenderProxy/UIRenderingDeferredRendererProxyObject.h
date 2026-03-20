#pragma once

#include "Abstracts/Rendering/RenderProxy/DeferredRendererProxyObject.h"

class UIRenderingComponent;

class UIRenderingDeferredRendererProxyObject : public DeferredRendererProxyObject
{
public:
	explicit UIRenderingDeferredRendererProxyObject(UIRenderingComponent* NewOwnerComponent);

	void RenderDeferredGeometryPass(const DeferredGeometryRenderPassState& DeferredGeometryRenderPassStateValue) override;
	void RenderDeferredShadowPass(const DeferredShadowRenderPassState& DeferredShadowRenderPassStateValue) override;

private:
	UIRenderingComponent* OwnerComponent;
};
