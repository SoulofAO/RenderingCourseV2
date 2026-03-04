#include "FirstTask/FirstTaskGame.h"
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

int main()
{
	FirstTask();
	return 0; 
}

