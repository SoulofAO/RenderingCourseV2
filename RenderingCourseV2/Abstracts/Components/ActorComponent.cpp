#include "Abstracts/Components/ActorComponent.h"
#include "Abstracts/Core/Actor.h"

ActorComponent::ActorComponent()
	: OwningActor(nullptr)
	, LocalTransform()
	, IsActive(true)
{
}

ActorComponent::~ActorComponent() = default;

void ActorComponent::SetOwningActor(Actor* ActorInstance)
{
	OwningActor = ActorInstance;
}

Actor* ActorComponent::GetOwningActor() const
{
	return OwningActor;
}

Game* ActorComponent::GetOwningGame() const
{
	if (OwningActor == nullptr)
	{
		return nullptr;
	}

	return OwningActor->GetOwningGame();
}

void ActorComponent::SetLocalTransform(const Transform& NewLocalTransform)
{
	LocalTransform = NewLocalTransform;
}

const Transform& ActorComponent::GetLocalTransform() const
{
	return LocalTransform;
}

Transform ActorComponent::GetWorldTransform() const
{
	if (OwningActor == nullptr)
	{
		return LocalTransform;
	}

	return Transform::Combine(OwningActor->GetTransform(), LocalTransform);
}

void ActorComponent::SetIsActive(bool NewIsActive)
{
	IsActive = NewIsActive;
}

bool ActorComponent::GetIsActive() const
{
	return IsActive;
}
