#include "Abstracts/Components/RenderingComponent.h"
#include "Abstracts/Rendering/RenderProxy/DeferredRendererProxyObject.h"
#include "Abstracts/Rendering/RenderProxy/ForwardRendererProxyObject.h"
#include "Abstracts/Rendering/RenderProxy/RenderingProxyObject.h"
#include "Abstracts/Subsystems/SceneViewportSubsystem.h"

RenderingComponent::RenderingComponent()
	: RenderOrder(0)
{
}

RenderingComponent::~RenderingComponent() = default;

void RenderingComponent::SetRenderOrder(int NewRenderOrder)
{
	RenderOrder = NewRenderOrder;
}

int RenderingComponent::GetRenderOrder() const
{
	return RenderOrder;
}

ForwardRendererProxyObject* RenderingComponent::GetForwardRendererProxyObject() const
{
	return ForwardRendererProxyObjectInstance.get();
}

DeferredRendererProxyObject* RenderingComponent::GetDeferredRendererProxyObject() const
{
	return DeferredRendererProxyObjectInstance.get();
}

RenderingProxyObject* RenderingComponent::GetRendererProxyObject(RenderPipelineType RenderPipelineTypeValue) const
{
	if (RenderPipelineTypeValue == RenderPipelineType::Deferred)
	{
		return DeferredRendererProxyObjectInstance.get();
	}

	return ForwardRendererProxyObjectInstance.get();
}

void RenderingComponent::SetForwardRendererProxyObject(std::unique_ptr<ForwardRendererProxyObject> NewForwardRendererProxyObject)
{
	ForwardRendererProxyObjectInstance = std::move(NewForwardRendererProxyObject);
}

void RenderingComponent::SetDeferredRendererProxyObject(std::unique_ptr<DeferredRendererProxyObject> NewDeferredRendererProxyObject)
{
	DeferredRendererProxyObjectInstance = std::move(NewDeferredRendererProxyObject);
}
