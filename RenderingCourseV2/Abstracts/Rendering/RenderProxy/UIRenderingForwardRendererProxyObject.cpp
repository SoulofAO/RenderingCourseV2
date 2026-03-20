#include "Abstracts/Rendering/RenderProxy/UIRenderingForwardRendererProxyObject.h"
#include "Abstracts/Components/UIRenderingComponent.h"

UIRenderingForwardRendererProxyObject::UIRenderingForwardRendererProxyObject(UIRenderingComponent* NewOwnerComponent)
	: OwnerComponent(NewOwnerComponent)
{
}

void UIRenderingForwardRendererProxyObject::RenderForwardMainPass(const ForwardMainRenderPassState& ForwardMainRenderPassStateValue)
{
	if (OwnerComponent == nullptr)
	{
		return;
	}

	if (ForwardMainRenderPassStateValue.IsDearImGuiInitialized)
	{
		OwnerComponent->RenderUI();
	}
}
