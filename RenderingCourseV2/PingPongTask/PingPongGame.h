#pragma once
#include "Abstracts/Core/Game.h"
#include <directxmath.h>

class PingPongPlane;
class PingPongSphere;

class PingPongGame : public Game
{
public:
	PingPongGame(LPCWSTR ApplicationName, int ScreenWidth, int ScreenHeight);

protected:
	void BeginPlay() override;
	void Update(float DeltaTime) override;

private:
	void HandlePlayerInput(float DeltaTime);
	void UpdateComputerPlane(float DeltaTime);
	void UpdateBall(float DeltaTime);
	void ClampPlanePosition(Actor* PlaneActor) const;
	bool IsBallCollidingWithPlane(const DirectX::XMFLOAT3& BallPosition, const Actor* PlaneActor) const;
	void ResetBall(bool ShouldMoveRight);

	PingPongPlane* LeftPlaneActor;
	PingPongPlane* RightPlaneActor;
	PingPongSphere* BallActor;
	DirectX::XMFLOAT3 BallVelocity;
	bool ShouldLaunchBallToRight;

	float ArenaHalfWidth;
	float ArenaHalfHeight;
	float ArenaDepth;
	float PlaneHalfWidth;
	float PlaneHalfHeight;
	float BallRadius;
	float PlayerPlaneSpeed;
	float ComputerPlaneSpeed;
	float BallBaseSpeed;
};
