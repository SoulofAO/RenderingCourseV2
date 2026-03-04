#include "Abstracts/Subsystems/InputDevice.h"

InputDevice::InputDevice(Game* GameInstance)
	: OwningGame(GameInstance)
	, MousePositionX(0)
	, MousePositionY(0)
{
}

void InputDevice::OnKeyDown(unsigned int KeyCode)
{
	PressedKeys.insert(KeyCode);
}

void InputDevice::OnKeyUp(unsigned int KeyCode)
{
	PressedKeys.erase(KeyCode);
}

void InputDevice::OnMouseMove(int PositionX, int PositionY)
{
	MousePositionX = PositionX;
	MousePositionY = PositionY;
}

bool InputDevice::IsKeyDown(unsigned int KeyCode) const
{
	return PressedKeys.count(KeyCode) > 0;
}

int InputDevice::GetMousePositionX() const
{
	return MousePositionX;
}

int InputDevice::GetMousePositionY() const
{
	return MousePositionY;
}
