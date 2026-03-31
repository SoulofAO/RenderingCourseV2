#pragma once

#include <memory>
#include <functional>
#include <string>
#include <vector>

class Game;

struct ViewportRectangleNormalized
{
	float Left;
	float Top;
	float Width;
	float Height;
};

struct GameDefinition
{
	int GameIdentifier;
	std::function<std::unique_ptr<Game>()> GameFactory;
};

struct PlayerBinding
{
	int PlayerIdentifier;
	int InputSourceIdentifier;
	int PreferredCameraIndex;
	ViewportRectangleNormalized ViewportRectangle;
	int ActiveGameIdentifier;
};

struct GameConfigurator
{
	std::wstring SessionName;
	bool ReceiveInputWhenSessionIsInactive;
	std::vector<GameDefinition> Games;
	std::vector<PlayerBinding> Players;
};
