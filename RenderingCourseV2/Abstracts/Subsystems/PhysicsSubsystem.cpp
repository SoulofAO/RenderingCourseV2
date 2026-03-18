#include "Abstracts/Subsystems/PhysicsSubsystem.h"
#include "Abstracts/Components/PhysicsComponent.h"
#include "Abstracts/Core/Game.h"
#include "Abstracts/Physics/PhysXTypeConversion.h"
#include "Abstracts/Resources/ModelResource.h"
#include "Abstracts/Resources/ResourceManager.h"
#include <algorithm>
#include <cmath>
#include <vector>

class PhysicsSimulationEventCallback : public physx::PxSimulationEventCallback
{
public:
	explicit PhysicsSimulationEventCallback(PhysicsSubsystem* NewPhysicsSubsystem)
		: PhysicsSubsystemInstance(NewPhysicsSubsystem)
	{
	}

	void onConstraintBreak(physx::PxConstraintInfo* Constraints, physx::PxU32 Count) override
	{
		(void)Constraints;
		(void)Count;
	}

	void onWake(physx::PxActor** Actors, physx::PxU32 Count) override
	{
		(void)Actors;
		(void)Count;
	}

	void onSleep(physx::PxActor** Actors, physx::PxU32 Count) override
	{
		(void)Actors;
		(void)Count;
	}

	void onAdvance(const physx::PxRigidBody* const* BodyBuffer, const physx::PxTransform* PoseBuffer, const physx::PxU32 Count) override
	{
		(void)BodyBuffer;
		(void)PoseBuffer;
		(void)Count;
	}

	void onTrigger(physx::PxTriggerPair* TriggerPairs, physx::PxU32 PairCount) override
	{
		if (PhysicsSubsystemInstance == nullptr || TriggerPairs == nullptr)
		{
			return;
		}

		for (physx::PxU32 PairIndex = 0; PairIndex < PairCount; ++PairIndex)
		{
			const physx::PxTriggerPair& TriggerPair = TriggerPairs[PairIndex];
			if ((TriggerPair.flags & physx::PxTriggerPairFlag::eREMOVED_SHAPE_TRIGGER) != 0 ||
				(TriggerPair.flags & physx::PxTriggerPairFlag::eREMOVED_SHAPE_OTHER) != 0)
			{
				continue;
			}

			physx::PxRigidActor* TriggerActor = static_cast<physx::PxRigidActor*>(TriggerPair.triggerActor);
			physx::PxRigidActor* OtherActor = static_cast<physx::PxRigidActor*>(TriggerPair.otherActor);
			if ((TriggerPair.status & physx::PxPairFlag::eNOTIFY_TOUCH_FOUND) != 0)
			{
				PhysicsSubsystemInstance->NotifyOverlapBeginEvent(TriggerActor, OtherActor);
			}
			if ((TriggerPair.status & physx::PxPairFlag::eNOTIFY_TOUCH_LOST) != 0)
			{
				PhysicsSubsystemInstance->NotifyOverlapEndEvent(TriggerActor, OtherActor);
			}
		}
	}

	void onContact(const physx::PxContactPairHeader& PairHeader, const physx::PxContactPair* ContactPairs, physx::PxU32 PairCount) override
	{
		if (PhysicsSubsystemInstance == nullptr || ContactPairs == nullptr)
		{
			return;
		}

		physx::PxRigidActor* FirstActor = static_cast<physx::PxRigidActor*>(PairHeader.actors[0]);
		physx::PxRigidActor* SecondActor = static_cast<physx::PxRigidActor*>(PairHeader.actors[1]);
		if (FirstActor == nullptr || SecondActor == nullptr)
		{
			return;
		}

		for (physx::PxU32 PairIndex = 0; PairIndex < PairCount; ++PairIndex)
		{
			const physx::PxContactPair& ContactPair = ContactPairs[PairIndex];
			if (
				ContactPair.events.isSet(physx::PxPairFlag::eNOTIFY_TOUCH_FOUND) == false &&
				ContactPair.events.isSet(physx::PxPairFlag::eNOTIFY_TOUCH_PERSISTS) == false)
			{
				continue;
			}

			physx::PxContactPairPoint ContactPoints[8];
			const physx::PxU32 ContactPointCount = ContactPair.extractContacts(ContactPoints, 8);
			if (ContactPointCount == 0)
			{
				continue;
			}

			const physx::PxVec3 ContactNormal = ContactPoints[0].normal;
			const float PenetrationDepth = (std::max)(0.0f, -ContactPoints[0].separation);
			PhysicsSubsystemInstance->NotifyContactEvent(
				FirstActor,
				SecondActor,
				ContactNormal,
				PenetrationDepth);
		}
	}

private:
	PhysicsSubsystem* PhysicsSubsystemInstance;
};

PhysicsSubsystem::PhysicsSubsystem()
	: FixedDeltaTime(1.0f / 60.0f)
	, AccumulatedTime(0.0f)
	, HasWorldBoundarySphere(false)
	, WorldBoundaryCenter(0.0f, 0.0f, 0.0f)
	, WorldBoundaryRadius(100.0f)
	, PhysicsFoundation(nullptr)
	, Physics(nullptr)
	, PhysicsScene(nullptr)
	, CpuDispatcher(nullptr)
	, DefaultMaterial(nullptr)
	, SimulationEventCallback(nullptr)
{
}

PhysicsSubsystem::~PhysicsSubsystem()
{
	Shutdown();
}

void PhysicsSubsystem::Initialize()
{
	if (GetIsInitialized())
	{
		return;
	}

	Subsystem::Initialize();
	InitializePhysXContext();
	for (PhysicsComponent* ExistingComponent : PhysicsComponents)
	{
		if (ExistingComponent != nullptr)
		{
			ExistingComponent->CreatePhysicsActor(this);
		}
	}
}

void PhysicsSubsystem::Shutdown()
{
	if (GetIsInitialized() == false)
	{
		return;
	}

	for (PhysicsComponent* ExistingComponent : PhysicsComponents)
	{
		if (ExistingComponent != nullptr)
		{
			ExistingComponent->DestroyPhysicsActor();
		}
	}
	PhysicsActorToComponentMap.clear();
	ReleaseCachedConvexMeshes();
	ShutdownPhysXContext();
	AccumulatedTime = 0.0f;
	Subsystem::Shutdown();
}

void PhysicsSubsystem::Update(float DeltaTime)
{
	if (PhysicsScene == nullptr || Physics == nullptr)
	{
		return;
	}

	AccumulatedTime += DeltaTime;
	while (AccumulatedTime >= FixedDeltaTime)
	{
		StepSimulation(FixedDeltaTime);
		AccumulatedTime -= FixedDeltaTime;
	}
}

void PhysicsSubsystem::RegisterPhysicsComponent(PhysicsComponent* Component)
{
	if (Component == nullptr)
	{
		return;
	}

	const auto ExistingPosition = std::find(PhysicsComponents.begin(), PhysicsComponents.end(), Component);
	if (ExistingPosition != PhysicsComponents.end())
	{
		return;
	}

	PhysicsComponents.push_back(Component);
	if (GetIsInitialized())
	{
		Component->CreatePhysicsActor(this);
	}
}

void PhysicsSubsystem::UnregisterPhysicsComponent(PhysicsComponent* Component)
{
	if (Component == nullptr)
	{
		return;
	}

	Component->DestroyPhysicsActor();
	PhysicsComponents.erase(
		std::remove(PhysicsComponents.begin(), PhysicsComponents.end(), Component),
		PhysicsComponents.end());
}

void PhysicsSubsystem::RegisterPhysicsActor(PhysicsComponent* Component, physx::PxRigidActor* PhysicsActor)
{
	if (Component == nullptr || PhysicsActor == nullptr)
	{
		return;
	}

	PhysicsActorToComponentMap[PhysicsActor] = Component;
}

void PhysicsSubsystem::UnregisterPhysicsActor(physx::PxRigidActor* PhysicsActor)
{
	if (PhysicsActor == nullptr)
	{
		return;
	}

	PhysicsActorToComponentMap.erase(PhysicsActor);
}

void PhysicsSubsystem::SetFixedDeltaTime(float NewFixedDeltaTime)
{
	if (NewFixedDeltaTime > 0.0f)
	{
		FixedDeltaTime = NewFixedDeltaTime;
	}
}

float PhysicsSubsystem::GetFixedDeltaTime() const
{
	return FixedDeltaTime;
}

void PhysicsSubsystem::SetWorldBoundarySphere(const DirectX::XMFLOAT3& NewBoundaryCenter, float NewBoundaryRadius)
{
	if (NewBoundaryRadius <= 0.0f)
	{
		return;
	}

	HasWorldBoundarySphere = true;
	WorldBoundaryCenter = NewBoundaryCenter;
	WorldBoundaryRadius = NewBoundaryRadius;
}

void PhysicsSubsystem::DisableWorldBoundarySphere()
{
	HasWorldBoundarySphere = false;
}

bool PhysicsSubsystem::GetHasWorldBoundarySphere() const
{
	return HasWorldBoundarySphere;
}

const DirectX::XMFLOAT3& PhysicsSubsystem::GetWorldBoundaryCenter() const
{
	return WorldBoundaryCenter;
}

float PhysicsSubsystem::GetWorldBoundaryRadius() const
{
	return WorldBoundaryRadius;
}

physx::PxPhysics* PhysicsSubsystem::GetPhysics() const
{
	return Physics;
}

physx::PxScene* PhysicsSubsystem::GetScene() const
{
	return PhysicsScene;
}

physx::PxMaterial* PhysicsSubsystem::GetDefaultMaterial() const
{
	return DefaultMaterial;
}

physx::PxConvexMesh* PhysicsSubsystem::AcquireConvexMesh(const std::string& ModelMeshPath)
{
	if (ModelMeshPath.empty() || Physics == nullptr)
	{
		return nullptr;
	}

	const auto CachedPosition = ConvexMeshCache.find(ModelMeshPath);
	if (CachedPosition != ConvexMeshCache.end())
	{
		return CachedPosition->second;
	}

	Game* OwningGameInstance = GetOwningGame();
	if (OwningGameInstance == nullptr)
	{
		return nullptr;
	}

	ResourceManager* ResourceManagerInstance = OwningGameInstance->GetResourceManager();
	if (ResourceManagerInstance == nullptr)
	{
		return nullptr;
	}

	const std::shared_ptr<ModelResource> ModelResourceInstance = ResourceManagerInstance->LoadModelResource(ModelMeshPath);
	if (ModelResourceInstance == nullptr || ModelResourceInstance->Vertices.size() < 4)
	{
		return nullptr;
	}

	std::vector<physx::PxVec3> ConvexPoints;
	ConvexPoints.reserve(ModelResourceInstance->Vertices.size());
	for (const ModelResourceVertex& ExistingVertex : ModelResourceInstance->Vertices)
	{
		ConvexPoints.push_back(physx::PxVec3(
			ExistingVertex.Position.x,
			ExistingVertex.Position.y,
			ExistingVertex.Position.z));
	}

	physx::PxConvexMeshDesc ConvexDescription = {};
	ConvexDescription.points.count = static_cast<physx::PxU32>(ConvexPoints.size());
	ConvexDescription.points.stride = sizeof(physx::PxVec3);
	ConvexDescription.points.data = ConvexPoints.data();
	ConvexDescription.flags = physx::PxConvexFlag::eCOMPUTE_CONVEX | physx::PxConvexFlag::eSHIFT_VERTICES;

	physx::PxConvexMeshCookingResult::Enum CookingResult = physx::PxConvexMeshCookingResult::eFAILURE;
	physx::PxCookingParams CookingParameters(Physics->getTolerancesScale());
	physx::PxConvexMesh* NewConvexMesh = PxCreateConvexMesh(
		CookingParameters,
		ConvexDescription,
		Physics->getPhysicsInsertionCallback(),
		&CookingResult);
	if (NewConvexMesh == nullptr)
	{
		return nullptr;
	}

	ConvexMeshCache.insert({ ModelMeshPath, NewConvexMesh });
	return NewConvexMesh;
}

PhysicsCollisionDetectedDelegate& PhysicsSubsystem::GetOnCollisionDetectedDelegate()
{
	return OnCollisionDetectedDelegate;
}

PhysicsOverlapDetectedDelegate& PhysicsSubsystem::GetOnOverlapBeginDelegate()
{
	return OnOverlapBeginDelegate;
}

PhysicsOverlapDetectedDelegate& PhysicsSubsystem::GetOnOverlapEndDelegate()
{
	return OnOverlapEndDelegate;
}

void PhysicsSubsystem::NotifyContactEvent(
	physx::PxRigidActor* FirstPhysicsActor,
	physx::PxRigidActor* SecondPhysicsActor,
	const physx::PxVec3& ContactNormal,
	float PenetrationDepth)
{
	PhysicsComponent* FirstComponent = FindPhysicsComponent(FirstPhysicsActor);
	PhysicsComponent* SecondComponent = FindPhysicsComponent(SecondPhysicsActor);
	if (FirstComponent == nullptr || SecondComponent == nullptr)
	{
		return;
	}

	const DirectX::XMFLOAT3 ContactNormalDirectX = PhysXTypeConversion::ToDirectXVector(ContactNormal);
	OnCollisionDetectedDelegate.Broadcast(
		FirstComponent,
		SecondComponent,
		ContactNormalDirectX,
		PenetrationDepth);
}

void PhysicsSubsystem::NotifyOverlapBeginEvent(
	physx::PxRigidActor* FirstPhysicsActor,
	physx::PxRigidActor* SecondPhysicsActor)
{
	PhysicsComponent* FirstComponent = FindPhysicsComponent(FirstPhysicsActor);
	PhysicsComponent* SecondComponent = FindPhysicsComponent(SecondPhysicsActor);
	if (FirstComponent == nullptr || SecondComponent == nullptr)
	{
		return;
	}

	OnOverlapBeginDelegate.Broadcast(FirstComponent, SecondComponent);
}

void PhysicsSubsystem::NotifyOverlapEndEvent(
	physx::PxRigidActor* FirstPhysicsActor,
	physx::PxRigidActor* SecondPhysicsActor)
{
	PhysicsComponent* FirstComponent = FindPhysicsComponent(FirstPhysicsActor);
	PhysicsComponent* SecondComponent = FindPhysicsComponent(SecondPhysicsActor);
	if (FirstComponent == nullptr || SecondComponent == nullptr)
	{
		return;
	}

	OnOverlapEndDelegate.Broadcast(FirstComponent, SecondComponent);
}

PhysicsComponent* PhysicsSubsystem::FindPhysicsComponent(physx::PxRigidActor* PhysicsActor) const
{
	if (PhysicsActor == nullptr)
	{
		return nullptr;
	}

	const auto ExistingPosition = PhysicsActorToComponentMap.find(PhysicsActor);
	if (ExistingPosition == PhysicsActorToComponentMap.end())
	{
		return nullptr;
	}
	return ExistingPosition->second;
}

void PhysicsSubsystem::StepSimulation(float DeltaTime)
{
	PushStaticTransformsToPhysics();
	PhysicsScene->simulate(DeltaTime);
	PhysicsScene->fetchResults(true);
	EnforceWorldBoundarySphere();
	PullDynamicTransformsFromPhysics();
}

void PhysicsSubsystem::PushStaticTransformsToPhysics()
{
	for (PhysicsComponent* ExistingComponent : PhysicsComponents)
	{
		if (ExistingComponent != nullptr)
		{
			ExistingComponent->SyncPhysicsTransformFromActor();
		}
	}
}

void PhysicsSubsystem::PullDynamicTransformsFromPhysics()
{
	for (PhysicsComponent* ExistingComponent : PhysicsComponents)
	{
		if (ExistingComponent != nullptr)
		{
			ExistingComponent->SyncActorTransformFromPhysics();
		}
	}
}

void PhysicsSubsystem::EnforceWorldBoundarySphere()
{
	if (HasWorldBoundarySphere == false)
	{
		return;
	}

	for (PhysicsComponent* ExistingComponent : PhysicsComponents)
	{
		if (ExistingComponent != nullptr)
		{
			ExistingComponent->ApplyBoundarySphereConstraint(WorldBoundaryCenter, WorldBoundaryRadius);
		}
	}
}

void PhysicsSubsystem::InitializePhysXContext()
{
	PhysicsFoundation = PxCreateFoundation(
		PX_PHYSICS_VERSION,
		AllocatorCallback,
		ErrorCallback);
	if (PhysicsFoundation == nullptr)
	{
		return;
	}

	const physx::PxTolerancesScale ToleranceScale;
	Physics = PxCreatePhysics(
		PX_PHYSICS_VERSION,
		*PhysicsFoundation,
		ToleranceScale,
		true,
		nullptr);
	if (Physics == nullptr)
	{
		return;
	}

	CpuDispatcher = physx::PxDefaultCpuDispatcherCreate(2);
	if (CpuDispatcher == nullptr)
	{
		return;
	}

	SimulationEventCallback = new PhysicsSimulationEventCallback(this);

	physx::PxSceneDesc SceneDescription(ToleranceScale);
	SceneDescription.gravity = physx::PxVec3(0.0f, -9.81f, 0.0f);
	SceneDescription.cpuDispatcher = CpuDispatcher;
	SceneDescription.filterShader = physx::PxDefaultSimulationFilterShader;
	SceneDescription.simulationEventCallback = SimulationEventCallback;
	PhysicsScene = Physics->createScene(SceneDescription);
	if (PhysicsScene == nullptr)
	{
		return;
	}

	DefaultMaterial = Physics->createMaterial(0.5f, 0.5f, 0.5f);
}

void PhysicsSubsystem::ShutdownPhysXContext()
{
	if (DefaultMaterial != nullptr)
	{
		DefaultMaterial->release();
		DefaultMaterial = nullptr;
	}
	if (PhysicsScene != nullptr)
	{
		PhysicsScene->release();
		PhysicsScene = nullptr;
	}
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
	if (SimulationEventCallback != nullptr)
	{
		delete SimulationEventCallback;
		SimulationEventCallback = nullptr;
	}
}

void PhysicsSubsystem::ReleaseCachedConvexMeshes()
{
	for (auto& ExistingPair : ConvexMeshCache)
	{
		if (ExistingPair.second != nullptr)
		{
			ExistingPair.second->release();
			ExistingPair.second = nullptr;
		}
	}
	ConvexMeshCache.clear();
}
