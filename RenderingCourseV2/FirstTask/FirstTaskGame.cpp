#include "FirstTaskGame.h"
#include "FirstTask/TriangleComponent.h"
#include "Abstracts/Core/Actor.h"
#include "Abstracts/Subsystems/SceneViewportSubsystem.h"

FirstTaskGame::FirstTaskGame(LPCWSTR ApplicationName, int ScreenWidth, int ScreenHeight)
	: Game(ApplicationName, ScreenWidth, ScreenHeight)
{
}

void FirstTaskGame::BeginPlay()
{
	std::unique_ptr<Actor> TriangleActor = std::make_unique<Actor>();
	std::unique_ptr<TriangleComponent> Triangle = std::make_unique<TriangleComponent>();
	TriangleActor->AddComponent(std::move(Triangle));
	AddActor(std::move(TriangleActor));
	GetSubsystem<SceneViewportSubsystem>()->bDisplayChangedColor = true;
	Game::BeginPlay();
}
