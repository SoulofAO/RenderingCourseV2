#include "Abstracts/Subsystems/PhysicsRuntimeGameInstanceSubsystem.h"

PhysicsRuntimeGameInstanceSubsystem::PhysicsRuntimeGameInstanceSubsystem()
	: PhysicsFoundation(nullptr)
	, Physics(nullptr)
	, CpuDispatcher(nullptr)
{
}

PhysicsRuntimeGameInstanceSubsystem::~PhysicsRuntimeGameInstanceSubsystem()
{
	Shutdown();
}

bool PhysicsRuntimeGameInstanceSubsystem::AcquireSharedContext(PhysicsSharedContext& OutSharedContext)
{
	if (PhysicsFoundation == nullptr)
	{
		PhysicsFoundation = PxCreateFoundation(
			PX_PHYSICS_VERSION,
			AllocatorCallback,
			ErrorCallback);
		if (PhysicsFoundation == nullptr)
		{
			return false;
		}
	}

	if (Physics == nullptr)
	{
		const physx::PxTolerancesScale ToleranceScale;
		Physics = PxCreatePhysics(
			PX_PHYSICS_VERSION,
			*PhysicsFoundation,
			ToleranceScale,
			true,
			nullptr);
		if (Physics == nullptr)
		{
			return false;
		}
	}

	if (CpuDispatcher == nullptr)
	{
		CpuDispatcher = physx::PxDefaultCpuDispatcherCreate(2);
		if (CpuDispatcher == nullptr)
		{
			return false;
		}
	}

	OutSharedContext.PhysicsFoundation = PhysicsFoundation;
	OutSharedContext.Physics = Physics;
	OutSharedContext.CpuDispatcher = CpuDispatcher;
	return true;
}

void PhysicsRuntimeGameInstanceSubsystem::Shutdown()
{
	if (CpuDispatcher != nullptr)
	{
		CpuDispatcher->release();
		CpuDispatcher = nullptr;
	}
	if (Physics != nullptr)
	{
		Physics->release();
		Physics = nullptr;
	}
	if (PhysicsFoundation != nullptr)
	{
		PhysicsFoundation->release();
		PhysicsFoundation = nullptr;
	}

	GameInstanceSubsystem::Shutdown();
}
