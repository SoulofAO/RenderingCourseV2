#pragma once
#include "Abstracts/Core/Game.h"


class PingPongGame : public Game
{
public:
	PingPongGame(LPCWSTR ApplicationName, int ScreenWidth, int ScreenHeight);

protected:
	void BeginPlay() override;
	void Update(float DeltaTime) override;
    
	int WinConditionPoint;
};
