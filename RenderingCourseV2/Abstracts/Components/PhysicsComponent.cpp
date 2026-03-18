#include "Abstracts/Components/PhysicsComponent.h"
#include "Abstracts/Components/MeshUniversalComponent.h"
#include "Abstracts/Core/Actor.h"
#include "Abstracts/Core/Game.h"
#include "Abstracts/Physics/PhysXTypeConversion.h"
#include "Abstracts/Subsystems/PhysicsSubsystem.h"
#include <physx/extensions/PxFixedJoint.h>
#include <physx/extensions/PxRigidBodyExt.h>
#include <algorithm>
#include <cmath>

PhysicsComponent::PhysicsComponent()
	: Mass(1.0f)
	, InverseMass(1.0f)
	, Restitution(0.5f)
	, LinearDamping(0.05f)
	, AngularDamping(0.1f)
	, UseGravity(true)
	, IsStatic(false)
	, ColliderKind(PhysicsColliderKind::Sphere)
	, CollisionMode(PhysicsCollisionMode::Simulation)
	, SphereRadius(0.5f)
	, HalfExtents(0.5f, 0.5f, 0.5f)
	, CachedVelocity(0.0f, 0.0f, 0.0f)
	, CachedAngularVelocity(0.0f, 0.0f, 0.0f)
	, PhysicsSubsystemInstance(nullptr)
	, PhysicsMaterial(nullptr)
	, PhysicsActor(nullptr)
	, PhysicsDynamicActor(nullptr)
{
}

PhysicsComponent::~PhysicsComponent()
{
	Shutdown();
}

void PhysicsComponent::Initialize()
{
	if (GetIsInitialized())
	{
		return;
	}

	ActorComponent::Initialize();
	Game* GameInstance = GetOwningGame();
	if (GameInstance == nullptr)
	{
		return;
	}

	PhysicsSubsystem* PhysicsSubsystemValue = GameInstance->GetSubsystem<PhysicsSubsystem>();
	if (PhysicsSubsystemValue == nullptr)
	{
		return;
	}

	PhysicsSubsystemValue->RegisterPhysicsComponent(this);
}

void PhysicsComponent::Shutdown()
{
	if (GetIsInitialized() == false)
	{
		return;
	}

	Game* GameInstance = GetOwningGame();
	if (GameInstance != nullptr)
	{
		PhysicsSubsystem* PhysicsSubsystemValue = GameInstance->GetSubsystem<PhysicsSubsystem>();
		if (PhysicsSubsystemValue != nullptr)
		{
			PhysicsSubsystemValue->UnregisterPhysicsComponent(this);
		}
	}

	DestroyPhysicsActor();
	ActorComponent::Shutdown();
}

void PhysicsComponent::SetMass(float NewMass)
{
	Mass = (std::max)(0.0f, NewMass);
	RecalculateInverseMass();
	if (PhysicsDynamicActor != nullptr && Mass > 0.0f)
	{
		physx::PxRigidBodyExt::updateMassAndInertia(*PhysicsDynamicActor, Mass);
	}
}

float PhysicsComponent::GetMass() const
{
	return Mass;
}

float PhysicsComponent::GetInverseMass() const
{
	return InverseMass;
}

void PhysicsComponent::SetRestitution(float NewRestitution)
{
	Restitution = std::clamp(NewRestitution, 0.0f, 1.0f);
	if (PhysicsMaterial != nullptr)
	{
		PhysicsMaterial->setRestitution(Restitution);
	}
}

float PhysicsComponent::GetRestitution() const
{
	return Restitution;
}

void PhysicsComponent::SetLinearDamping(float NewLinearDamping)
{
	LinearDamping = (std::max)(0.0f, NewLinearDamping);
	if (PhysicsDynamicActor != nullptr)
	{
		PhysicsDynamicActor->setLinearDamping(LinearDamping);
	}
}

float PhysicsComponent::GetLinearDamping() const
{
	return LinearDamping;
}

void PhysicsComponent::SetAngularDamping(float NewAngularDamping)
{
	AngularDamping = (std::max)(0.0f, NewAngularDamping);
	if (PhysicsDynamicActor != nullptr)
	{
		PhysicsDynamicActor->setAngularDamping(AngularDamping);
	}
}

float PhysicsComponent::GetAngularDamping() const
{
	return AngularDamping;
}

void PhysicsComponent::SetUseGravity(bool NewUseGravity)
{
	UseGravity = NewUseGravity;
	if (PhysicsDynamicActor != nullptr)
	{
		PhysicsDynamicActor->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY, UseGravity == false);
	}
}

bool PhysicsComponent::GetUseGravity() const
{
	return UseGravity;
}

void PhysicsComponent::SetVelocity(const DirectX::XMFLOAT3& NewVelocity)
{
	CachedVelocity = NewVelocity;
	if (PhysicsDynamicActor != nullptr)
	{
		PhysicsDynamicActor->setLinearVelocity(PhysXTypeConversion::ToPxVector(NewVelocity));
	}
}

DirectX::XMFLOAT3 PhysicsComponent::GetVelocity() const
{
	if (PhysicsDynamicActor != nullptr)
	{
		return PhysXTypeConversion::ToDirectXVector(PhysicsDynamicActor->getLinearVelocity());
	}
	return CachedVelocity;
}

void PhysicsComponent::SetAngularVelocity(const DirectX::XMFLOAT3& NewAngularVelocity)
{
	CachedAngularVelocity = NewAngularVelocity;
	if (PhysicsDynamicActor != nullptr)
	{
		PhysicsDynamicActor->setAngularVelocity(PhysXTypeConversion::ToPxVector(NewAngularVelocity));
	}
}

DirectX::XMFLOAT3 PhysicsComponent::GetAngularVelocity() const
{
	if (PhysicsDynamicActor != nullptr)
	{
		return PhysXTypeConversion::ToDirectXVector(PhysicsDynamicActor->getAngularVelocity());
	}
	return CachedAngularVelocity;
}

void PhysicsComponent::AddForce(const DirectX::XMFLOAT3& ForceValue)
{
	if (PhysicsDynamicActor == nullptr)
	{
		return;
	}

	PhysicsDynamicActor->addForce(
		PhysXTypeConversion::ToPxVector(ForceValue),
		physx::PxForceMode::eFORCE,
		true);
}

void PhysicsComponent::AddTorque(const DirectX::XMFLOAT3& TorqueValue)
{
	if (PhysicsDynamicActor == nullptr)
	{
		return;
	}

	PhysicsDynamicActor->addTorque(
		PhysXTypeConversion::ToPxVector(TorqueValue),
		physx::PxForceMode::eFORCE,
		true);
}

void PhysicsComponent::ClearForces()
{
	if (PhysicsDynamicActor == nullptr)
	{
		return;
	}

	PhysicsDynamicActor->clearForce(physx::PxForceMode::eFORCE);
	PhysicsDynamicActor->clearTorque(physx::PxForceMode::eFORCE);
}

void PhysicsComponent::SetIsStatic(bool NewIsStatic)
{
	IsStatic = NewIsStatic;
	RecalculateInverseMass();
	RebuildPhysicsActor();
}

bool PhysicsComponent::GetIsStatic() const
{
	return IsStatic;
}

void PhysicsComponent::SetSphereCollider(float NewRadius)
{
	ColliderKind = PhysicsColliderKind::Sphere;
	SphereRadius = (std::max)(0.01f, NewRadius);
	RebuildPhysicsActor();
}

void PhysicsComponent::SetAabbCollider(const DirectX::XMFLOAT3& NewHalfExtents)
{
	ColliderKind = PhysicsColliderKind::Box;
	HalfExtents.x = (std::max)(0.01f, NewHalfExtents.x);
	HalfExtents.y = (std::max)(0.01f, NewHalfExtents.y);
	HalfExtents.z = (std::max)(0.01f, NewHalfExtents.z);
	RebuildPhysicsActor();
}

void PhysicsComponent::EnableAutoConvexColliderFromMesh(bool NewAutoConvexEnabled)
{
	ColliderKind = NewAutoConvexEnabled ? PhysicsColliderKind::ConvexMeshAuto : PhysicsColliderKind::Sphere;
	RebuildPhysicsActor();
}

bool PhysicsComponent::GetAutoConvexColliderFromMeshEnabled() const
{
	return ColliderKind == PhysicsColliderKind::ConvexMeshAuto;
}

bool PhysicsComponent::GetUsesSphereCollider() const
{
	return ColliderKind == PhysicsColliderKind::Sphere;
}

const DirectX::XMFLOAT3& PhysicsComponent::GetHalfExtents() const
{
	return HalfExtents;
}

float PhysicsComponent::GetSphereRadius() const
{
	return SphereRadius;
}

void PhysicsComponent::SetCollisionMode(PhysicsCollisionMode NewCollisionMode)
{
	CollisionMode = NewCollisionMode;
	ApplyCollisionModeToAllShapes();
}

PhysicsCollisionMode PhysicsComponent::GetCollisionMode() const
{
	return CollisionMode;
}

void PhysicsComponent::Integrate(float DeltaTime)
{
	(void)DeltaTime;
}

void PhysicsComponent::ApplyImpulse(const DirectX::XMFLOAT3& ImpulseValue)
{
	if (PhysicsDynamicActor == nullptr)
	{
		return;
	}

	PhysicsDynamicActor->addForce(
		PhysXTypeConversion::ToPxVector(ImpulseValue),
		physx::PxForceMode::eIMPULSE,
		true);
}

void PhysicsComponent::ApplyPositionCorrection(const DirectX::XMFLOAT3& CorrectionValue)
{
	if (PhysicsActor == nullptr)
	{
		return;
	}

	physx::PxTransform PhysicsTransform = PhysicsActor->getGlobalPose();
	PhysicsTransform.p += PhysXTypeConversion::ToPxVector(CorrectionValue);
	PhysicsActor->setGlobalPose(PhysicsTransform, true);
	SyncActorTransformFromPhysics();
}

bool PhysicsComponent::WeldWithComponent(PhysicsComponent* TargetComponent, bool PreserveRelativePose)
{
	if (TargetComponent == nullptr || PhysicsDynamicActor == nullptr || TargetComponent->PhysicsDynamicActor == nullptr)
	{
		return false;
	}

	if (PhysicsSubsystemInstance == nullptr || PhysicsSubsystemInstance != TargetComponent->PhysicsSubsystemInstance)
	{
		return false;
	}

	physx::PxTransform FirstLocalPose(physx::PxIdentity);
	physx::PxTransform SecondLocalPose(physx::PxIdentity);
	if (PreserveRelativePose)
	{
		const physx::PxTransform FirstGlobalPose = PhysicsDynamicActor->getGlobalPose();
		const physx::PxTransform SecondGlobalPose = TargetComponent->PhysicsDynamicActor->getGlobalPose();
		SecondLocalPose = FirstGlobalPose.transformInv(SecondGlobalPose);
	}

	physx::PxFixedJoint* NewJoint = physx::PxFixedJointCreate(
		*PhysicsSubsystemInstance->GetPhysics(),
		PhysicsDynamicActor,
		FirstLocalPose,
		TargetComponent->PhysicsDynamicActor,
		SecondLocalPose);
	if (NewJoint == nullptr)
	{
		return false;
	}

	WeldJoints.push_back(NewJoint);
	return true;
}

void PhysicsComponent::UnweldAllComponents()
{
	ReleaseAllWeldJoints();
}

void PhysicsComponent::CreatePhysicsActor(PhysicsSubsystem* NewPhysicsSubsystem)
{
	if (NewPhysicsSubsystem == nullptr)
	{
		return;
	}

	DestroyPhysicsActor();
	PhysicsSubsystemInstance = NewPhysicsSubsystem;

	physx::PxPhysics* PhysicsInstance = PhysicsSubsystemInstance->GetPhysics();
	physx::PxScene* PhysicsSceneInstance = PhysicsSubsystemInstance->GetScene();
	if (PhysicsInstance == nullptr || PhysicsSceneInstance == nullptr)
	{
		return;
	}

	Actor* OwningActorInstance = GetOwningActor();
	if (OwningActorInstance == nullptr)
	{
		return;
	}

	const Transform ActorWorldTransform = OwningActorInstance->GetTransform();
	const physx::PxTransform ActorPhysicsTransform = PhysXTypeConversion::ToPxTransform(ActorWorldTransform);
	if (IsStatic)
	{
		PhysicsActor = PhysicsInstance->createRigidStatic(ActorPhysicsTransform);
		PhysicsDynamicActor = nullptr;
	}
	else
	{
		PhysicsDynamicActor = PhysicsInstance->createRigidDynamic(ActorPhysicsTransform);
		PhysicsActor = PhysicsDynamicActor;
	}

	if (PhysicsActor == nullptr)
	{
		return;
	}

	PhysicsMaterial = PhysicsInstance->createMaterial(0.5f, 0.5f, Restitution);
	if (PhysicsMaterial == nullptr)
	{
		DestroyPhysicsActor();
		return;
	}

	physx::PxShape* NewShape = nullptr;
	if (ColliderKind == PhysicsColliderKind::ConvexMeshAuto)
	{
		MeshUniversalComponent* MeshComponent = nullptr;
		const std::vector<std::unique_ptr<ActorComponent>>& ActorComponents = OwningActorInstance->GetComponents();
		for (const std::unique_ptr<ActorComponent>& ExistingComponent : ActorComponents)
		{
			MeshUniversalComponent* MeshCandidate = dynamic_cast<MeshUniversalComponent*>(ExistingComponent.get());
			if (MeshCandidate != nullptr)
			{
				MeshComponent = MeshCandidate;
				break;
			}
		}

		if (MeshComponent != nullptr && MeshComponent->ModelMeshPath.empty() == false)
		{
			physx::PxConvexMesh* ConvexMeshValue = PhysicsSubsystemInstance->AcquireConvexMesh(MeshComponent->ModelMeshPath);
			if (ConvexMeshValue != nullptr)
			{
				const physx::PxMeshScale ConvexScale(
					PhysXTypeConversion::ToPxVector(ActorWorldTransform.Scale),
					physx::PxQuat(physx::PxIdentity));
				const physx::PxConvexMeshGeometry GeometryValue(ConvexMeshValue, ConvexScale);
				NewShape = PhysicsInstance->createShape(GeometryValue, *PhysicsMaterial, true);
			}
		}
	}

	if (NewShape == nullptr && ColliderKind == PhysicsColliderKind::Box)
	{
		const DirectX::XMFLOAT3 ScaleValue = ActorWorldTransform.Scale;
		const DirectX::XMFLOAT3 GeometryHalfExtents(
			HalfExtents.x * std::fabs(ScaleValue.x),
			HalfExtents.y * std::fabs(ScaleValue.y),
			HalfExtents.z * std::fabs(ScaleValue.z));
		const physx::PxBoxGeometry GeometryValue(
			GeometryHalfExtents.x,
			GeometryHalfExtents.y,
			GeometryHalfExtents.z);
		NewShape = PhysicsInstance->createShape(GeometryValue, *PhysicsMaterial, true);
	}

	if (NewShape == nullptr)
	{
		const float MaximumScaleAxis = (std::max)(
			(std::max)(std::fabs(ActorWorldTransform.Scale.x), std::fabs(ActorWorldTransform.Scale.y)),
			std::fabs(ActorWorldTransform.Scale.z));
		const float EffectiveSphereRadius = (std::max)(0.01f, SphereRadius * MaximumScaleAxis);
		const physx::PxSphereGeometry GeometryValue(EffectiveSphereRadius);
		NewShape = PhysicsInstance->createShape(GeometryValue, *PhysicsMaterial, true);
	}

	if (NewShape == nullptr)
	{
		DestroyPhysicsActor();
		return;
	}

	if (CollisionMode == PhysicsCollisionMode::Trigger)
	{
		NewShape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, false);
		NewShape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, true);
	}
	else
	{
		NewShape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, true);
		NewShape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, false);
	}

	PhysicsActor->attachShape(*NewShape);
	NewShape->release();

	if (PhysicsDynamicActor != nullptr)
	{
		if (Mass > 0.0f)
		{
			physx::PxRigidBodyExt::updateMassAndInertia(*PhysicsDynamicActor, Mass);
		}
		PhysicsDynamicActor->setLinearDamping(LinearDamping);
		PhysicsDynamicActor->setAngularDamping(AngularDamping);
		PhysicsDynamicActor->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY, UseGravity == false);
		PhysicsDynamicActor->setLinearVelocity(PhysXTypeConversion::ToPxVector(CachedVelocity));
		PhysicsDynamicActor->setAngularVelocity(PhysXTypeConversion::ToPxVector(CachedAngularVelocity));
	}

	PhysicsSceneInstance->addActor(*PhysicsActor);
	PhysicsSubsystemInstance->RegisterPhysicsActor(this, PhysicsActor);
}

void PhysicsComponent::DestroyPhysicsActor()
{
	ReleaseAllWeldJoints();

	if (PhysicsSubsystemInstance != nullptr && PhysicsActor != nullptr)
	{
		PhysicsSubsystemInstance->UnregisterPhysicsActor(PhysicsActor);
		physx::PxScene* PhysicsSceneInstance = PhysicsSubsystemInstance->GetScene();
		if (PhysicsSceneInstance != nullptr && PhysicsActor->getScene() == PhysicsSceneInstance)
		{
			PhysicsSceneInstance->removeActor(*PhysicsActor);
		}
	}

	if (PhysicsActor != nullptr)
	{
		PhysicsActor->release();
		PhysicsActor = nullptr;
		PhysicsDynamicActor = nullptr;
	}

	if (PhysicsMaterial != nullptr)
	{
		PhysicsMaterial->release();
		PhysicsMaterial = nullptr;
	}
}

void PhysicsComponent::SyncPhysicsTransformFromActor()
{
	if (PhysicsActor == nullptr)
	{
		return;
	}

	const Actor* OwningActorInstance = GetOwningActor();
	if (OwningActorInstance == nullptr)
	{
		return;
	}

	const Transform ActorTransform = OwningActorInstance->GetTransform();
	const physx::PxTransform PhysicsTransform = PhysXTypeConversion::ToPxTransform(ActorTransform);
	PhysicsActor->setGlobalPose(PhysicsTransform, true);
}

void PhysicsComponent::SyncActorTransformFromPhysics()
{
	if (PhysicsActor == nullptr || IsStatic)
	{
		return;
	}

	Actor* OwningActorInstance = GetOwningActor();
	if (OwningActorInstance == nullptr)
	{
		return;
	}

	const Transform ExistingActorTransform = OwningActorInstance->GetTransform();
	const physx::PxTransform PhysicsTransform = PhysicsActor->getGlobalPose();
	const Transform NewActorTransform = PhysXTypeConversion::ToDirectXTransform(
		PhysicsTransform,
		ExistingActorTransform.Scale);
	OwningActorInstance->SetTransform(NewActorTransform);
}

void PhysicsComponent::ApplyBoundarySphereConstraint(const DirectX::XMFLOAT3& BoundaryCenter, float BoundaryRadius)
{
	if (PhysicsDynamicActor == nullptr)
	{
		return;
	}

	physx::PxTransform PhysicsTransform = PhysicsDynamicActor->getGlobalPose();
	const physx::PxVec3 BoundaryCenterValue = PhysXTypeConversion::ToPxVector(BoundaryCenter);
	const physx::PxVec3 OffsetValue = PhysicsTransform.p - BoundaryCenterValue;
	const float OffsetLength = OffsetValue.magnitude();
	if (OffsetLength <= BoundaryRadius || OffsetLength <= 0.0001f)
	{
		return;
	}

	const physx::PxVec3 NormalizedDirection = OffsetValue / OffsetLength;
	PhysicsTransform.p = BoundaryCenterValue + (NormalizedDirection * BoundaryRadius);
	PhysicsDynamicActor->setGlobalPose(PhysicsTransform, true);

	physx::PxVec3 LinearVelocityValue = PhysicsDynamicActor->getLinearVelocity();
	const float VelocityNormalProjection = LinearVelocityValue.dot(NormalizedDirection);
	if (VelocityNormalProjection > 0.0f)
	{
		LinearVelocityValue -= NormalizedDirection * VelocityNormalProjection;
		PhysicsDynamicActor->setLinearVelocity(LinearVelocityValue);
	}
}

physx::PxRigidActor* PhysicsComponent::GetPhysicsActor() const
{
	return PhysicsActor;
}

physx::PxRigidDynamic* PhysicsComponent::GetPhysicsDynamicActor() const
{
	return PhysicsDynamicActor;
}

void PhysicsComponent::RecalculateInverseMass()
{
	if (IsStatic || Mass <= 0.0f)
	{
		InverseMass = 0.0f;
	}
	else
	{
		InverseMass = 1.0f / Mass;
	}
}

void PhysicsComponent::RebuildPhysicsActor()
{
	if (PhysicsSubsystemInstance == nullptr || GetIsInitialized() == false)
	{
		return;
	}

	CreatePhysicsActor(PhysicsSubsystemInstance);
}

void PhysicsComponent::ApplyCollisionModeToAllShapes()
{
	if (PhysicsActor == nullptr)
	{
		return;
	}

	const physx::PxU32 ShapeCount = PhysicsActor->getNbShapes();
	if (ShapeCount == 0)
	{
		return;
	}

	std::vector<physx::PxShape*> Shapes;
	Shapes.resize(ShapeCount);
	PhysicsActor->getShapes(Shapes.data(), ShapeCount);
	for (physx::PxShape* ExistingShape : Shapes)
	{
		if (ExistingShape == nullptr)
		{
			continue;
		}

		if (CollisionMode == PhysicsCollisionMode::Trigger)
		{
			ExistingShape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, false);
			ExistingShape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, true);
		}
		else
		{
			ExistingShape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, true);
			ExistingShape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, false);
		}
	}
}

void PhysicsComponent::ReleaseAllWeldJoints()
{
	for (physx::PxJoint*& ExistingJoint : WeldJoints)
	{
		if (ExistingJoint != nullptr)
		{
			ExistingJoint->release();
			ExistingJoint = nullptr;
		}
	}
	WeldJoints.clear();
}
