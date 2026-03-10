#pragma once

#include "Engine/Core/Runtime/Abstract/Components/ActorComponent.h"
#include <DirectXCollision.h>
#include <directxmath.h>

class PhysicsSubsystem;

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

	void SetVelocity(const DirectX::XMFLOAT3& NewVelocity);
	const DirectX::XMFLOAT3& GetVelocity() const;
	void AddForce(const DirectX::XMFLOAT3& Force);
	void ClearForces();

	void SetIsStatic(bool NewIsStatic);
	bool GetIsStatic() const;

	void SetSphereCollider(float NewRadius);
	void SetAabbCollider(const DirectX::XMFLOAT3& NewHalfExtents);
	bool GetUsesSphereCollider() const;

	DirectX::BoundingSphere GetBoundingSphere() const;
	DirectX::BoundingBox GetBoundingBox() const;
	const DirectX::XMFLOAT3& GetHalfExtents() const;
	float GetSphereRadius() const;

	void Integrate(float DeltaTime);
	void ApplyImpulse(const DirectX::XMFLOAT3& Impulse);
	void ApplyPositionCorrection(const DirectX::XMFLOAT3& Correction);

private:
	void RecalculateInverseMass();

	float Mass;
	float InverseMass;
	float Restitution;
	bool IsStatic;

	bool UsesSphereCollider;
	float SphereRadius;
	DirectX::XMFLOAT3 HalfExtents;

	DirectX::XMFLOAT3 Velocity;
	DirectX::XMFLOAT3 AccumulatedForce;
};

