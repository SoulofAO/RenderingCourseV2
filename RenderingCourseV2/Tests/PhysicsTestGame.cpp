#include "Tests/PhysicsTestGame.h"
#include "Abstracts/Components/MeshUniversalComponent.h"
#include "Abstracts/Components/PhysicsComponent.h"
#include "Abstracts/Core/Actor.h"
#include "Abstracts/Core/Transform.h"
#include "Abstracts/Subsystems/PhysicsSubsystem.h"
#include <iostream>
#include <memory>

PhysicsTestGame::PhysicsTestGame(LPCWSTR ApplicationName, int ScreenWidth, int ScreenHeight)
	: TestsBaseGame(ApplicationName, ScreenWidth, ScreenHeight)
	, PlayerSpherePhysicsComponent(nullptr)
	, CollisionDelegateHandle()
	, OverlapBeginDelegateHandle()
	, OverlapEndDelegateHandle()
	, CollisionEventCount(0)
	, OverlapBeginEventCount(0)
	, OverlapEndEventCount(0)
{
}

PhysicsTestGame::~PhysicsTestGame()
{
	PhysicsSubsystem* PhysicsSubsystemInstance = GetSubsystem<PhysicsSubsystem>();
	if (PhysicsSubsystemInstance != nullptr)
	{
		if (CollisionDelegateHandle.IsValid())
		{
			PhysicsSubsystemInstance->GetOnCollisionDetectedDelegate().Remove(CollisionDelegateHandle);
			CollisionDelegateHandle.Reset();
		}
		if (OverlapBeginDelegateHandle.IsValid())
		{
			PhysicsSubsystemInstance->GetOnOverlapBeginDelegate().Remove(OverlapBeginDelegateHandle);
			OverlapBeginDelegateHandle.Reset();
		}
		if (OverlapEndDelegateHandle.IsValid())
		{
			PhysicsSubsystemInstance->GetOnOverlapEndDelegate().Remove(OverlapEndDelegateHandle);
			OverlapEndDelegateHandle.Reset();
		}
	}
}

void PhysicsTestGame::BuildTestScene()
{
	PhysicsSubsystem* PhysicsSubsystemInstance = GetSubsystem<PhysicsSubsystem>();
	if (PhysicsSubsystemInstance != nullptr)
	{
		PhysicsSubsystemInstance->SetFixedDeltaTime(1.0f / 90.0f);
		PhysicsSubsystemInstance->SetWorldBoundarySphere(DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f), 26.0f);

		CollisionDelegateHandle = PhysicsSubsystemInstance->GetOnCollisionDetectedDelegate().AddRaw(
			this,
			&PhysicsTestGame::HandlePhysicsCollisionDetected);
		OverlapBeginDelegateHandle = PhysicsSubsystemInstance->GetOnOverlapBeginDelegate().AddRaw(
			this,
			&PhysicsTestGame::HandlePhysicsOverlapBegin);
		OverlapEndDelegateHandle = PhysicsSubsystemInstance->GetOnOverlapEndDelegate().AddRaw(
			this,
			&PhysicsTestGame::HandlePhysicsOverlapEnd);
	}

	std::unique_ptr<Actor> FloorActor = std::make_unique<Actor>();
	Transform FloorTransform;
	FloorTransform.Position = DirectX::XMFLOAT3(0.0f, -2.0f, 0.0f);
	FloorTransform.Scale = DirectX::XMFLOAT3(20.0f, 1.0f, 20.0f);
	FloorActor->SetTransform(FloorTransform);
	std::unique_ptr<MeshUniversalComponent> FloorMeshComponent = std::make_unique<MeshUniversalComponent>();
	FloorMeshComponent->ModelMeshPath = "G:/RenderingCourseV2/InputResources/Meshes/SimpleCube.fbx";
	FloorMeshComponent->BaseColor = DirectX::XMFLOAT4(0.22f, 0.24f, 0.28f, 1.0f);
	std::unique_ptr<PhysicsComponent> FloorPhysicsComponent = std::make_unique<PhysicsComponent>();
	FloorPhysicsComponent->SetIsStatic(true);
	FloorPhysicsComponent->SetAabbCollider(DirectX::XMFLOAT3(0.5f, 0.5f, 0.5f));
	FloorActor->AddComponent(std::move(FloorMeshComponent));
	FloorActor->AddComponent(std::move(FloorPhysicsComponent));
	AddActor(std::move(FloorActor));

	std::unique_ptr<Actor> PlayerSphereActor = std::make_unique<Actor>();
	Transform PlayerSphereTransform;
	PlayerSphereTransform.Position = DirectX::XMFLOAT3(-4.0f, 2.0f, -3.0f);
	PlayerSphereTransform.Scale = DirectX::XMFLOAT3(1.1f, 1.1f, 1.1f);
	PlayerSphereActor->SetTransform(PlayerSphereTransform);
	std::unique_ptr<MeshUniversalComponent> PlayerSphereMeshComponent = std::make_unique<MeshUniversalComponent>();
	PlayerSphereMeshComponent->ModelMeshPath = "G:/RenderingCourseV2/InputResources/Meshes/SimpleSphere.fbx";
	PlayerSphereMeshComponent->BaseColor = DirectX::XMFLOAT4(0.95f, 0.25f, 0.28f, 1.0f);
	std::unique_ptr<PhysicsComponent> PlayerSpherePhysics = std::make_unique<PhysicsComponent>();
	PlayerSpherePhysics->SetMass(2.0f);
	PlayerSpherePhysics->SetSphereCollider(0.5f);
	PlayerSpherePhysics->SetRestitution(0.45f);
	PlayerSpherePhysics->SetAngularVelocity(DirectX::XMFLOAT3(0.0f, 2.2f, 0.0f));
	PlayerSpherePhysicsComponent = PlayerSpherePhysics.get();
	PlayerSphereActor->AddComponent(std::move(PlayerSphereMeshComponent));
	PlayerSphereActor->AddComponent(std::move(PlayerSpherePhysics));
	AddActor(std::move(PlayerSphereActor));

	std::unique_ptr<Actor> TriggerActor = std::make_unique<Actor>();
	Transform TriggerTransform;
	TriggerTransform.Position = DirectX::XMFLOAT3(2.0f, 1.8f, 3.0f);
	TriggerTransform.Scale = DirectX::XMFLOAT3(2.0f, 2.0f, 2.0f);
	TriggerActor->SetTransform(TriggerTransform);
	std::unique_ptr<MeshUniversalComponent> TriggerMeshComponent = std::make_unique<MeshUniversalComponent>();
	TriggerMeshComponent->ModelMeshPath = "G:/RenderingCourseV2/InputResources/Meshes/SimpleCube.fbx";
	TriggerMeshComponent->BaseColor = DirectX::XMFLOAT4(0.1f, 0.8f, 0.8f, 1.0f);
	std::unique_ptr<PhysicsComponent> TriggerPhysicsComponent = std::make_unique<PhysicsComponent>();
	TriggerPhysicsComponent->SetIsStatic(true);
	TriggerPhysicsComponent->SetCollisionMode(PhysicsCollisionMode::Trigger);
	TriggerPhysicsComponent->SetAabbCollider(DirectX::XMFLOAT3(0.5f, 0.5f, 0.5f));
	TriggerActor->AddComponent(std::move(TriggerMeshComponent));
	TriggerActor->AddComponent(std::move(TriggerPhysicsComponent));
	AddActor(std::move(TriggerActor));

	SpawnDynamicPhysicsRow();
}

void PhysicsTestGame::Update(float DeltaTime)
{
	if (PlayerSpherePhysicsComponent != nullptr)
	{
		PlayerSpherePhysicsComponent->AddTorque(DirectX::XMFLOAT3(0.0f, 6.0f, 0.0f));
	}

	TestsBaseGame::Update(DeltaTime);
}

void PhysicsTestGame::HandlePhysicsCollisionDetected(
	PhysicsComponent* FirstPhysicsComponent,
	PhysicsComponent* SecondPhysicsComponent,
	const DirectX::XMFLOAT3& CollisionNormal,
	float CollisionPenetrationDepth)
{
	(void)FirstPhysicsComponent;
	(void)SecondPhysicsComponent;

	CollisionEventCount += 1;
	if (CollisionEventCount <= 12)
	{
		std::cout
			<< "PhysicsTest collision #" << CollisionEventCount
			<< " normal(" << CollisionNormal.x << ", " << CollisionNormal.y << ", " << CollisionNormal.z << ")"
			<< " depth " << CollisionPenetrationDepth
			<< std::endl;
	}
}

void PhysicsTestGame::HandlePhysicsOverlapBegin(
	PhysicsComponent* FirstPhysicsComponent,
	PhysicsComponent* SecondPhysicsComponent)
{
	(void)FirstPhysicsComponent;
	(void)SecondPhysicsComponent;

	OverlapBeginEventCount += 1;
	std::cout << "PhysicsTest overlap begin #" << OverlapBeginEventCount << std::endl;
}

void PhysicsTestGame::HandlePhysicsOverlapEnd(
	PhysicsComponent* FirstPhysicsComponent,
	PhysicsComponent* SecondPhysicsComponent)
{
	(void)FirstPhysicsComponent;
	(void)SecondPhysicsComponent;

	OverlapEndEventCount += 1;
	std::cout << "PhysicsTest overlap end #" << OverlapEndEventCount << std::endl;
}

void PhysicsTestGame::SpawnDynamicPhysicsRow()
{
	for (int ActorIndex = 0; ActorIndex < 6; ++ActorIndex)
	{
		std::unique_ptr<Actor> DynamicActor = std::make_unique<Actor>();
		Transform DynamicTransform;
		DynamicTransform.Position = DirectX::XMFLOAT3(
			-2.0f + static_cast<float>(ActorIndex) * 1.6f,
			3.4f + static_cast<float>(ActorIndex) * 0.22f,
			1.8f);
		DynamicTransform.Scale = DirectX::XMFLOAT3(0.8f, 0.8f, 0.8f);
		DynamicActor->SetTransform(DynamicTransform);

		std::unique_ptr<MeshUniversalComponent> DynamicMeshComponent = std::make_unique<MeshUniversalComponent>();
		DynamicMeshComponent->ModelMeshPath = (ActorIndex % 2 == 0)
			? "G:/RenderingCourseV2/InputResources/Meshes/SimpleCube.fbx"
			: "G:/RenderingCourseV2/InputResources/Meshes/SimpleCone.fbx";
		DynamicMeshComponent->BaseColor = DirectX::XMFLOAT4(
			0.25f + static_cast<float>(ActorIndex) * 0.1f,
			0.45f,
			0.95f - static_cast<float>(ActorIndex) * 0.09f,
			1.0f);

		std::unique_ptr<PhysicsComponent> DynamicPhysicsComponent = std::make_unique<PhysicsComponent>();
		DynamicPhysicsComponent->SetMass(1.0f + static_cast<float>(ActorIndex) * 0.3f);
		DynamicPhysicsComponent->SetRestitution(0.25f + static_cast<float>(ActorIndex) * 0.05f);
		if ((ActorIndex % 2) == 0)
		{
			DynamicPhysicsComponent->SetAabbCollider(DirectX::XMFLOAT3(0.5f, 0.5f, 0.5f));
		}
		else
		{
			DynamicPhysicsComponent->SetSphereCollider(0.45f);
		}

		DynamicActor->AddComponent(std::move(DynamicMeshComponent));
		DynamicActor->AddComponent(std::move(DynamicPhysicsComponent));
		AddActor(std::move(DynamicActor));
	}
}
