#include "Abstracts/Core/Actor.h"
#include "Abstracts/Components/ActorComponent.h"

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
