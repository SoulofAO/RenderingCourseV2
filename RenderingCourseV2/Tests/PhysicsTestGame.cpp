#include "Tests/PhysicsTestGame.h"
#include "Abstracts/Components/MeshUniversalComponent.h"
#include "Abstracts/Components/PhysicsComponent.h"
#include "Abstracts/Core/Actor.h"
#include "Abstracts/Core/Transform.h"
#include "Abstracts/Subsystems/PhysicsSubsystem.h"
#include <memory>

PhysicsTestGame::PhysicsTestGame(LPCWSTR ApplicationName, int ScreenWidth, int ScreenHeight)
	: TestsBaseGame(ApplicationName, ScreenWidth, ScreenHeight)
{
}

PhysicsTestGame::~PhysicsTestGame() = default;

void PhysicsTestGame::BuildTestScene()
{
	PhysicsSubsystem* PhysicsSubsystemInstance = GetSubsystem<PhysicsSubsystem>();
	if (PhysicsSubsystemInstance != nullptr)
	{
		PhysicsSubsystemInstance->SetFixedDeltaTime(1.0f / 90.0f);
		PhysicsSubsystemInstance->DisableWorldBoundarySphere();
	}

	std::unique_ptr<Actor> FloorActor = std::make_unique<Actor>();
	Transform FloorTransform;
	FloorTransform.Position = DirectX::XMFLOAT3(0.0, -2.0f, -26.0f);
	FloorTransform.Scale = DirectX::XMFLOAT3(26.0f, 1.0f, 26.0f);
	FloorActor->SetTransform(FloorTransform);
	std::unique_ptr<MeshUniversalComponent> FloorMeshComponent = std::make_unique<MeshUniversalComponent>();
	FloorMeshComponent->ModelMeshPath = "G:/RenderingCourseV2/InputResources/Meshes/SimpleCube.fbx";
	FloorMeshComponent->BaseColor = DirectX::XMFLOAT4(0.22f, 0.24f, 0.28f, 1.0f);
	std::unique_ptr<PhysicsComponent> FloorPhysicsComponent = std::make_unique<PhysicsComponent>();
	FloorPhysicsComponent->SetIsStatic(true);
	FloorPhysicsComponent->SetUseGravity(false);
	FloorPhysicsComponent->SetAabbCollider(DirectX::XMFLOAT3(0.5f, 0.5f, 0.5f));
	FloorActor->AddComponent(std::move(FloorMeshComponent));
	FloorActor->AddComponent(std::move(FloorPhysicsComponent));
	AddActor(std::move(FloorActor));

	std::unique_ptr<Actor> SphereActor = std::make_unique<Actor>();
	Transform SphereTransform;
	SphereTransform.Position = DirectX::XMFLOAT3(0.0f, 8.0f, 0.0f);
	SphereTransform.Scale = DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f);
	SphereActor->SetTransform(SphereTransform);
	std::unique_ptr<MeshUniversalComponent> SphereMeshComponent = std::make_unique<MeshUniversalComponent>();
	SphereMeshComponent->ModelMeshPath = "G:/RenderingCourseV2/InputResources/Meshes/SimpleSphere.fbx";
	SphereMeshComponent->BaseColor = DirectX::XMFLOAT4(0.95f, 0.28f, 0.28f, 1.0f);
	std::unique_ptr<PhysicsComponent> SpherePhysicsComponent = std::make_unique<PhysicsComponent>();
	SpherePhysicsComponent->SetMass(1.0f);
	SpherePhysicsComponent->SetUseGravity(true);
	SpherePhysicsComponent->SetSphereCollider(0.5f);
	SpherePhysicsComponent->SetRestitution(0.2f);
	SphereActor->AddComponent(std::move(SphereMeshComponent));
	SphereActor->AddComponent(std::move(SpherePhysicsComponent));
	AddActor(std::move(SphereActor));

	std::unique_ptr<Actor> CubeActor = std::make_unique<Actor>();
	Transform CubeTransform;
	CubeTransform.Position = DirectX::XMFLOAT3(0.0f, 11.0f, 0.0f);
	CubeTransform.Scale = DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f);
	CubeActor->SetTransform(CubeTransform);
	std::unique_ptr<MeshUniversalComponent> CubeMeshComponent = std::make_unique<MeshUniversalComponent>();
	CubeMeshComponent->ModelMeshPath = "G:/RenderingCourseV2/InputResources/Meshes/SimpleCube.fbx";
	CubeMeshComponent->BaseColor = DirectX::XMFLOAT4(0.25f, 0.62f, 0.98f, 1.0f);
	std::unique_ptr<PhysicsComponent> CubePhysicsComponent = std::make_unique<PhysicsComponent>();
	CubePhysicsComponent->SetMass(1.1f);
	CubePhysicsComponent->SetUseGravity(true);
	CubePhysicsComponent->SetAabbCollider(DirectX::XMFLOAT3(0.5f, 0.5f, 0.5f));
	CubePhysicsComponent->SetRestitution(0.15f);
	CubeActor->AddComponent(std::move(CubeMeshComponent));
	CubeActor->AddComponent(std::move(CubePhysicsComponent));
	AddActor(std::move(CubeActor));

	std::unique_ptr<Actor> TriangleActor = std::make_unique<Actor>();
	Transform TriangleTransform;
	TriangleTransform.Position = DirectX::XMFLOAT3(0.0f, 14.0f, 0.0f);
	TriangleTransform.Scale = DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f);
	TriangleActor->SetTransform(TriangleTransform);
	std::unique_ptr<MeshUniversalComponent> TriangleMeshComponent = std::make_unique<MeshUniversalComponent>();
	TriangleMeshComponent->ModelMeshPath = "G:/RenderingCourseV2/InputResources/Meshes/Test.fbx";
	TriangleMeshComponent->BaseColor = DirectX::XMFLOAT4(0.35f, 0.95f, 0.45f, 1.0f);
	std::unique_ptr<PhysicsComponent> TrianglePhysicsComponent = std::make_unique<PhysicsComponent>();
	TrianglePhysicsComponent->SetMass(1.2f);
	TrianglePhysicsComponent->SetUseGravity(true);
	TrianglePhysicsComponent->EnableAutoConvexColliderFromMesh(true);
	TrianglePhysicsComponent->SetRestitution(0.1f);
	TriangleActor->AddComponent(std::move(TriangleMeshComponent));
	TriangleActor->AddComponent(std::move(TrianglePhysicsComponent));
	AddActor(std::move(TriangleActor));
}
