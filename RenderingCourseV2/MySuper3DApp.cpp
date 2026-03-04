#include "FirstTask/FirstTaskGame.h"
#include "FirstTask/TriangleComponent.h"
#include <memory>

int FirstTask()
{
	FirstTaskGame MyGame(L"My3DApp", 800, 800);
	MyGame.AddComponent(std::make_unique<TriangleComponent>(&MyGame));
	MyGame.Initialize();
	MyGame.Run();

	return 0;
}

int main()
{
	FirstTask();
	return 0; 
}

