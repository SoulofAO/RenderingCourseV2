#pragma once

class Game;
class InputDevice;

class GameInputHandler
{
public:
	virtual ~GameInputHandler() = default;
	virtual void HandleInput(Game* OwningGame, InputDevice* Input, float DeltaTime) = 0;
	
	bool bEnable = true;
};
