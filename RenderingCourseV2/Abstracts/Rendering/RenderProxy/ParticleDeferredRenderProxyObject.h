#pragma once

#include "DeferredRendererProxyObject.h"

class ParticleDeferredRenderProxyObject : public DeferredRendererProxyObject
{
public:
    virtual void RenderDeferredGeometryPass(const DeferredGeometryRenderPassState& DeferredGeometryRenderPassStateValue) override;
    virtual void RenderDeferredShadowPass(const DeferredShadowRenderPassState& DeferredShadowRenderPassStateValue) override;
};
