#pragma once

#include "Abstracts/Rendering/RenderProxy/RenderingProxyObject.h"
#include "Abstracts/Rendering/RenderProxy/RenderingProxyPassState.h"

class DeferredRendererProxyObject : public RenderingProxyObject
{
public:
	virtual void RenderDeferredGeometryPass(const DeferredGeometryRenderPassState& DeferredGeometryRenderPassStateValue) = 0;
	virtual void RenderDeferredShadowPass(const DeferredShadowRenderPassState& DeferredShadowRenderPassStateValue) = 0;
	virtual void RenderDeferredStencilShadowVolumePass(const DeferredStencilShadowRenderPassState& DeferredStencilShadowRenderPassStateValue)
	{
	}
};
