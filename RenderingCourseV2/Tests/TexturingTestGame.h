#pragma once

#include "Tests/TestsBaseGame.h"

class TexturingTestGame : public TestsBaseGame
{
public:
	TexturingTestGame(LPCWSTR ApplicationName, int ScreenWidth, int ScreenHeight);
	~TexturingTestGame() override;

protected:
	void BuildTestScene() override;
};
