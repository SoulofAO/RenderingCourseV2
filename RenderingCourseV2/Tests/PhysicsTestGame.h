#pragma once

#include "Abstracts/Core/MulticastDelegate.h"
#include "Tests/TestsBaseGame.h"
#include <directxmath.h>

class PhysicsComponent;

class PhysicsTestGame : public TestsBaseGame
{
public:
	PhysicsTestGame(LPCWSTR ApplicationName, int ScreenWidth, int ScreenHeight);
	~PhysicsTestGame() override;

protected:
	void BuildTestScene() override;
	void Update(float DeltaTime) override;

private:
	void HandlePhysicsCollisionDetected(
		PhysicsComponent* FirstPhysicsComponent,
		PhysicsComponent* SecondPhysicsComponent,
		const DirectX::XMFLOAT3& CollisionNormal,
		float CollisionPenetrationDepth);
	void HandlePhysicsOverlapBegin(
		PhysicsComponent* FirstPhysicsComponent,
		PhysicsComponent* SecondPhysicsComponent);
	void HandlePhysicsOverlapEnd(
		PhysicsComponent* FirstPhysicsComponent,
		PhysicsComponent* SecondPhysicsComponent);
	void SpawnDynamicPhysicsRow();

	PhysicsComponent* PlayerSpherePhysicsComponent;
	DelegateHandle CollisionDelegateHandle;
	DelegateHandle OverlapBeginDelegateHandle;
	DelegateHandle OverlapEndDelegateHandle;
	int CollisionEventCount;
	int OverlapBeginEventCount;
	int OverlapEndEventCount;
};
