#pragma once

#include "Abstracts/Core/Object.h"

class Game;

class Subsystem : public Object
{
public:
	Subsystem();
	~Subsystem() override;

	void SetOwningGame(Game* GameInstance);
	Game* GetOwningGame() const;

protected:
	Game* OwningGame;
};
