#include "PingPongGame.h"
#include "PingPongPlane.h"
#include "PingPongSphere.h"
#include <directxmath.h>
#include <memory>

PingPongGame::PingPongGame(LPCWSTR ApplicationName, int ScreenWidth, int ScreenHeight)
	: Game(ApplicationName, ScreenWidth, ScreenHeight)
	, WinConditionPoint(5)
{
}

void PingPongGame::BeginPlay()
{
	std::unique_ptr<PingPongPlane> FloorPlane = std::make_unique<PingPongPlane>();
	FloorPlane->SetPosition(DirectX::XMFLOAT3(0.0f, -1.25f, 2.5f));
	AddActor(std::move(FloorPlane));

	std::unique_ptr<PingPongSphere> BallActor = std::make_unique<PingPongSphere>();
	BallActor->SetPosition(DirectX::XMFLOAT3(0.0f, 0.25f, 2.5f));
	AddActor(std::move(BallActor));

    Game::BeginPlay();
}

void PingPongGame::Update(float DeltaTime)
{
    Game::Update(DeltaTime);
}
