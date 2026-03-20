#pragma once

#include "Abstracts/Rendering/RenderProxy/RenderingProxyObject.h"
#include "Abstracts/Rendering/RenderProxy/RenderingProxyPassState.h"

class ForwardRendererProxyObject : public RenderingProxyObject
{
public:
	virtual void RenderForwardMainPass(const ForwardMainRenderPassState& ForwardMainRenderPassStateValue) = 0;
};
