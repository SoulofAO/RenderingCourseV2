#pragma once

#include "Abstracts/Rendering/RenderProxy/ForwardRendererProxyObject.h"

class ParticleForwardRenderProxyObject : public ForwardRendererProxyObject
{
public:
    virtual void RenderForwardMainPass(const ForwardMainRenderPassState& ForwardMainRenderPassStateValue) override;
};
