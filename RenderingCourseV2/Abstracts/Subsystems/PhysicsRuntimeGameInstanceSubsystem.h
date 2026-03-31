#pragma once

#include "Abstracts/Subsystems/GameInstanceSubsystem.h"
#include <physx/PxPhysicsAPI.h>

struct PhysicsSharedContext
{
	physx::PxFoundation* PhysicsFoundation;
	physx::PxPhysics* Physics;
	physx::PxDefaultCpuDispatcher* CpuDispatcher;
};

class PhysicsRuntimeGameInstanceSubsystem : public GameInstanceSubsystem
{
public:
	PhysicsRuntimeGameInstanceSubsystem();
	~PhysicsRuntimeGameInstanceSubsystem() override;

	bool AcquireSharedContext(PhysicsSharedContext& OutSharedContext);
	void Shutdown() override;

private:
	physx::PxDefaultAllocator AllocatorCallback;
	physx::PxDefaultErrorCallback ErrorCallback;
	physx::PxFoundation* PhysicsFoundation;
	physx::PxPhysics* Physics;
	physx::PxDefaultCpuDispatcher* CpuDispatcher;
};
