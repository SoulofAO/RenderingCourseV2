#include "PingPongGame.h"
#include "PingPongPlane.h"
#include "PingPongSphere.h"
#include "PingPongUIRenderingComponent.h"
#include "Engine/Core/Runtime/Abstract/Core/Actor.h"
#include "Engine/Core/Runtime/Abstract/Subsystems/InputDevice.h"
#include <algorithm>
#include <cmath>
#include <memory>

PingPongGame::PingPongGame(LPCWSTR ApplicationName, int ScreenWidth, int ScreenHeight)
	: Game(ApplicationName, ScreenWidth, ScreenHeight)
	, LeftPlaneActor(nullptr)
	, RightPlaneActor(nullptr)
	, BallActor(nullptr)
	, UIRenderingComponent(nullptr)
	, BallVelocity(0.0f, 0.0f, 0.0f)
	, ShouldLaunchBallToRight(true)
	, ArenaHalfWidth(2.0f)
	, ArenaHalfHeight(1.0f)
	, ArenaDepth(2.5f)
	, PlaneHalfWidth(0.12f)
	, PlaneHalfHeight(0.65f)
	, BallRadius(0.08f)
	, PlayerPlaneSpeed(2.8f)
	, ComputerPlaneSpeed(2.3f)
	, BallBaseSpeed(2.0f)
	, BounceAccelerationFactor(1.08f)
	, IsBounceAccelerationEnabled(false)
	, PlayerVictoryCount(0)
	, ComputerVictoryCount(0)
{
}

PingPongGame::~PingPongGame() = default;

void PingPongGame::BeginPlay()
{
	std::unique_ptr<PingPongPlane> LeftPlane = std::make_unique<PingPongPlane>();
	LeftPlaneActor = LeftPlane.get();
	
	LeftPlaneActor->SetPosition(DirectX::XMFLOAT3(-ArenaHalfWidth + 0.2f, 0.0f, ArenaDepth));
	AddActor(std::move(LeftPlane));

	std::unique_ptr<PingPongPlane> RightPlane = std::make_unique<PingPongPlane>();
	RightPlaneActor = RightPlane.get();
	RightPlaneActor->SetPosition(DirectX::XMFLOAT3(ArenaHalfWidth - 0.2f, 0.0f, ArenaDepth));
	AddActor(std::move(RightPlane));

	std::unique_ptr<PingPongSphere> Ball = std::make_unique<PingPongSphere>();
	BallActor = Ball.get();
	AddActor(std::move(Ball));

	std::unique_ptr<Actor> UIActor = std::make_unique<Actor>();
	std::unique_ptr<PingPongUIRenderingComponent> NewUIRenderingComponent = std::make_unique<PingPongUIRenderingComponent>();
	UIRenderingComponent = NewUIRenderingComponent.get();
	UIActor->AddComponent(std::move(NewUIRenderingComponent));
	AddActor(std::move(UIActor));

	ResetBall(true);

	Game::BeginPlay();
}

void PingPongGame::Update(float DeltaTime)
{
	HandlePlayerInput(DeltaTime);
	UpdateComputerPlane(DeltaTime);
	UpdateBall(DeltaTime);
	Game::Update(DeltaTime);
}

void PingPongGame::HandlePlayerInput(float DeltaTime)
{
	if (LeftPlaneActor == nullptr)
	{
		return;
	}

	InputDevice* Input = GetInputDevice();
	if (Input == nullptr)
	{
		return;
	}

	float Direction = 0.0f;
	if (Input->IsKeyDown(0x57) || Input->IsKeyDown(VK_UP))
	{
		Direction += 1.0f;
	}

	if (Input->IsKeyDown(0x53) || Input->IsKeyDown(VK_DOWN))
	{
		Direction -= 1.0f;
	}

	DirectX::XMFLOAT3 Position = LeftPlaneActor->GetPosition();
	Position.y += Direction * PlayerPlaneSpeed * DeltaTime;
	LeftPlaneActor->SetPosition(Position);
	ClampPlanePosition(LeftPlaneActor);
}

void PingPongGame::UpdateComputerPlane(float DeltaTime)
{
	if (RightPlaneActor == nullptr || BallActor == nullptr)
	{
		return;
	}

	DirectX::XMFLOAT3 BallPosition = BallActor->GetPosition();
	DirectX::XMFLOAT3 PlanePosition = RightPlaneActor->GetPosition();
	float VerticalDifference = BallPosition.y - PlanePosition.y;

	if (std::fabs(VerticalDifference) > 0.03f)
	{
		float Direction = VerticalDifference > 0.0f ? 1.0f : -1.0f;
		PlanePosition.y += Direction * ComputerPlaneSpeed * DeltaTime;
		RightPlaneActor->SetPosition(PlanePosition);
		ClampPlanePosition(RightPlaneActor);
	}
}

void PingPongGame::UpdateBall(float DeltaTime)
{
	if (BallActor == nullptr)
	{
		return;
	}

	DirectX::XMFLOAT3 BallPosition = BallActor->GetPosition();
	BallPosition.x += BallVelocity.x * DeltaTime;
	BallPosition.y += BallVelocity.y * DeltaTime;
	int BounceEventCount = 0;

	if (BallPosition.y + BallRadius >= ArenaHalfHeight)
	{
		BallPosition.y = ArenaHalfHeight - BallRadius;
		BallVelocity.y = -std::fabs(BallVelocity.y);
		BounceEventCount += 1;
	}
	else if (BallPosition.y - BallRadius <= -ArenaHalfHeight)
	{
		BallPosition.y = -ArenaHalfHeight + BallRadius;
		BallVelocity.y = std::fabs(BallVelocity.y);
		BounceEventCount += 1;
	}

	if (LeftPlaneActor != nullptr && BallVelocity.x < 0.0f && IsBallCollidingWithPlane(BallPosition, LeftPlaneActor))
	{
		const float PlaneCenterY = LeftPlaneActor->GetPosition().y;
		const float HitOffset = (BallPosition.y - PlaneCenterY) / PlaneHalfHeight;
		BallPosition.x = LeftPlaneActor->GetPosition().x + PlaneHalfWidth + BallRadius;
		BallVelocity.x = std::fabs(BallVelocity.x);
		BallVelocity.y += HitOffset * 0.8f;
		BounceEventCount += 1;
	}

	if (RightPlaneActor != nullptr && BallVelocity.x > 0.0f && IsBallCollidingWithPlane(BallPosition, RightPlaneActor))
	{
		const float PlaneCenterY = RightPlaneActor->GetPosition().y;
		const float HitOffset = (BallPosition.y - PlaneCenterY) / PlaneHalfHeight;
		BallPosition.x = RightPlaneActor->GetPosition().x - PlaneHalfWidth - BallRadius;
		BallVelocity.x = -std::fabs(BallVelocity.x);
		BallVelocity.y += HitOffset * 0.8f;
		BounceEventCount += 1;
	}

	if (IsBounceAccelerationEnabled)
	{
		for (int BounceIndex = 0; BounceIndex < BounceEventCount; ++BounceIndex)
		{
			ApplyBounceAcceleration();
		}
	}

	const float CurrentHorizontalSpeed = std::fabs(BallVelocity.x);
	const float MaximumVerticalSpeed = (std::max)(BallBaseSpeed, CurrentHorizontalSpeed) * 1.25f;
	BallVelocity.y = std::clamp(BallVelocity.y, -MaximumVerticalSpeed, MaximumVerticalSpeed);

	if (BallPosition.x + BallRadius < -ArenaHalfWidth)
	{
		ComputerVictoryCount += 1;
		ResetBall(ShouldLaunchBallToRight);
		ShouldLaunchBallToRight = !ShouldLaunchBallToRight;
		return;
	}

	if (BallPosition.x - BallRadius > ArenaHalfWidth)
	{
		PlayerVictoryCount += 1;
		ResetBall(ShouldLaunchBallToRight);
		ShouldLaunchBallToRight = !ShouldLaunchBallToRight;
		return;
	}

	BallActor->SetPosition(BallPosition);
}

void PingPongGame::ClampPlanePosition(Actor* PlaneActor) const
{
	if (PlaneActor == nullptr)
	{
		return;
	}

	DirectX::XMFLOAT3 PlanePosition = PlaneActor->GetPosition();
	PlanePosition.y = std::clamp(PlanePosition.y, -ArenaHalfHeight + PlaneHalfHeight, ArenaHalfHeight - PlaneHalfHeight);
	PlaneActor->SetPosition(PlanePosition);
}

bool PingPongGame::IsBallCollidingWithPlane(const DirectX::XMFLOAT3& BallPosition, const Actor* PlaneActor) const
{
	if (PlaneActor == nullptr)
	{
		return false;
	}

	const DirectX::XMFLOAT3 PlanePosition = PlaneActor->GetPosition();
	const bool IsOverlappingX = std::fabs(BallPosition.x - PlanePosition.x) <= (PlaneHalfWidth + BallRadius);
	const bool IsOverlappingY = std::fabs(BallPosition.y - PlanePosition.y) <= (PlaneHalfHeight + BallRadius);
	return IsOverlappingX && IsOverlappingY;
}

void PingPongGame::ResetBall(bool ShouldMoveRight)
{
	if (BallActor == nullptr)
	{
		return;
	}

	BallActor->SetPosition(DirectX::XMFLOAT3(0.0f, 0.0f, ArenaDepth));
	const float HorizontalDirection = ShouldMoveRight ? 1.0f : -1.0f;
	BallVelocity = DirectX::XMFLOAT3(BallBaseSpeed * HorizontalDirection, BallBaseSpeed * 0.35f, 0.0f);
}

LRESULT PingPongGame::MessageHandler(HWND WindowHandle, UINT Message, WPARAM WParam, LPARAM LParam)
{
	if (UIRenderingComponent != nullptr)
	{
		if (UIRenderingComponent->HandleMessage(WindowHandle, Message, WParam, LParam))
		{
			return 1;
		}
	}

	return Game::MessageHandler(WindowHandle, Message, WParam, LParam);
}

void PingPongGame::ResetBallFromUI()
{
	ResetBall(true);
}

const DirectX::XMFLOAT3& PingPongGame::GetBallVelocity() const
{
	return BallVelocity;
}

int PingPongGame::GetPlayerVictoryCount() const
{
	return PlayerVictoryCount;
}

int PingPongGame::GetComputerVictoryCount() const
{
	return ComputerVictoryCount;
}

bool PingPongGame::GetBounceAccelerationEnabled() const
{
	return IsBounceAccelerationEnabled;
}

void PingPongGame::SetBounceAccelerationEnabledFromUI(bool NewBounceAccelerationEnabled)
{
	if (IsBounceAccelerationEnabled == NewBounceAccelerationEnabled)
	{
		return;
	}

	IsBounceAccelerationEnabled = NewBounceAccelerationEnabled;
	if (IsBounceAccelerationEnabled == false)
	{
		ResetBallSpeedToNormal();
	}
}

void PingPongGame::ApplyBounceAcceleration()
{
	const float CurrentSpeed = std::sqrt((BallVelocity.x * BallVelocity.x) + (BallVelocity.y * BallVelocity.y));
	if (CurrentSpeed <= 0.0f)
	{
		return;
	}

	const float NewSpeed = CurrentSpeed * BounceAccelerationFactor;
	const float SpeedScaleFactor = NewSpeed / CurrentSpeed;
	BallVelocity.x *= SpeedScaleFactor;
	BallVelocity.y *= SpeedScaleFactor;
}

void PingPongGame::ResetBallSpeedToNormal()
{
	const float HorizontalDirection = BallVelocity.x < 0.0f ? -1.0f : 1.0f;
	const float VerticalDirection = BallVelocity.y < 0.0f ? -1.0f : 1.0f;
	BallVelocity = DirectX::XMFLOAT3(BallBaseSpeed * HorizontalDirection, BallBaseSpeed * 0.35f * VerticalDirection, 0.0f);
}

