#pragma once

#include "Abstracts/Components/ActorComponent.h"
#include <memory>

class RenderingProxyObject;
class ForwardRendererProxyObject;
class DeferredRendererProxyObject;
enum class RenderPipelineType;

class RenderingComponent : public ActorComponent
{
public:
	RenderingComponent();
	~RenderingComponent() override;

	void SetRenderOrder(int NewRenderOrder);
	int GetRenderOrder() const;
	ForwardRendererProxyObject* GetForwardRendererProxyObject() const;
	DeferredRendererProxyObject* GetDeferredRendererProxyObject() const;
	RenderingProxyObject* GetRendererProxyObject(RenderPipelineType RenderPipelineTypeValue) const;

protected:
	void SetForwardRendererProxyObject(std::unique_ptr<ForwardRendererProxyObject> NewForwardRendererProxyObject);
	void SetDeferredRendererProxyObject(std::unique_ptr<DeferredRendererProxyObject> NewDeferredRendererProxyObject);

private:
	int RenderOrder;
	std::unique_ptr<ForwardRendererProxyObject> ForwardRendererProxyObjectInstance;
	std::unique_ptr<DeferredRendererProxyObject> DeferredRendererProxyObjectInstance;
};
