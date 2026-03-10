#pragma once
#include "Engine/Core/Runtime/Abstract/Core/Game.h"
#include <directxmath.h>

class PingPongPlane;
class PingPongSphere;
class PingPongUIRenderingComponent;

class PingPongGame : public Game
{
public:
	PingPongGame(LPCWSTR ApplicationName, int ScreenWidth, int ScreenHeight);
	~PingPongGame() override;

	void ResetBallFromUI();
	const DirectX::XMFLOAT3& GetBallVelocity() const;
	int GetPlayerVictoryCount() const;
	int GetComputerVictoryCount() const;
	bool GetBounceAccelerationEnabled() const;
	void SetBounceAccelerationEnabledFromUI(bool NewBounceAccelerationEnabled);

protected:
	void BeginPlay() override;
	void Update(float DeltaTime) override;
	LRESULT MessageHandler(HWND WindowHandle, UINT Message, WPARAM WParam, LPARAM LParam) override;

private:
	void HandlePlayerInput(float DeltaTime);
	void UpdateComputerPlane(float DeltaTime);
	void UpdateBall(float DeltaTime);
	void ClampPlanePosition(Actor* PlaneActor) const;
	bool IsBallCollidingWithPlane(const DirectX::XMFLOAT3& BallPosition, const Actor* PlaneActor) const;
	void ResetBall(bool ShouldMoveRight);
	void ApplyBounceAcceleration();
	void ResetBallSpeedToNormal();

	PingPongPlane* LeftPlaneActor;
	PingPongPlane* RightPlaneActor;
	PingPongSphere* BallActor;
	PingPongUIRenderingComponent* UIRenderingComponent;
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
	float BounceAccelerationFactor;
	bool IsBounceAccelerationEnabled;
	int PlayerVictoryCount;
	int ComputerVictoryCount;
};

