#include "Abstracts/Components/RenderingComponent.h"

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
