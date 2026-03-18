#pragma once

#include <directxmath.h>
#include <physx/PxPhysicsAPI.h>

struct PhysicsLineTraceHitResult
{
	bool HasBlockingHit;
	DirectX::XMFLOAT3 HitLocation;
	DirectX::XMFLOAT3 HitNormal;
	float HitDistance;
	physx::PxRigidActor* HitActor;
	physx::PxShape* HitShape;
};

class PhysicsLibrary
{
public:
	static bool BuildLineTraceFromMousePosition(
		int MousePositionX,
		int MousePositionY,
		DirectX::XMFLOAT3& TraceStart,
		DirectX::XMFLOAT3& TraceDirection);

	static bool LineTrace(
		const DirectX::XMFLOAT3& TraceStart,
		const DirectX::XMFLOAT3& TraceDirection,
		float TraceLength,
		PhysicsLineTraceHitResult& HitResult);
};
