#include "Abstracts/Rendering/RenderProxy/UIRenderingDeferredRendererProxyObject.h"
#include "Abstracts/Components/UIRenderingComponent.h"

UIRenderingDeferredRendererProxyObject::UIRenderingDeferredRendererProxyObject(UIRenderingComponent* NewOwnerComponent)
	: OwnerComponent(NewOwnerComponent)
{
}

void UIRenderingDeferredRendererProxyObject::RenderDeferredGeometryPass(const DeferredGeometryRenderPassState& DeferredGeometryRenderPassStateValue)
{
	if (OwnerComponent == nullptr)
	{
		return;
	}

	if (DeferredGeometryRenderPassStateValue.IsDearImGuiInitialized)
	{
		OwnerComponent->RenderUI();
	}
}

void UIRenderingDeferredRendererProxyObject::RenderDeferredShadowPass(const DeferredShadowRenderPassState& DeferredShadowRenderPassStateValue)
{
	(void)DeferredShadowRenderPassStateValue;
}
