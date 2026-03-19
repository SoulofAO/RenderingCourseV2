#pragma once

#include "Abstracts/Input/GameInputHandler.h"

class PlanetsSelectionInputHandler : public GameInputHandler
{
public:
	void HandleInput(Game* OwningGame, InputDevice* Input, float DeltaTime) override;
};
