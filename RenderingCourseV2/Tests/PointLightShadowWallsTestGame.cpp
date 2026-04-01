#include "Tests/PointLightShadowWallsTestGame.h"
#include "Abstracts/Components/LightComponent.h"
#include "Abstracts/Components/MeshUniversalComponent.h"
#include "Abstracts/Core/Actor.h"
#include "Abstracts/Core/Transform.h"
#include "Abstracts/Subsystems/SceneViewportSubsystem.h"
#include <memory>

PointLightShadowWallsTestGame::PointLightShadowWallsTestGame(LPCWSTR ApplicationName, int ScreenWidth, int ScreenHeight)
	: TestsBaseGame(ApplicationName, ScreenWidth, ScreenHeight)
{
	bSpawnDirectionalLightActor = false;
}

PointLightShadowWallsTestGame::~PointLightShadowWallsTestGame() = default;

void PointLightShadowWallsTestGame::BuildTestScene()
{
	SceneViewportSubsystem* SceneViewportSubsystemInstance = GetSubsystem<SceneViewportSubsystem>();
	if (SceneViewportSubsystemInstance != nullptr)
	{
		SceneViewportSubsystemInstance->SetRenderPipelineType(RenderPipelineType::Deferred);
	}

	std::unique_ptr<Actor> FloorActor = std::make_unique<Actor>();
	Transform FloorTransform;
	FloorTransform.Position = DirectX::XMFLOAT3(0.0f, -0.6f, 3.5f);
	FloorTransform.Scale = DirectX::XMFLOAT3(16.0f, 1.0f, 16.0f);
	FloorActor->SetTransform(FloorTransform);
	std::unique_ptr<MeshUniversalComponent> FloorMeshComponent = std::make_unique<MeshUniversalComponent>();
	FloorMeshComponent->ModelMeshPath = "InputResources/Meshes/SimpleCube.fbx";
	FloorMeshComponent->BaseColor = DirectX::XMFLOAT4(0.12f, 0.12f, 0.14f, 1.0f);
	FloorActor->AddComponent(std::move(FloorMeshComponent));
	AddActor(std::move(FloorActor));

	std::unique_ptr<Actor> BackWallActor = std::make_unique<Actor>();
	Transform BackWallTransform;
	BackWallTransform.Position = DirectX::XMFLOAT3(0.0f, 2.0f, 8.5f);
	BackWallTransform.Scale = DirectX::XMFLOAT3(10.0f, 5.0f, 0.4f);
	BackWallActor->SetTransform(BackWallTransform);
	std::unique_ptr<MeshUniversalComponent> BackWallMeshComponent = std::make_unique<MeshUniversalComponent>();
	BackWallMeshComponent->ModelMeshPath = "InputResources/Meshes/SimpleCube.fbx";
	BackWallMeshComponent->BaseColor = DirectX::XMFLOAT4(0.45f, 0.45f, 0.48f, 1.0f);
	BackWallActor->AddComponent(std::move(BackWallMeshComponent));
	AddActor(std::move(BackWallActor));

	std::unique_ptr<Actor> LeftWallActor = std::make_unique<Actor>();
	Transform LeftWallTransform;
	LeftWallTransform.Position = DirectX::XMFLOAT3(-5.0f, 2.0f, 3.5f);
	LeftWallTransform.RotationEuler = DirectX::XMFLOAT3(0.0f, 1.5707963f, 0.0f);
	LeftWallTransform.Scale = DirectX::XMFLOAT3(10.0f, 5.0f, 0.4f);
	LeftWallActor->SetTransform(LeftWallTransform);
	std::unique_ptr<MeshUniversalComponent> LeftWallMeshComponent = std::make_unique<MeshUniversalComponent>();
	LeftWallMeshComponent->ModelMeshPath = "InputResources/Meshes/SimpleCube.fbx";
	LeftWallMeshComponent->BaseColor = DirectX::XMFLOAT4(0.4f, 0.42f, 0.45f, 1.0f);
	LeftWallActor->AddComponent(std::move(LeftWallMeshComponent));
	AddActor(std::move(LeftWallActor));

	std::unique_ptr<Actor> RightWallActor = std::make_unique<Actor>();
	Transform RightWallTransform;
	RightWallTransform.Position = DirectX::XMFLOAT3(5.0f, 2.0f, 3.5f);
	RightWallTransform.RotationEuler = DirectX::XMFLOAT3(0.0f, 1.5707963f, 0.0f);
	RightWallTransform.Scale = DirectX::XMFLOAT3(10.0f, 5.0f, 0.4f);
	RightWallActor->SetTransform(RightWallTransform);
	std::unique_ptr<MeshUniversalComponent> RightWallMeshComponent = std::make_unique<MeshUniversalComponent>();
	RightWallMeshComponent->ModelMeshPath = "InputResources/Meshes/SimpleCube.fbx";
	RightWallMeshComponent->BaseColor = DirectX::XMFLOAT4(0.4f, 0.42f, 0.45f, 1.0f);
	RightWallActor->AddComponent(std::move(RightWallMeshComponent));
	AddActor(std::move(RightWallActor));

	std::unique_ptr<Actor> ShadowTestMeshActor = std::make_unique<Actor>();
	Transform ShadowTestMeshTransform;
	ShadowTestMeshTransform.Position = DirectX::XMFLOAT3(0.0f, 2.0f, 1.5f);
	ShadowTestMeshTransform.Scale = DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f);
	ShadowTestMeshActor->SetTransform(ShadowTestMeshTransform);
	std::unique_ptr<MeshUniversalComponent> ShadowTestMeshComponent = std::make_unique<MeshUniversalComponent>();
	ShadowTestMeshComponent->ModelMeshPath = "InputResources/Meshes/ShadowTestMesh.fbx";
	ShadowTestMeshComponent->BaseColor = DirectX::XMFLOAT4(0.85f, 0.82f, 0.78f, 1.0f);
	ShadowTestMeshComponent->SpecularPower = 24.0f;
	ShadowTestMeshComponent->SpecularIntensity = 0.35f;
	ShadowTestMeshActor->AddComponent(std::move(ShadowTestMeshComponent));
	AddActor(std::move(ShadowTestMeshActor));

	std::unique_ptr<Actor> PointLightActor = std::make_unique<Actor>();
	Transform PointLightTransform;
	PointLightTransform.Position = DirectX::XMFLOAT3(0.0f, 2.0f, -2.5f);
	PointLightActor->SetTransform(PointLightTransform);
	std::unique_ptr<LightComponent> PointLightComponent = std::make_unique<LightComponent>();
	PointLightComponent->SetLightType(LightType::Point);
	PointLightComponent->SetColor(DirectX::XMFLOAT4(1.0f, 0.95f, 0.85f, 1.0f));
	PointLightComponent->SetIntensity(7.0f);
	PointLightComponent->SetRange(18.0f);
	PointLightActor->AddComponent(std::move(PointLightComponent));
	AddActor(std::move(PointLightActor));
}
