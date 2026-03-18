#include "FirstTask/FirstTaskGame.h"
#include "Planets/PlanetsGame.h"
#include "PingPongTask/PingPongGame.h"
#include "Tests/LightingTestGame.h"
#include "Tests/MeshTestGame.h"
#include "Tests/PhysicsTestGame.h"
#include "Tests/TexturingTestGame.h"
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

int RunPhysicsTest()
{
	PhysicsTestGame MyGame(L"My3DApp PhysicsTest", 1280, 720);
	MyGame.Initialize();
	MyGame.Run();
	return 0;
}

int RunMeshTest()
{
	MeshTestGame MyGame(L"My3DApp MeshTest", 1280, 720);
	MyGame.Initialize();
	MyGame.Run();
	return 0;
}

int RunTexturingTest()
{
	TexturingTestGame MyGame(L"My3DApp TexturingTest", 1280, 720);
	MyGame.Initialize();
	MyGame.Run();
	return 0;
}

int RunLightingTest()
{
	LightingTestGame MyGame(L"My3DApp LightingTest", 1280, 720);
	MyGame.Initialize();
	MyGame.Run();
	return 0;
}

int main()
{
	RunPhysicsTest();
}

