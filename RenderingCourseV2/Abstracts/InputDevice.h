#pragma once

#include <unordered_set>

class Game;

class InputDevice
{
public:
	InputDevice(Game* GameInstance);

	void OnKeyDown(unsigned int KeyCode);
	void OnKeyUp(unsigned int KeyCode);
	void OnMouseMove(int PositionX, int PositionY);

	bool IsKeyDown(unsigned int KeyCode) const;
	int GetMousePositionX() const;
	int GetMousePositionY() const;

private:
	Game* OwningGame;
	std::unordered_set<unsigned int> PressedKeys;
	int MousePositionX;
	int MousePositionY;
};
