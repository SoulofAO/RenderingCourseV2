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
	bool WasKeyPressedThisFrame(unsigned int KeyCode) const;
	void EndFrame();
	int GetMouseDeltaX() const;
	int GetMouseDeltaY() const;
	int GetMousePositionX() const;
	int GetMousePositionY() const;

private:
	Game* OwningGame;
	std::unordered_set<unsigned int> PressedKeys;
	std::unordered_set<unsigned int> PressedKeysThisFrame;
	int MousePositionX;
	int MousePositionY;
	int MouseDeltaX;
	int MouseDeltaY;
	bool HasMousePositionSample;
};
