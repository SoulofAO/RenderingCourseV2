#include "Abstracts/Core/PlayerObject.h"

PlayerObject::PlayerObject()
	: PlayerIdentifier(-1)
	, InputSourceIdentifier(-1)
	, PreferredCameraIndex(-1)
	, ViewportRectangle{ 0.0f, 0.0f, 1.0f, 1.0f }
	, ActiveGameIdentifier(-1)
{
}

PlayerObject::~PlayerObject() = default;

void PlayerObject::SetPlayerIdentifier(int NewPlayerIdentifier)
{
	PlayerIdentifier = NewPlayerIdentifier;
}

int PlayerObject::GetPlayerIdentifier() const
{
	return PlayerIdentifier;
}

void PlayerObject::SetInputSourceIdentifier(int NewInputSourceIdentifier)
{
	InputSourceIdentifier = NewInputSourceIdentifier;
}

int PlayerObject::GetInputSourceIdentifier() const
{
	return InputSourceIdentifier;
}

void PlayerObject::SetPreferredCameraIndex(int NewPreferredCameraIndex)
{
	PreferredCameraIndex = NewPreferredCameraIndex;
}

int PlayerObject::GetPreferredCameraIndex() const
{
	return PreferredCameraIndex;
}

void PlayerObject::SetViewportRectangle(const ViewportRectangleNormalized& NewViewportRectangle)
{
	ViewportRectangle = NewViewportRectangle;
}

const ViewportRectangleNormalized& PlayerObject::GetViewportRectangle() const
{
	return ViewportRectangle;
}

void PlayerObject::SetActiveGameIdentifier(int NewActiveGameIdentifier)
{
	ActiveGameIdentifier = NewActiveGameIdentifier;
}

int PlayerObject::GetActiveGameIdentifier() const
{
	return ActiveGameIdentifier;
}
