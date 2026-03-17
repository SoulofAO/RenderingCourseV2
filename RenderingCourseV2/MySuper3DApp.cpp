#include "FirstTask/FirstTaskGame.h"
#include "Planets/PlanetsGame.h"
#include "PingPongTask/PingPongGame.h"
#include <memory>

int FirstTask()
{
	FirstTaskGame MyGame(L"My3DApp", 800, 800);
	MyGame.Initialize();
	MyGame.Run();

	return 0;
}

int PingPongTask()
{
	PingPongGame MyGame(L"My3DApp PingPong", 1200, 800);
	MyGame.Initialize();
	MyGame.Run();

	return 0;
}

int PlanetsTask()
{
	PlanetsGame MyGame(L"My3DApp Planets", 1280, 720);
	MyGame.Initialize();
	MyGame.Run();

	return 0;
}

int main()
{
	PlanetsTask();
	return 0; 
}

