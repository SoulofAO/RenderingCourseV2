#include "Abstracts/Core/Actor.h"
#include "Abstracts/Components/ActorComponent.h"
#include <algorithm>

static uint64_t GlobalActorUniqueIndexCounter = 1;

static Transform ApplyPivotToTransform(const Transform& SourceTransform, const Transform& PivotTransform)
{
	Transform PivotedTransform = SourceTransform;
	PivotedTransform.Position.x += PivotTransform.Position.x;
	PivotedTransform.Position.y += PivotTransform.Position.y;
	PivotedTransform.Position.z += PivotTransform.Position.z;
	PivotedTransform.RotationEuler.x += PivotTransform.RotationEuler.x;
	PivotedTransform.RotationEuler.y += PivotTransform.RotationEuler.y;
	PivotedTransform.RotationEuler.z += PivotTransform.RotationEuler.z;
	PivotedTransform.Scale.x *= PivotTransform.Scale.x;
	PivotedTransform.Scale.y *= PivotTransform.Scale.y;
	PivotedTransform.Scale.z *= PivotTransform.Scale.z;
	return PivotedTransform;
}

static Transform RemovePivotFromTransform(const Transform& PivotedTransform, const Transform& PivotTransform)
{
	Transform SourceTransform = PivotedTransform;
	SourceTransform.Position.x -= PivotTransform.Position.x;
	SourceTransform.Position.y -= PivotTransform.Position.y;
	SourceTransform.Position.z -= PivotTransform.Position.z;
	SourceTransform.RotationEuler.x -= PivotTransform.RotationEuler.x;
	SourceTransform.RotationEuler.y -= PivotTransform.RotationEuler.y;
	SourceTransform.RotationEuler.z -= PivotTransform.RotationEuler.z;
	if (PivotTransform.Scale.x != 0.0f)
	{
		SourceTransform.Scale.x /= PivotTransform.Scale.x;
	}
	if (PivotTransform.Scale.y != 0.0f)
	{
		SourceTransform.Scale.y /= PivotTransform.Scale.y;
	}
	if (PivotTransform.Scale.z != 0.0f)
	{
		SourceTransform.Scale.z /= PivotTransform.Scale.z;
	}
	return SourceTransform;
}

Actor::Actor()
	: OwningGame(nullptr)
	, UniqueIndex(GlobalActorUniqueIndexCounter++)
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

uint64_t Actor::GetUniqueIndex() const
{
	return UniqueIndex;
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

void Actor::SetTransform(const Transform& NewTransform, ETransformSpace TransformSpace)
{
	if (TransformSpace == ETransformSpace::Local)
	{
		LocalTransform = NewTransform;
		return;
	}

	if (ParentActor == nullptr)
	{
		LocalTransform = RemovePivotFromTransform(NewTransform, PivotTransform);
		return;
	}

	const Transform ParentWorldTransform = ParentActor->GetTransform(ETransformSpace::World);
	const Transform PivotedLocalTransform = Transform::MakeRelative(ParentWorldTransform, NewTransform);
	LocalTransform = RemovePivotFromTransform(PivotedLocalTransform, PivotTransform);
}

Transform Actor::GetTransform(ETransformSpace TransformSpace) const
{
	if (TransformSpace == ETransformSpace::Local)
	{
		return LocalTransform;
	}

	const Transform PivotedLocalTransform = ApplyPivotToTransform(LocalTransform, PivotTransform);

	if (ParentActor == nullptr)
	{
		return PivotedLocalTransform;
	}

	const Transform ParentWorldTransform = ParentActor->GetTransform(ETransformSpace::World);
	return Transform::Combine(ParentWorldTransform, PivotedLocalTransform);
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

void Actor::SetLocation(const DirectX::XMFLOAT3& NewLocation, ETransformSpace TransformSpace)
{
	if (TransformSpace == ETransformSpace::Local)
	{
		LocalTransform.Position = NewLocation;
		return;
	}

	Transform WorldTransform = GetTransform(ETransformSpace::World);
	WorldTransform.Position = NewLocation;
	SetTransform(WorldTransform, ETransformSpace::World);
}

DirectX::XMFLOAT3 Actor::GetLocation(ETransformSpace TransformSpace) const
{
	if (TransformSpace == ETransformSpace::Local)
	{
		return LocalTransform.Position;
	}

	return GetTransform(ETransformSpace::World).Position;
}

void Actor::SetRotation(const DirectX::XMFLOAT3& NewRotation, ETransformSpace TransformSpace)
{
	if (TransformSpace == ETransformSpace::Local)
	{
		LocalTransform.RotationEuler = NewRotation;
		return;
	}

	Transform WorldTransform = GetTransform(ETransformSpace::World);
	WorldTransform.RotationEuler = NewRotation;
	SetTransform(WorldTransform, ETransformSpace::World);
}

DirectX::XMFLOAT3 Actor::GetRotation(ETransformSpace TransformSpace) const
{
	if (TransformSpace == ETransformSpace::Local)
	{
		return LocalTransform.RotationEuler;
	}

	return GetTransform(ETransformSpace::World).RotationEuler;
}

void Actor::SetScale(const DirectX::XMFLOAT3& NewScale, ETransformSpace TransformSpace)
{
	if (TransformSpace == ETransformSpace::Local)
	{
		LocalTransform.Scale = NewScale;
		return;
	}

	Transform WorldTransform = GetTransform(ETransformSpace::World);
	WorldTransform.Scale = NewScale;
	SetTransform(WorldTransform, ETransformSpace::World);
}

DirectX::XMFLOAT3 Actor::GetScale(ETransformSpace TransformSpace) const
{
	if (TransformSpace == ETransformSpace::Local)
	{
		return LocalTransform.Scale;
	}

	return GetTransform(ETransformSpace::World).Scale;
}
