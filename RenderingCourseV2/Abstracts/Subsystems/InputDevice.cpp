#include "Abstracts/Subsystems/InputDevice.h"

InputDevice::InputDevice(Game* GameInstance)
	: OwningGame(GameInstance)
	, MousePositionX(0)
	, MousePositionY(0)
	, MouseDeltaX(0)
	, MouseDeltaY(0)
	, MouseWheelDelta(0)
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

void InputDevice::OnMouseWheel(int NewMouseWheelDelta)
{
	MouseWheelDelta += NewMouseWheelDelta;
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
	MouseWheelDelta = 0;
}

void InputDevice::ResetMouseTracking(int PositionX, int PositionY)
{
	MousePositionX = PositionX;
	MousePositionY = PositionY;
	MouseDeltaX = 0;
	MouseDeltaY = 0;
	HasMousePositionSample = true;
}

int InputDevice::GetMouseDeltaX() const
{
	return MouseDeltaX;
}

int InputDevice::GetMouseDeltaY() const
{
	return MouseDeltaY;
}

int InputDevice::GetMouseWheelDelta() const
{
	return MouseWheelDelta;
}

int InputDevice::GetMousePositionX() const
{
	return MousePositionX;
}

int InputDevice::GetMousePositionY() const
{
	return MousePositionY;
}
