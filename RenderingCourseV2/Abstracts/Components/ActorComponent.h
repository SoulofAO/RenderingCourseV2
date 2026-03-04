#pragma once

#include "Abstracts/Core/Object.h"

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

	void SetIsActive(bool NewIsActive);
	bool GetIsActive() const;

protected:
	Actor* OwningActor;
	bool IsActive;
};
