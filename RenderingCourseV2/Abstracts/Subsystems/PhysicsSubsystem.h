#pragma once

#include "Abstracts/Core/MulticastDelegate.h"
#include "Abstracts/Subsystems/Subsystem.h"
#include <directxmath.h>
#include <vector>

class PhysicsComponent;
DECLARE_MULTICAST_DELEGATE_FourParams(
	PhysicsCollisionDetectedDelegate,
	PhysicsComponent*,
	PhysicsComponent*,
	const DirectX::XMFLOAT3&,
	float);

class PhysicsSubsystem : public Subsystem
{
public:
	PhysicsSubsystem();
	~PhysicsSubsystem() override;

	void Update(float DeltaTime) override;

	void RegisterPhysicsComponent(PhysicsComponent* Component);
	void UnregisterPhysicsComponent(PhysicsComponent* Component);

	void SetFixedDeltaTime(float NewFixedDeltaTime);
	float GetFixedDeltaTime() const;
	PhysicsCollisionDetectedDelegate& GetOnCollisionDetectedDelegate();

private:
	struct CollisionManifold
	{
		PhysicsComponent* FirstComponent;
		PhysicsComponent* SecondComponent;
		DirectX::XMFLOAT3 Normal;
		float PenetrationDepth;
	};

	void StepSimulation(float DeltaTime);
	void DetectAndResolveCollisions();
	bool TryBuildCollisionManifold(PhysicsComponent* FirstComponent, PhysicsComponent* SecondComponent, CollisionManifold& OutCollision) const;
	void ResolveCollision(const CollisionManifold& Collision) const;

	std::vector<PhysicsComponent*> PhysicsComponents;
	float FixedDeltaTime;
	float AccumulatedTime;
	PhysicsCollisionDetectedDelegate OnCollisionDetectedDelegate;
};
