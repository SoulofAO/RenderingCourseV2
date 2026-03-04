#include "FirstTaskGame.h"

FirstTaskGame::FirstTaskGame(LPCWSTR ApplicationName, int ScreenWidth, int ScreenHeight)
	: Game(ApplicationName, ScreenWidth, ScreenHeight)
{
}

void FirstTaskGame::Draw()
{
	float ClearColor[] = { TotalTime, 0.1f, 0.1f, 1.0f };
	Context->ClearRenderTargetView(RenderView, ClearColor);

	Game::Draw();
}
