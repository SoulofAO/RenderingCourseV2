#include "Planets/PlanetsGame.h"
#include "Abstracts/Components/MeshUniversalComponent.h"
#include "Abstracts/Core/Actor.h"
#include "Abstracts/Core/Transform.h"

PlanetsGame::PlanetsGame(LPCWSTR ApplicationName, int ScreenWidth, int ScreenHeight)
	: Game(ApplicationName, ScreenWidth, ScreenHeight)
{
}

void PlanetsGame::BeginPlay()
{
	std::unique_ptr<Actor> MeshActor = std::make_unique<Actor>();
	Transform MeshTransform;
	MeshTransform.Position = DirectX::XMFLOAT3(0.0f, 0.0f, 4.0f);
	MeshTransform.Scale = DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f);
	MeshActor->SetTransform(MeshTransform);

	std::unique_ptr<MeshUniversalComponent> MeshComponent = std::make_unique<MeshUniversalComponent>();
	MeshComponent->ModelMeshPath = "G:/RenderingCourseV2/InputResources/Meshes/Test.fbx";
	MeshComponent->BaseColor = DirectX::XMFLOAT4(0.8f, 0.85f, 1.0f, 1.0f);
	MeshActor->AddComponent(std::move(MeshComponent));
	AddActor(std::move(MeshActor));

	Game::BeginPlay();
}
