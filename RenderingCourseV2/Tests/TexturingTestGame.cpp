#include "Tests/TexturingTestGame.h"
#include "Abstracts/Components/MeshUniversalComponent.h"
#include "Abstracts/Core/Actor.h"
#include "Abstracts/Core/Transform.h"
#include <memory>

TexturingTestGame::TexturingTestGame(LPCWSTR ApplicationName, int ScreenWidth, int ScreenHeight)
	: TestsBaseGame(ApplicationName, ScreenWidth, ScreenHeight)
{
}

TexturingTestGame::~TexturingTestGame() = default;

void TexturingTestGame::BuildTestScene()
{
	std::unique_ptr<Actor> FloorActor = std::make_unique<Actor>();
	Transform FloorTransform;
	FloorTransform.Position = DirectX::XMFLOAT3(0.0f, -1.8f, 0.0f);
	FloorTransform.Scale = DirectX::XMFLOAT3(25.0f, 1.0f, 25.0f);
	FloorActor->SetTransform(FloorTransform);
	std::unique_ptr<MeshUniversalComponent> FloorMeshComponent = std::make_unique<MeshUniversalComponent>();
	FloorMeshComponent->ModelMeshPath = "../../InputResources/Meshes/SimpleCube.fbx";
	FloorMeshComponent->BaseColor = DirectX::XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	FloorMeshComponent->AlbedoTexturePath = "../../InputResources/Textures/TestTexture.png";
	FloorMeshComponent->SpecularPower = 24.0f;
	FloorMeshComponent->SpecularIntensity = 0.3f;
	FloorActor->AddComponent(std::move(FloorMeshComponent));
	AddActor(std::move(FloorActor));

	for (int ActorIndex = 0; ActorIndex < 5; ++ActorIndex)
	{
		std::unique_ptr<Actor> MeshActor = std::make_unique<Actor>();
		Transform MeshTransform;
		MeshTransform.Position = DirectX::XMFLOAT3(-8.0f + static_cast<float>(ActorIndex) * 4.0f, 0.8f, 1.2f);
		MeshTransform.Scale = DirectX::XMFLOAT3(1.15f, 1.15f, 1.15f);
		MeshActor->SetTransform(MeshTransform);

		std::unique_ptr<MeshUniversalComponent> MeshComponent = std::make_unique<MeshUniversalComponent>();
		MeshComponent->ModelMeshPath = (ActorIndex % 2 == 0)
			? "../../InputResources/Meshes/SimpleCube.fbx"
			: "../../InputResources/Meshes/SimpleSphere.fbx";
		MeshComponent->AlbedoTexturePath = "../../InputResources/Textures/TestTexture.png";
		MeshComponent->BaseColor = DirectX::XMFLOAT4(
			1.0f - static_cast<float>(ActorIndex) * 0.14f,
			0.75f + static_cast<float>(ActorIndex) * 0.04f,
			0.7f - static_cast<float>(ActorIndex) * 0.12f,
			1.0f);
		MeshComponent->SpecularPower = 8.0f + static_cast<float>(ActorIndex) * 18.0f;
		MeshComponent->SpecularIntensity = 0.2f + static_cast<float>(ActorIndex) * 0.16f;
		if (ActorIndex >= 3)
		{
			MeshComponent->NormalTexturePath = "../../InputResources/Textures/TestTexture.png";
		}

		MeshActor->AddComponent(std::move(MeshComponent));
		AddActor(std::move(MeshActor));
	}
}
