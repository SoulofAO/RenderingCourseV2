#pragma once

#include "Engine/Core/Runtime/Abstract/Core/Game.h"

class FirstTaskGame : public Game
{
public:
	FirstTaskGame(LPCWSTR ApplicationName, int ScreenWidth, int ScreenHeight);

protected:
	void BeginPlay() override;
};

