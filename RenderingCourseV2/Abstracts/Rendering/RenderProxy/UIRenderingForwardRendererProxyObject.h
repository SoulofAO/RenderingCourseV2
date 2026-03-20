#pragma once

#include "Abstracts/Rendering/RenderProxy/ForwardRendererProxyObject.h"

class UIRenderingComponent;

class UIRenderingForwardRendererProxyObject : public ForwardRendererProxyObject
{
public:
	explicit UIRenderingForwardRendererProxyObject(UIRenderingComponent* NewOwnerComponent);

	void RenderForwardMainPass(const ForwardMainRenderPassState& ForwardMainRenderPassStateValue) override;

private:
	UIRenderingComponent* OwnerComponent;
};
