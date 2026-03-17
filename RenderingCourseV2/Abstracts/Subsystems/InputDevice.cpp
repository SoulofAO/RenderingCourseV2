#include "Abstracts/Subsystems/InputDevice.h"

InputDevice::InputDevice(Game* GameInstance)
	: OwningGame(GameInstance)
	, MousePositionX(0)
	, MousePositionY(0)
	, MouseDeltaX(0)
	, MouseDeltaY(0)
	, HasMousePositionSample(false)
{
}

void InputDevice::OnKeyDown(unsigned int KeyCode)
{
	if (PressedKeys.count(KeyCode) == 0)
	{
		PressedKeysThisFrame.insert(KeyCode);
	}

	PressedKeys.insert(KeyCode);
}

void InputDevice::OnKeyUp(unsigned int KeyCode)
{
	PressedKeys.erase(KeyCode);
}

void InputDevice::OnMouseMove(int PositionX, int PositionY)
{
	if (HasMousePositionSample)
	{
		MouseDeltaX += PositionX - MousePositionX;
		MouseDeltaY += PositionY - MousePositionY;
	}

	MousePositionX = PositionX;
	MousePositionY = PositionY;
	HasMousePositionSample = true;
}

bool InputDevice::IsKeyDown(unsigned int KeyCode) const
{
	return PressedKeys.count(KeyCode) > 0;
}

bool InputDevice::WasKeyPressedThisFrame(unsigned int KeyCode) const
{
	return PressedKeysThisFrame.count(KeyCode) > 0;
}

void InputDevice::EndFrame()
{
	PressedKeysThisFrame.clear();
	MouseDeltaX = 0;
	MouseDeltaY = 0;
}

int InputDevice::GetMouseDeltaX() const
{
	return MouseDeltaX;
}

int InputDevice::GetMouseDeltaY() const
{
	return MouseDeltaY;
}

int InputDevice::GetMousePositionX() const
{
	return MousePositionX;
}

int InputDevice::GetMousePositionY() const
{
	return MousePositionY;
}
