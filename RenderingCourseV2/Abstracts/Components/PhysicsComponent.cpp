#include "Abstracts/Components/PhysicsComponent.h"
#include "Abstracts/Core/Actor.h"
#include "Abstracts/Core/Game.h"
#include "Abstracts/Subsystems/PhysicsSubsystem.h"
#include <algorithm>

PhysicsComponent::PhysicsComponent()
	: Mass(1.0f)
	, InverseMass(1.0f)
	, Restitution(0.5f)
	, IsStatic(false)
	, UsesSphereCollider(true)
	, SphereRadius(0.5f)
	, HalfExtents(0.5f, 0.5f, 0.5f)
	, Velocity(0.0f, 0.0f, 0.0f)
	, AccumulatedForce(0.0f, 0.0f, 0.0f)
{
}

PhysicsComponent::~PhysicsComponent() = default;

void PhysicsComponent::Initialize()
{
	ActorComponent::Initialize();

	Game* GameInstance = GetOwningGame();
	if (GameInstance != nullptr)
	{
		PhysicsSubsystem* Physics = GameInstance->GetSubsystem<PhysicsSubsystem>();
		if (Physics != nullptr)
		{
			Physics->RegisterPhysicsComponent(this);
		}
	}
}

void PhysicsComponent::Shutdown()
{
	Game* GameInstance = GetOwningGame();
	if (GameInstance != nullptr)
	{
		PhysicsSubsystem* Physics = GameInstance->GetSubsystem<PhysicsSubsystem>();
		if (Physics != nullptr)
		{
			Physics->UnregisterPhysicsComponent(this);
		}
	}

	ActorComponent::Shutdown();
}

void PhysicsComponent::SetMass(float NewMass)
{
	Mass = (std::max)(NewMass, 0.0f);
	RecalculateInverseMass();
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
}

float PhysicsComponent::GetRestitution() const
{
	return Restitution;
}

void PhysicsComponent::SetVelocity(const DirectX::XMFLOAT3& NewVelocity)
{
	Velocity = NewVelocity;
}

const DirectX::XMFLOAT3& PhysicsComponent::GetVelocity() const
{
	return Velocity;
}

void PhysicsComponent::AddForce(const DirectX::XMFLOAT3& Force)
{
	AccumulatedForce.x += Force.x;
	AccumulatedForce.y += Force.y;
	AccumulatedForce.z += Force.z;
}

void PhysicsComponent::ClearForces()
{
	AccumulatedForce = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
}

void PhysicsComponent::SetIsStatic(bool NewIsStatic)
{
	IsStatic = NewIsStatic;
	RecalculateInverseMass();
}

bool PhysicsComponent::GetIsStatic() const
{
	return IsStatic;
}

void PhysicsComponent::SetSphereCollider(float NewRadius)
{
	UsesSphereCollider = true;
	SphereRadius = (std::max)(NewRadius, 0.01f);
}

void PhysicsComponent::SetAabbCollider(const DirectX::XMFLOAT3& NewHalfExtents)
{
	UsesSphereCollider = false;
	HalfExtents.x = (std::max)(NewHalfExtents.x, 0.01f);
	HalfExtents.y = (std::max)(NewHalfExtents.y, 0.01f);
	HalfExtents.z = (std::max)(NewHalfExtents.z, 0.01f);
}

bool PhysicsComponent::GetUsesSphereCollider() const
{
	return UsesSphereCollider;
}

DirectX::BoundingSphere PhysicsComponent::GetBoundingSphere() const
{
	const DirectX::XMFLOAT3& Position = GetOwningActor()->GetPosition();
	return DirectX::BoundingSphere(Position, SphereRadius);
}

DirectX::BoundingBox PhysicsComponent::GetBoundingBox() const
{
	const DirectX::XMFLOAT3& Position = GetOwningActor()->GetPosition();
	return DirectX::BoundingBox(Position, HalfExtents);
}

const DirectX::XMFLOAT3& PhysicsComponent::GetHalfExtents() const
{
	return HalfExtents;
}

float PhysicsComponent::GetSphereRadius() const
{
	return SphereRadius;
}

void PhysicsComponent::Integrate(float DeltaTime)
{
	if (IsStatic || InverseMass <= 0.0f)
	{
		ClearForces();
		return;
	}

	DirectX::XMFLOAT3 Acceleration(
		AccumulatedForce.x * InverseMass,
		AccumulatedForce.y * InverseMass,
		AccumulatedForce.z * InverseMass);

	Velocity.x += Acceleration.x * DeltaTime;
	Velocity.y += Acceleration.y * DeltaTime;
	Velocity.z += Acceleration.z * DeltaTime;

	Actor* OwningActor = GetOwningActor();
	DirectX::XMFLOAT3 Position = OwningActor->GetPosition();
	Position.x += Velocity.x * DeltaTime;
	Position.y += Velocity.y * DeltaTime;
	Position.z += Velocity.z * DeltaTime;
	OwningActor->SetPosition(Position);

	ClearForces();
}

void PhysicsComponent::ApplyImpulse(const DirectX::XMFLOAT3& Impulse)
{
	if (IsStatic || InverseMass <= 0.0f)
	{
		return;
	}

	Velocity.x += Impulse.x * InverseMass;
	Velocity.y += Impulse.y * InverseMass;
	Velocity.z += Impulse.z * InverseMass;
}

void PhysicsComponent::ApplyPositionCorrection(const DirectX::XMFLOAT3& Correction)
{
	if (IsStatic || InverseMass <= 0.0f)
	{
		return;
	}

	Actor* OwningActor = GetOwningActor();
	DirectX::XMFLOAT3 Position = OwningActor->GetPosition();
	Position.x += Correction.x;
	Position.y += Correction.y;
	Position.z += Correction.z;
	OwningActor->SetPosition(Position);
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
