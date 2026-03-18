#pragma once

#include "Tests/TestsBaseGame.h"

class PhysicsTestGame : public TestsBaseGame
{
public:
	PhysicsTestGame(LPCWSTR ApplicationName, int ScreenWidth, int ScreenHeight);
	~PhysicsTestGame() override;

protected:
	void BuildTestScene() override;
};
