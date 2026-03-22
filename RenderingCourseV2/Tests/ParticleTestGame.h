#pragma once

#include "Tests/TestsBaseGame.h"

class ParticleTestGame : public TestsBaseGame
{
public:
	ParticleTestGame(LPCWSTR ApplicationName, int ScreenWidth, int ScreenHeight);
	~ParticleTestGame() override;

protected:
	void BuildTestScene() override;
};
