#pragma once

#include "Abstracts/Rendering/RenderProxy/ForwardRendererProxyObject.h"

class TriangleComponent;

class TriangleForwardRendererProxyObject : public ForwardRendererProxyObject
{
public:
	explicit TriangleForwardRendererProxyObject(TriangleComponent* NewOwnerComponent);

	void RenderForwardMainPass(const ForwardMainRenderPassState& ForwardMainRenderPassStateValue) override;

private:
	TriangleComponent* OwnerComponent;
};
