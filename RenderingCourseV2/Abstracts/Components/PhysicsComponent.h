#pragma once

#include "Abstracts/Components/ActorComponent.h"
#include <physx/PxPhysicsAPI.h>
#include <directxmath.h>
#include <cstdint>
#include <string>
#include <vector>

class PhysicsSubsystem;

enum class PhysicsColliderKind
{
	Sphere,
	Box,
	ConvexMeshAuto,
	TriangleMeshAuto
};

enum class PhysicsCollisionMode
{
	Simulation,
	Trigger
};

class PhysicsComponent : public ActorComponent
{
public:
	PhysicsComponent();
	~PhysicsComponent() override;

	void Initialize() override;
	void Shutdown() override;

	void SetMass(float NewMass);
	float GetMass() const;
	float GetInverseMass() const;

	void SetRestitution(float NewRestitution);
	float GetRestitution() const;

	void SetLinearDamping(float NewLinearDamping);
	float GetLinearDamping() const;
	void SetAngularDamping(float NewAngularDamping);
	float GetAngularDamping() const;

	void SetUseGravity(bool NewUseGravity);
	bool GetUseGravity() const;

	void SetVelocity(const DirectX::XMFLOAT3& NewVelocity);
	DirectX::XMFLOAT3 GetVelocity() const;
	void SetAngularVelocity(const DirectX::XMFLOAT3& NewAngularVelocity);
	DirectX::XMFLOAT3 GetAngularVelocity() const;
	void AddForce(const DirectX::XMFLOAT3& ForceValue);
	void AddTorque(const DirectX::XMFLOAT3& TorqueValue);
	void ClearForces();

	void SetIsStatic(bool NewIsStatic);
	bool GetIsStatic() const;

	void SetSphereCollider(float NewRadius);
	void SetAabbCollider(const DirectX::XMFLOAT3& NewHalfExtents);
	void EnableAutoConvexColliderFromMesh(bool NewAutoConvexEnabled);
	void EnableAutoTriangleMeshColliderFromMesh(bool NewAutoTriangleMeshEnabled);
	bool GetAutoConvexColliderFromMeshEnabled() const;
	bool GetAutoTriangleMeshColliderFromMeshEnabled() const;
	bool GetUsesSphereCollider() const;
	const DirectX::XMFLOAT3& GetHalfExtents() const;
	float GetSphereRadius() const;

	void SetCollisionMode(PhysicsCollisionMode NewCollisionMode);
	PhysicsCollisionMode GetCollisionMode() const;
	void SetCollisionLayer(std::uint32_t NewCollisionLayer);
	std::uint32_t GetCollisionLayer() const;
	void SetCollisionMask(std::uint32_t NewCollisionMask);
	std::uint32_t GetCollisionMask() const;

	void Integrate(float DeltaTime);
	void ApplyImpulse(const DirectX::XMFLOAT3& ImpulseValue);
	void ApplyAngularImpulse(const DirectX::XMFLOAT3& AngularImpulseValue);
	void ApplyPositionCorrection(const DirectX::XMFLOAT3& CorrectionValue);

	bool WeldWithComponent(PhysicsComponent* TargetComponent, bool PreserveRelativePose);
	void UnweldAllComponents();

	void CreatePhysicsActor(PhysicsSubsystem* NewPhysicsSubsystem);
	void DestroyPhysicsActor();
	void SyncPhysicsTransformFromActor();
	void SyncActorTransformFromPhysics();
	void ApplyBoundarySphereConstraint(const DirectX::XMFLOAT3& BoundaryCenter, float BoundaryRadius);

	physx::PxRigidActor* GetPhysicsActor() const;
	physx::PxRigidDynamic* GetPhysicsDynamicActor() const;

private:
	void RecalculateInverseMass();
	void RebuildPhysicsActor();
	void ApplyCollisionModeToAllShapes();
	void ApplyCollisionFilterDataToAllShapes();
	void ReleaseAllWeldJoints();

	float Mass;
	float InverseMass;
	float Restitution;
	float LinearDamping;
	float AngularDamping;
	bool UseGravity;
	bool IsStatic;
	PhysicsColliderKind ColliderKind;
	PhysicsCollisionMode CollisionMode;
	std::uint32_t CollisionLayer;
	std::uint32_t CollisionMask;
	float SphereRadius;
	DirectX::XMFLOAT3 HalfExtents;
	DirectX::XMFLOAT3 CachedVelocity;
	DirectX::XMFLOAT3 CachedAngularVelocity;

	PhysicsSubsystem* PhysicsSubsystemInstance;
	physx::PxMaterial* PhysicsMaterial;
	physx::PxRigidActor* PhysicsActor;
	physx::PxRigidDynamic* PhysicsDynamicActor;
	std::vector<physx::PxJoint*> WeldJoints;
};
