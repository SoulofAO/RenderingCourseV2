#include "Game.h"
#include "TriangleComponent.h"
#include <memory>

int main()
{
	Game MyGame(L"My3DApp", 800, 800);
	MyGame.AddComponent(std::make_unique<TriangleComponent>(&MyGame));
	MyGame.Initialize();
	MyGame.Run();

	return 0;
}
