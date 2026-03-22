#include "Tests/ParticleTestGame.h"
#include "Abstracts/Components/MeshUniversalComponent.h"
#include "Abstracts/Components/ParticleRenderingComponent.h"
#include "Tests/ParticleTestUIRenderingComponent.h"
#include "Abstracts/Core/Actor.h"
#include "Abstracts/Core/Transform.h"
#include "Abstracts/Subsystems/SceneViewportSubsystem.h"
#include <memory>

ParticleTestGame::ParticleTestGame(LPCWSTR ApplicationName, int ScreenWidth, int ScreenHeight)
	: TestsBaseGame(ApplicationName, ScreenWidth, ScreenHeight)
{
	bSpawnDirectionalLightActor = true;
	CurrentMouseInputMode = MouseInputMode::UIOnly;
}

ParticleTestGame::~ParticleTestGame() = default;

void ParticleTestGame::BuildTestScene()
{
	SceneViewportSubsystem* SceneViewportSubsystemInstance = GetSubsystem<SceneViewportSubsystem>();
	if (SceneViewportSubsystemInstance != nullptr)
	{
		SceneViewportSubsystemInstance->SetRenderPipelineType(RenderPipelineType::Forward);
	}

	/*
	std::unique_ptr<Actor> FloorActor = std::make_unique<Actor>();
	Transform FloorTransform;
	FloorTransform.Position = DirectX::XMFLOAT3(0.0f, -0.5f, 0.0f);
	FloorTransform.Scale = DirectX::XMFLOAT3(32.0f, 1.0f, 32.0f);
	FloorActor->SetTransform(FloorTransform);
	std::unique_ptr<MeshUniversalComponent> FloorMeshComponent = std::make_unique<MeshUniversalComponent>();
	FloorMeshComponent->ModelMeshPath = "InputResources/Meshes/SimpleCube.fbx";
	FloorMeshComponent->BaseColor = DirectX::XMFLOAT4(0.1f, 0.1f, 0.11f, 1.0f);
	FloorMeshComponent->SpecularPower = 12.0f;
	FloorMeshComponent->SpecularIntensity = 0.4f;
	FloorActor->AddComponent(std::move(FloorMeshComponent));
	AddActor(std::move(FloorActor));
	*/

	std::unique_ptr<Actor> ParticleActor = std::make_unique<Actor>();
	std::unique_ptr<ParticleRenderingComponent> ParticleRenderingComponentInstance = std::make_unique<ParticleRenderingComponent>(4096);
	ParticleActor->AddComponent(std::move(ParticleRenderingComponentInstance));
	AddActor(std::move(ParticleActor));

	std::unique_ptr<Actor> UserInterfaceActor = std::make_unique<Actor>();
	std::unique_ptr<ParticleTestUIRenderingComponent> UserInterfaceRenderingComponentInstance =
		std::make_unique<ParticleTestUIRenderingComponent>();
	UserInterfaceActor->AddComponent(std::move(UserInterfaceRenderingComponentInstance));
	AddActor(std::move(UserInterfaceActor));
}
