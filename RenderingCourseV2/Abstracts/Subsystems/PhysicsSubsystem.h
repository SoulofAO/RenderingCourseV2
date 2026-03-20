#pragma once

#include "Abstracts/Core/MulticastDelegate.h"
#include "Abstracts/Subsystems/Subsystem.h"
#include <physx/cooking/PxCooking.h>
#include <physx/PxPhysicsAPI.h>
#include <directxmath.h>
#include <string>
#include <unordered_map>
#include <vector>

class PhysicsComponent;
class PhysicsSimulationEventCallback;

DECLARE_MULTICAST_DELEGATE_FourParams(
	PhysicsCollisionDetectedDelegate,
	PhysicsComponent*,
	PhysicsComponent*,
	const DirectX::XMFLOAT3&,
	float);
DECLARE_MULTICAST_DELEGATE_TwoParams(
	PhysicsOverlapDetectedDelegate,
	PhysicsComponent*,
	PhysicsComponent*);

class PhysicsSubsystem : public Subsystem
{
public:
	PhysicsSubsystem();
	~PhysicsSubsystem() override;

	void Initialize() override;
	void Shutdown() override;
	void Update(float DeltaTime) override;

	void RegisterPhysicsComponent(PhysicsComponent* Component);
	void UnregisterPhysicsComponent(PhysicsComponent* Component);

	void RegisterPhysicsActor(PhysicsComponent* Component, physx::PxRigidActor* PhysicsActor);
	void UnregisterPhysicsActor(physx::PxRigidActor* PhysicsActor);
	void RebuildAllPhysicsComponents();

	void SetFixedDeltaTime(float NewFixedDeltaTime);
	float GetFixedDeltaTime() const;

	void SetWorldBoundarySphere(const DirectX::XMFLOAT3& NewBoundaryCenter, float NewBoundaryRadius);
	void DisableWorldBoundarySphere();
	bool GetHasWorldBoundarySphere() const;
	const DirectX::XMFLOAT3& GetWorldBoundaryCenter() const;
	float GetWorldBoundaryRadius() const;

	physx::PxPhysics* GetPhysics() const;
	physx::PxScene* GetScene() const;
	physx::PxMaterial* GetDefaultMaterial() const;
	physx::PxConvexMesh* AcquireConvexMesh(const std::string& ModelMeshPath);
	physx::PxTriangleMesh* AcquireTriangleMesh(const std::string& ModelMeshPath);

	PhysicsCollisionDetectedDelegate& GetOnCollisionDetectedDelegate();
	PhysicsOverlapDetectedDelegate& GetOnOverlapBeginDelegate();
	PhysicsOverlapDetectedDelegate& GetOnOverlapEndDelegate();

	void NotifyContactEvent(
		physx::PxRigidActor* FirstPhysicsActor,
		physx::PxRigidActor* SecondPhysicsActor,
		const physx::PxVec3& ContactNormal,
		float PenetrationDepth);
	void NotifyOverlapBeginEvent(
		physx::PxRigidActor* FirstPhysicsActor,
		physx::PxRigidActor* SecondPhysicsActor);
	void NotifyOverlapEndEvent(
		physx::PxRigidActor* FirstPhysicsActor,
		physx::PxRigidActor* SecondPhysicsActor);

private:
	PhysicsComponent* FindPhysicsComponent(physx::PxRigidActor* PhysicsActor) const;
	void StepSimulation(float DeltaTime);
	void PushStaticTransformsToPhysics();
	void PullDynamicTransformsFromPhysics();
	void EnforceWorldBoundarySphere();
	void InitializePhysXContext();
	void ShutdownPhysXContext();
	void ReleaseCachedConvexMeshes();
	void ReleaseCachedTriangleMeshes();

	std::vector<PhysicsComponent*> PhysicsComponents;
	std::unordered_map<physx::PxRigidActor*, PhysicsComponent*> PhysicsActorToComponentMap;
	std::unordered_map<std::string, physx::PxConvexMesh*> ConvexMeshCache;
	std::unordered_map<std::string, physx::PxTriangleMesh*> TriangleMeshCache;

	float FixedDeltaTime;
	float AccumulatedTime;

	bool HasWorldBoundarySphere;
	DirectX::XMFLOAT3 WorldBoundaryCenter;
	float WorldBoundaryRadius;

	physx::PxDefaultAllocator AllocatorCallback;
	physx::PxDefaultErrorCallback ErrorCallback;
	physx::PxFoundation* PhysicsFoundation;
	physx::PxPhysics* Physics;
	physx::PxScene* PhysicsScene;
	physx::PxDefaultCpuDispatcher* CpuDispatcher;
	physx::PxMaterial* DefaultMaterial;
	PhysicsSimulationEventCallback* SimulationEventCallback;

	PhysicsCollisionDetectedDelegate OnCollisionDetectedDelegate;
	PhysicsOverlapDetectedDelegate OnOverlapBeginDelegate;
	PhysicsOverlapDetectedDelegate OnOverlapEndDelegate;
};
