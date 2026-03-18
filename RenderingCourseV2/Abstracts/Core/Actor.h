#pragma once

#include "Abstracts/Core/Object.h"
#include "Abstracts/Core/Transform.h"
#include <memory>
#include <vector>

class Game;
class ActorComponent;

class Actor : public Object
{
public:
	Actor();
	~Actor() override;

	void SetOwningGame(Game* GameInstance);
	Game* GetOwningGame() const;

	void Initialize() override;
	void Update(float DeltaTime) override;
	void Shutdown() override;

	void AddComponent(std::unique_ptr<ActorComponent> Component);
	const std::vector<std::unique_ptr<ActorComponent>>& GetComponents() const;

	void SetTransform(const Transform& NewTransform);
	Transform GetTransform() const;
	const Transform& GetLocalTransform() const;
	void SetPivotTransform(const Transform& NewPivotTransform);
	const Transform& GetPivotTransform() const;
	void AttachToActor(Actor* NewParentActor);
	void DetachFromActor();
	Actor* GetParentActor() const;
	const std::vector<Actor*>& GetChildActors() const;

	void SetPosition(const DirectX::XMFLOAT3& NewPosition);
	const DirectX::XMFLOAT3& GetPosition() const;
	void SetRotation(const DirectX::XMFLOAT3& NewRotation);
	const DirectX::XMFLOAT3& GetRotation() const;
	void SetScale(const DirectX::XMFLOAT3& NewScale);
	const DirectX::XMFLOAT3& GetScale() const;

private:
	void RemoveChildActorReference(Actor* ExistingChildActor);

	Game* OwningGame;
	std::vector<std::unique_ptr<ActorComponent>> Components;
	Transform LocalTransform;
	Transform PivotTransform;
	Actor* ParentActor;
	std::vector<Actor*> ChildActors;
};
