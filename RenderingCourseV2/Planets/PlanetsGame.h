#pragma once

#include "Abstracts/Core/Game.h"

class PlanetsGame : public Game
{
public:
	PlanetsGame(LPCWSTR ApplicationName, int ScreenWidth, int ScreenHeight);

protected:
	void BeginPlay() override;
};
