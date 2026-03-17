#include "Engine/Core/Runtime/Abstract/Core/Actor.h"
#include "Engine/Core/Runtime/Abstract/Core/RuntimeObjectSystem.h"
#include "Engine/Core/Runtime/Abstract/Components/ActorComponent.h"
#include <cstddef>

Actor::Actor()
	: OwningGame(nullptr)
	, WorldTransform()
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
	WorldTransform = NewTransform;
}

const Transform& Actor::GetTransform() const
{
	return WorldTransform;
}

void Actor::SetPosition(const DirectX::XMFLOAT3& NewPosition)
{
	WorldTransform.Position = NewPosition;
}

const DirectX::XMFLOAT3& Actor::GetPosition() const
{
	return WorldTransform.Position;
}

const char* Actor::GetRuntimeClassName() const
{
	return "Actor";
}

void Actor::RegisterActorClass(RuntimeObjectSystem& RuntimeSystem)
{
	RuntimeSystem.RegisterClassInternal(
		"Actor",
		[]() -> std::unique_ptr<UObject>
		{
			return std::make_unique<Actor>();
		},
		[](const UObject& ClassDefaultObject) -> std::unique_ptr<UObject>
		{
			const Actor* SourceActor = dynamic_cast<const Actor*>(&ClassDefaultObject);
			if (SourceActor == nullptr)
			{
				return nullptr;
			}

			std::unique_ptr<Actor> ClonedActor = std::make_unique<Actor>();
			ClonedActor->SetTransform(SourceActor->GetTransform());
			return ClonedActor;
		});
	std::vector<UPropertyDescriptor> PropertyDescriptors;
	PropertyDescriptors.push_back({
		"Position",
		UPropertyType::Float3,
		static_cast<uint32_t>(offsetof(Actor, WorldTransform) + offsetof(Transform, Position)),
		PropertyFlagEditableInDetails });
	PropertyDescriptors.push_back({
		"Rotation",
		UPropertyType::Float3,
		static_cast<uint32_t>(offsetof(Actor, WorldTransform) + offsetof(Transform, RotationEuler)),
		PropertyFlagEditableInDetails });
	PropertyDescriptors.push_back({
		"Scale",
		UPropertyType::Float3,
		static_cast<uint32_t>(offsetof(Actor, WorldTransform) + offsetof(Transform, Scale)),
		PropertyFlagEditableInDetails });
	RuntimeSystem.RegisterPropertyDescriptors("Actor", PropertyDescriptors);
}
