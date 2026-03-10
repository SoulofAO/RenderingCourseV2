#pragma once

#include "Engine/Core/Runtime/Abstract/Core/Object.h"
#include "Engine/Core/Runtime/Abstract/Core/Transform.h"

class Actor;
class Game;

class ActorComponent : public Object
{
public:
	ActorComponent();
	~ActorComponent() override;

	void SetOwningActor(Actor* ActorInstance);
	Actor* GetOwningActor() const;
	Game* GetOwningGame() const;

	void SetLocalTransform(const Transform& NewLocalTransform);
	const Transform& GetLocalTransform() const;
	Transform GetWorldTransform() const;

	void SetIsActive(bool NewIsActive);
	bool GetIsActive() const;

protected:
	Actor* OwningActor;
	Transform LocalTransform;
	bool IsActive;
};

