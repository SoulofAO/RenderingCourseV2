#pragma once

#include "Tests/TestsBaseGame.h"

class PointLightShadowWallsTestGame : public TestsBaseGame
{
public:
	PointLightShadowWallsTestGame(LPCWSTR ApplicationName, int ScreenWidth, int ScreenHeight);
	~PointLightShadowWallsTestGame() override;

protected:
	void BuildTestScene() override;
};
