#pragma once

#include "Tests/TestsBaseGame.h"

class MeshTestGame : public TestsBaseGame
{
public:
	MeshTestGame(LPCWSTR ApplicationName, int ScreenWidth, int ScreenHeight);
	~MeshTestGame() override;

protected:
	void BuildTestScene() override;
};
