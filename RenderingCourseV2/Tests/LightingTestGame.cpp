#include "Tests/LightingTestGame.h"
#include "Abstracts/Components/MeshUniversalComponent.h"
#include "Abstracts/Core/Actor.h"
#include "Abstracts/Core/Transform.h"
#include <memory>

LightingTestGame::LightingTestGame(LPCWSTR ApplicationName, int ScreenWidth, int ScreenHeight)
	: TestsBaseGame(ApplicationName, ScreenWidth, ScreenHeight)
{
}

LightingTestGame::~LightingTestGame() = default;

void LightingTestGame::BuildTestScene()
{
	std::unique_ptr<Actor> FloorActor = std::make_unique<Actor>();
	Transform FloorTransform;
	FloorTransform.Position = DirectX::XMFLOAT3(0.0f, -2.2f, 0.0f);
	FloorTransform.Scale = DirectX::XMFLOAT3(28.0f, 1.0f, 28.0f);
	FloorActor->SetTransform(FloorTransform);
	std::unique_ptr<MeshUniversalComponent> FloorMeshComponent = std::make_unique<MeshUniversalComponent>();
	FloorMeshComponent->ModelMeshPath = "../../InputResources/Meshes/SimpleCube.fbx";
	FloorMeshComponent->BaseColor = DirectX::XMFLOAT4(0.08f, 0.08f, 0.09f, 1.0f);
	FloorMeshComponent->SpecularPower = 10.0f;
	FloorMeshComponent->SpecularIntensity = 0.25f;
	FloorActor->AddComponent(std::move(FloorMeshComponent));
	AddActor(std::move(FloorActor));

	for (int ActorIndex = 0; ActorIndex < 6; ++ActorIndex)
	{
		std::unique_ptr<Actor> SphereActor = std::make_unique<Actor>();
		Transform SphereTransform;
		SphereTransform.Position = DirectX::XMFLOAT3(-10.0f + static_cast<float>(ActorIndex) * 4.0f, 0.8f, 1.8f);
		SphereTransform.Scale = DirectX::XMFLOAT3(1.2f, 1.2f, 1.2f);
		SphereActor->SetTransform(SphereTransform);

		std::unique_ptr<MeshUniversalComponent> SphereMeshComponent = std::make_unique<MeshUniversalComponent>();
		SphereMeshComponent->ModelMeshPath = "../../InputResources/Meshes/SimpleSphere.fbx";
		SphereMeshComponent->BaseColor = DirectX::XMFLOAT4(
			0.2f + static_cast<float>(ActorIndex) * 0.14f,
			0.2f + static_cast<float>(ActorIndex) * 0.11f,
			0.95f - static_cast<float>(ActorIndex) * 0.12f,
			1.0f);
		SphereMeshComponent->SpecularPower = 4.0f + static_cast<float>(ActorIndex) * 28.0f;
		SphereMeshComponent->SpecularIntensity = 0.1f + static_cast<float>(ActorIndex) * 0.17f;

		SphereActor->AddComponent(std::move(SphereMeshComponent));
		AddActor(std::move(SphereActor));
	}

	for (int ActorIndex = 0; ActorIndex < 3; ++ActorIndex)
	{
		std::unique_ptr<Actor> ConeActor = std::make_unique<Actor>();
		Transform ConeTransform;
		ConeTransform.Position = DirectX::XMFLOAT3(-6.0f + static_cast<float>(ActorIndex) * 6.0f, 0.9f, -4.5f);
		ConeTransform.Scale = DirectX::XMFLOAT3(1.1f, 1.8f, 1.1f);
		ConeTransform.RotationEuler = DirectX::XMFLOAT3(0.0f, static_cast<float>(ActorIndex) * 0.4f, 0.0f);
		ConeActor->SetTransform(ConeTransform);

		std::unique_ptr<MeshUniversalComponent> ConeMeshComponent = std::make_unique<MeshUniversalComponent>();
		ConeMeshComponent->ModelMeshPath = "../../InputResources/Meshes/SimpleCone.fbx";
		ConeMeshComponent->BaseColor = DirectX::XMFLOAT4(
			0.9f - static_cast<float>(ActorIndex) * 0.25f,
			0.35f + static_cast<float>(ActorIndex) * 0.22f,
			0.25f + static_cast<float>(ActorIndex) * 0.2f,
			1.0f);
		ConeMeshComponent->SpecularPower = 32.0f;
		ConeMeshComponent->SpecularIntensity = 0.7f;
		ConeActor->AddComponent(std::move(ConeMeshComponent));
		AddActor(std::move(ConeActor));
	}
}
