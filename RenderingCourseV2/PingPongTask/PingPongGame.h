#pragma once
#include "Abstracts/Game.h"


class PingPongGame : public Game
{
    virtual void BeginPlay();
    virtual void Update(float DeltaTime);
    
    int WinConditionPoint = 5;
};
