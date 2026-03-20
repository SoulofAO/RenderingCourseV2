#include "Tests/MeshTestGame.h"
#include "Abstracts/Components/MeshUniversalComponent.h"
#include "Abstracts/Components/PhysicsComponent.h"
#include "Abstracts/Core/Actor.h"
#include "Abstracts/Core/Transform.h"
#include "Abstracts/Subsystems/PhysicsSubsystem.h"
#include <memory>

MeshTestGame::MeshTestGame(LPCWSTR ApplicationName, int ScreenWidth, int ScreenHeight)
	: TestsBaseGame(ApplicationName, ScreenWidth, ScreenHeight)
{
	bSpawnDirectionalLightActor  = true;
}

MeshTestGame::~MeshTestGame() = default;

void MeshTestGame::BuildTestScene()
{
	PhysicsSubsystem* PhysicsSubsystemInstance = GetSubsystem<PhysicsSubsystem>();
	if (PhysicsSubsystemInstance != nullptr)
	{
		PhysicsSubsystemInstance->SetWorldBoundarySphere(DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f), 40.0f);
	}

	std::unique_ptr<Actor> FloorActor = std::make_unique<Actor>();
	Transform FloorTransform;
	FloorTransform.Position = DirectX::XMFLOAT3(0.0f, -2.0f, 0.0f);
	FloorTransform.RotationEuler = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
	FloorTransform.Scale = DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f);
	FloorActor->SetTransform(FloorTransform);
	std::unique_ptr<MeshUniversalComponent> FloorMeshComponent = std::make_unique<MeshUniversalComponent>();
	FloorMeshComponent->ModelMeshPath = "InputResources/Meshes/BlockArena.fbx";
	FloorMeshComponent->BaseColor = DirectX::XMFLOAT4(0.18f, 0.2f, 0.22f, 1.0f);
	std::unique_ptr<PhysicsComponent> FloorPhysicsComponent = std::make_unique<PhysicsComponent>();
	FloorPhysicsComponent->SetIsStatic(true);
	FloorPhysicsComponent->EnableAutoTriangleMeshColliderFromMesh(true);
	FloorActor->AddComponent(std::move(FloorMeshComponent));
	FloorActor->AddComponent(std::move(FloorPhysicsComponent));
	AddActor(std::move(FloorActor));

	const char* MeshPaths[] =
	{
		"InputResources/Meshes/SimpleCube.fbx",
		"InputResources/Meshes/SimpleSphere.fbx",
		"InputResources/Meshes/SimpleCone.fbx",
		"InputResources/Meshes/Test.fbx"
	};

	for (int MeshIndex = 0; MeshIndex < 4; ++MeshIndex)
	{
		std::unique_ptr<Actor> MeshActor = std::make_unique<Actor>();
		Transform MeshTransform;
		MeshTransform.Position = DirectX::XMFLOAT3(-6.0f + static_cast<float>(MeshIndex) * 4.0f, 1.0f, 3.0f);
		MeshTransform.Scale = DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f);
		MeshActor->SetTransform(MeshTransform);

		std::unique_ptr<MeshUniversalComponent> MeshComponent = std::make_unique<MeshUniversalComponent>();
		MeshComponent->ModelMeshPath = MeshPaths[MeshIndex];
		MeshComponent->BaseColor = DirectX::XMFLOAT4(
			0.25f + static_cast<float>(MeshIndex) * 0.18f,
			0.8f - static_cast<float>(MeshIndex) * 0.13f,
			0.35f + static_cast<float>(MeshIndex) * 0.1f,
			1.0f);

		std::unique_ptr<PhysicsComponent> MeshPhysicsComponent = std::make_unique<PhysicsComponent>();
		MeshPhysicsComponent->EnableAutoConvexColliderFromMesh(true);
		MeshPhysicsComponent->SetMass(1.0f + static_cast<float>(MeshIndex));
		MeshPhysicsComponent->SetRestitution(0.2f + static_cast<float>(MeshIndex) * 0.08f);

		MeshActor->AddComponent(std::move(MeshComponent));
		MeshActor->AddComponent(std::move(MeshPhysicsComponent));
		AddActor(std::move(MeshActor));
	}
}
