#include "Abstracts/Core/Actor.h"
#include "Abstracts/Components/ActorComponent.h"
#include <algorithm>

Actor::Actor()
	: OwningGame(nullptr)
	, LocalTransform()
	, PivotTransform()
	, ParentActor(nullptr)
{
}

Actor::~Actor()
{
	Shutdown();
}

void Actor::SetOwningGame(Game* GameInstance)
{
	OwningGame = GameInstance;
}

Game* Actor::GetOwningGame() const
{
	return OwningGame;
}

void Actor::Initialize()
{
	if (GetIsInitialized())
	{
		return;
	}

	Object::Initialize();

	for (std::unique_ptr<ActorComponent>& Component : Components)
	{
		Component->Initialize();
	}
}

void Actor::Update(float DeltaTime)
{
	Object::Update(DeltaTime);

	for (std::unique_ptr<ActorComponent>& Component : Components)
	{
		if (Component->GetIsActive())
		{
			Component->Update(DeltaTime);
		}
	}
}

void Actor::Shutdown()
{
	if (GetIsInitialized() == false)
	{
		return;
	}

	DetachFromActor();
	for (Actor* ExistingChildActor : ChildActors)
	{
		if (ExistingChildActor != nullptr)
		{
			ExistingChildActor->ParentActor = nullptr;
		}
	}
	ChildActors.clear();

	for (std::unique_ptr<ActorComponent>& Component : Components)
	{
		Component->Shutdown();
	}

	Object::Shutdown();
}

void Actor::AddComponent(std::unique_ptr<ActorComponent> Component)
{
	Component->SetOwningActor(this);
	Components.push_back(std::move(Component));
}

const std::vector<std::unique_ptr<ActorComponent>>& Actor::GetComponents() const
{
	return Components;
}

void Actor::SetTransform(const Transform& NewTransform)
{
	LocalTransform = NewTransform;
}

Transform Actor::GetTransform() const
{
	Transform PivotedLocalTransform = LocalTransform;
	PivotedLocalTransform.Position.x += PivotTransform.Position.x;
	PivotedLocalTransform.Position.y += PivotTransform.Position.y;
	PivotedLocalTransform.Position.z += PivotTransform.Position.z;
	PivotedLocalTransform.RotationEuler.x += PivotTransform.RotationEuler.x;
	PivotedLocalTransform.RotationEuler.y += PivotTransform.RotationEuler.y;
	PivotedLocalTransform.RotationEuler.z += PivotTransform.RotationEuler.z;
	PivotedLocalTransform.Scale.x *= PivotTransform.Scale.x;
	PivotedLocalTransform.Scale.y *= PivotTransform.Scale.y;
	PivotedLocalTransform.Scale.z *= PivotTransform.Scale.z;

	if (ParentActor == nullptr)
	{
		return PivotedLocalTransform;
	}

	const Transform ParentWorldTransform = ParentActor->GetTransform();
	return Transform::Combine(ParentWorldTransform, PivotedLocalTransform);
}

const Transform& Actor::GetLocalTransform() const
{
	return LocalTransform;
}

void Actor::SetPivotTransform(const Transform& NewPivotTransform)
{
	PivotTransform = NewPivotTransform;
}

const Transform& Actor::GetPivotTransform() const
{
	return PivotTransform;
}

void Actor::AttachToActor(Actor* NewParentActor)
{
	if (ParentActor == NewParentActor)
	{
		return;
	}

	DetachFromActor();
	ParentActor = NewParentActor;
	if (ParentActor != nullptr)
	{
		ParentActor->ChildActors.push_back(this);
	}
}

void Actor::DetachFromActor()
{
	if (ParentActor == nullptr)
	{
		return;
	}

	ParentActor->RemoveChildActorReference(this);
	ParentActor = nullptr;
}

Actor* Actor::GetParentActor() const
{
	return ParentActor;
}

const std::vector<Actor*>& Actor::GetChildActors() const
{
	return ChildActors;
}

void Actor::RemoveChildActorReference(Actor* ExistingChildActor)
{
	auto ChildActorIterator = std::remove(ChildActors.begin(), ChildActors.end(), ExistingChildActor);
	if (ChildActorIterator != ChildActors.end())
	{
		ChildActors.erase(ChildActorIterator, ChildActors.end());
	}
}

void Actor::SetPosition(const DirectX::XMFLOAT3& NewPosition)
{
	LocalTransform.Position = NewPosition;
}

const DirectX::XMFLOAT3& Actor::GetPosition() const
{
	return LocalTransform.Position;
}
