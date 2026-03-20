#pragma once

#include "Abstracts/Rendering/RenderProxy/ForwardRendererProxyObject.h"

class MeshUniversalComponent;

class MeshUniversalForwardRendererProxyObject : public ForwardRendererProxyObject
{
public:
	explicit MeshUniversalForwardRendererProxyObject(MeshUniversalComponent* NewOwnerComponent);

	void RenderForwardMainPass(const ForwardMainRenderPassState& ForwardMainRenderPassStateValue) override;

private:
	MeshUniversalComponent* OwnerComponent;
};
