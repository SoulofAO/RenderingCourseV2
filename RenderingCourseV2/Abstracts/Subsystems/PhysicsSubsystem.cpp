#include "Abstracts/Subsystems/PhysicsSubsystem.h"
#include "Abstracts/Components/PhysicsComponent.h"
#include <algorithm>
#include <cmath>

PhysicsSubsystem::PhysicsSubsystem()
	: FixedDeltaTime(1.0f / 60.0f)
	, AccumulatedTime(0.0f)
{
}

PhysicsSubsystem::~PhysicsSubsystem() = default;

void PhysicsSubsystem::Update(float DeltaTime)
{
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

	auto Position = std::find(PhysicsComponents.begin(), PhysicsComponents.end(), Component);
	if (Position == PhysicsComponents.end())
	{
		PhysicsComponents.push_back(Component);
	}
}

void PhysicsSubsystem::UnregisterPhysicsComponent(PhysicsComponent* Component)
{
	PhysicsComponents.erase(
		std::remove(PhysicsComponents.begin(), PhysicsComponents.end(), Component),
		PhysicsComponents.end());
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

PhysicsCollisionDetectedDelegate& PhysicsSubsystem::GetOnCollisionDetectedDelegate()
{
	return OnCollisionDetectedDelegate;
}

void PhysicsSubsystem::StepSimulation(float DeltaTime)
{
	for (PhysicsComponent* Component : PhysicsComponents)
	{
		if (Component->GetIsStatic() == false)
		{
			Component->AddForce(DirectX::XMFLOAT3(0.0f, -9.81f * Component->GetMass(), 0.0f));
		}
	}

	for (PhysicsComponent* Component : PhysicsComponents)
	{
		Component->Integrate(DeltaTime);
	}

	DetectAndResolveCollisions();
}

void PhysicsSubsystem::DetectAndResolveCollisions()
{
	const size_t ComponentCount = PhysicsComponents.size();
	for (size_t FirstIndex = 0; FirstIndex < ComponentCount; ++FirstIndex)
	{
		for (size_t SecondIndex = FirstIndex + 1; SecondIndex < ComponentCount; ++SecondIndex)
		{
			PhysicsComponent* FirstComponent = PhysicsComponents[FirstIndex];
			PhysicsComponent* SecondComponent = PhysicsComponents[SecondIndex];

			CollisionManifold Collision = {};
			if (TryBuildCollisionManifold(FirstComponent, SecondComponent, Collision))
			{
				ResolveCollision(Collision);
				OnCollisionDetectedDelegate.Broadcast(
					Collision.FirstComponent,
					Collision.SecondComponent,
					Collision.Normal,
					Collision.PenetrationDepth);
			}
		}
	}
}

bool PhysicsSubsystem::TryBuildCollisionManifold(PhysicsComponent* FirstComponent, PhysicsComponent* SecondComponent, CollisionManifold& OutCollision) const
{
	OutCollision.FirstComponent = FirstComponent;
	OutCollision.SecondComponent = SecondComponent;
	OutCollision.Normal = DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f);
	OutCollision.PenetrationDepth = 0.0f;

	const bool FirstIsSphere = FirstComponent->GetUsesSphereCollider();
	const bool SecondIsSphere = SecondComponent->GetUsesSphereCollider();

	if (FirstIsSphere && SecondIsSphere)
	{
		DirectX::BoundingSphere FirstSphere = FirstComponent->GetBoundingSphere();
		DirectX::BoundingSphere SecondSphere = SecondComponent->GetBoundingSphere();
		if (FirstSphere.Intersects(SecondSphere) == false)
		{
			return false;
		}

		DirectX::XMFLOAT3 Delta(
			SecondSphere.Center.x - FirstSphere.Center.x,
			SecondSphere.Center.y - FirstSphere.Center.y,
			SecondSphere.Center.z - FirstSphere.Center.z);
		float DistanceSquared = Delta.x * Delta.x + Delta.y * Delta.y + Delta.z * Delta.z;
		float Distance = std::sqrt((std::max)(DistanceSquared, 0.000001f));
		float RadiiSum = FirstSphere.Radius + SecondSphere.Radius;

		OutCollision.Normal = DirectX::XMFLOAT3(Delta.x / Distance, Delta.y / Distance, Delta.z / Distance);
		OutCollision.PenetrationDepth = (std::max)(RadiiSum - Distance, 0.0f);
		return true;
	}

	DirectX::BoundingBox FirstBox = FirstComponent->GetBoundingBox();
	DirectX::BoundingBox SecondBox = SecondComponent->GetBoundingBox();
	if (FirstBox.Intersects(SecondBox) == false)
	{
		return false;
	}

	DirectX::XMFLOAT3 Delta(
		SecondBox.Center.x - FirstBox.Center.x,
		SecondBox.Center.y - FirstBox.Center.y,
		SecondBox.Center.z - FirstBox.Center.z);

	float OverlapX = (FirstBox.Extents.x + SecondBox.Extents.x) - std::fabs(Delta.x);
	float OverlapY = (FirstBox.Extents.y + SecondBox.Extents.y) - std::fabs(Delta.y);
	float OverlapZ = (FirstBox.Extents.z + SecondBox.Extents.z) - std::fabs(Delta.z);

	OutCollision.PenetrationDepth = OverlapX;
	OutCollision.Normal = DirectX::XMFLOAT3((Delta.x >= 0.0f) ? 1.0f : -1.0f, 0.0f, 0.0f);

	if (OverlapY < OutCollision.PenetrationDepth)
	{
		OutCollision.PenetrationDepth = OverlapY;
		OutCollision.Normal = DirectX::XMFLOAT3(0.0f, (Delta.y >= 0.0f) ? 1.0f : -1.0f, 0.0f);
	}

	if (OverlapZ < OutCollision.PenetrationDepth)
	{
		OutCollision.PenetrationDepth = OverlapZ;
		OutCollision.Normal = DirectX::XMFLOAT3(0.0f, 0.0f, (Delta.z >= 0.0f) ? 1.0f : -1.0f);
	}

	return OutCollision.PenetrationDepth > 0.0f;
}

void PhysicsSubsystem::ResolveCollision(const CollisionManifold& Collision) const
{
	PhysicsComponent* FirstComponent = Collision.FirstComponent;
	PhysicsComponent* SecondComponent = Collision.SecondComponent;

	float FirstInverseMass = FirstComponent->GetInverseMass();
	float SecondInverseMass = SecondComponent->GetInverseMass();
	float TotalInverseMass = FirstInverseMass + SecondInverseMass;

	if (TotalInverseMass <= 0.0f)
	{
		return;
	}

	DirectX::XMFLOAT3 FirstVelocity = FirstComponent->GetVelocity();
	DirectX::XMFLOAT3 SecondVelocity = SecondComponent->GetVelocity();
	DirectX::XMFLOAT3 RelativeVelocity(
		SecondVelocity.x - FirstVelocity.x,
		SecondVelocity.y - FirstVelocity.y,
		SecondVelocity.z - FirstVelocity.z);

	float VelocityAlongNormal =
		RelativeVelocity.x * Collision.Normal.x +
		RelativeVelocity.y * Collision.Normal.y +
		RelativeVelocity.z * Collision.Normal.z;

	if (VelocityAlongNormal < 0.0f)
	{
		float Restitution = (std::min)(FirstComponent->GetRestitution(), SecondComponent->GetRestitution());
		float ImpulseScalar = -(1.0f + Restitution) * VelocityAlongNormal / TotalInverseMass;
		DirectX::XMFLOAT3 Impulse(
			Collision.Normal.x * ImpulseScalar,
			Collision.Normal.y * ImpulseScalar,
			Collision.Normal.z * ImpulseScalar);

		FirstComponent->ApplyImpulse(DirectX::XMFLOAT3(-Impulse.x, -Impulse.y, -Impulse.z));
		SecondComponent->ApplyImpulse(Impulse);
	}

	const float PositionCorrectionPercent = 0.8f;
	const float PositionCorrectionSlop = 0.001f;
	float CorrectedPenetration = (std::max)(Collision.PenetrationDepth - PositionCorrectionSlop, 0.0f);
	float CorrectionScalar = CorrectedPenetration / TotalInverseMass * PositionCorrectionPercent;
	DirectX::XMFLOAT3 Correction(
		Collision.Normal.x * CorrectionScalar,
		Collision.Normal.y * CorrectionScalar,
		Collision.Normal.z * CorrectionScalar);

	FirstComponent->ApplyPositionCorrection(DirectX::XMFLOAT3(-Correction.x * FirstInverseMass, -Correction.y * FirstInverseMass, -Correction.z * FirstInverseMass));
	SecondComponent->ApplyPositionCorrection(DirectX::XMFLOAT3(Correction.x * SecondInverseMass, Correction.y * SecondInverseMass, Correction.z * SecondInverseMass));
}
