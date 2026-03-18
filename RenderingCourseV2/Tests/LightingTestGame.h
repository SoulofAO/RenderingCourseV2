#pragma once

#include "Tests/TestsBaseGame.h"

class LightingTestGame : public TestsBaseGame
{
public:
	LightingTestGame(LPCWSTR ApplicationName, int ScreenWidth, int ScreenHeight);
	~LightingTestGame() override;

protected:
	void BuildTestScene() override;
};
