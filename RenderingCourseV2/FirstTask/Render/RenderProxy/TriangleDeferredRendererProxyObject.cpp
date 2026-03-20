#include "FirstTask/Render/RenderProxy/TriangleDeferredRendererProxyObject.h"
#include "FirstTask/Render/RenderProxy/TriangleForwardRendererProxyObject.h"
#include "FirstTask/TriangleComponent.h"

TriangleDeferredRendererProxyObject::TriangleDeferredRendererProxyObject(TriangleComponent* NewOwnerComponent)
	: OwnerComponent(NewOwnerComponent)
{
}

void TriangleDeferredRendererProxyObject::RenderDeferredGeometryPass(const DeferredGeometryRenderPassState& DeferredGeometryRenderPassStateValue)
{
	ForwardMainRenderPassState ForwardMainRenderPassStateValue = {};
	ForwardMainRenderPassStateValue.DeviceContext = DeferredGeometryRenderPassStateValue.DeviceContext;

	if (OwnerComponent == nullptr)
	{
		return;
	}

	TriangleForwardRendererProxyObject ForwardRendererProxyObjectInstance(OwnerComponent);
	ForwardRendererProxyObjectInstance.RenderForwardMainPass(ForwardMainRenderPassStateValue);
}

void TriangleDeferredRendererProxyObject::RenderDeferredShadowPass(const DeferredShadowRenderPassState& DeferredShadowRenderPassStateValue)
{
	(void)DeferredShadowRenderPassStateValue;
}
