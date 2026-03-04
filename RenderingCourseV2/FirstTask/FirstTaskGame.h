#pragma once

#include "Abstracts/Game.h"

class FirstTaskGame : public Game
{
public:
	FirstTaskGame(LPCWSTR ApplicationName, int ScreenWidth, int ScreenHeight);

protected:
	void Draw() override;
};
