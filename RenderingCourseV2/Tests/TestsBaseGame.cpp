#include "Tests/TestsBaseGame.h"
#include "Abstracts/Components/FPSSpectateCameraComponent.h"
#include "Abstracts/Components/LightComponent.h"
#include "Abstracts/Core/Actor.h"
#include "Abstracts/Core/Transform.h"
#include <memory>

TestsBaseGame::TestsBaseGame(LPCWSTR ApplicationName, int ScreenWidth, int ScreenHeight)
	: Game(ApplicationName, ScreenWidth, ScreenHeight)
{
}

TestsBaseGame::~TestsBaseGame() = default;

void TestsBaseGame::BeginPlay()
{
	SpawnDirectionalLightActor(
		DirectX::XMFLOAT3(-0.65f, 0.3f, 0.0f),
		DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		2.3f);
	SpawnFPSSpectateCameraActor(
		DirectX::XMFLOAT3(0.0f, 2.0f, -10.0f),
		DirectX::XMFLOAT3(0.05f, 0.0f, 0.0f),
		1.0f);

	BuildTestScene();
	Game::BeginPlay();
}

Actor* TestsBaseGame::SpawnDirectionalLightActor(
	const DirectX::XMFLOAT3& RotationEuler,
	const DirectX::XMFLOAT4& LightColor,
	float LightIntensity)
{
	std::unique_ptr<Actor> NewLightActor = std::make_unique<Actor>();
	Transform LightTransform;
	LightTransform.RotationEuler = RotationEuler;
	NewLightActor->SetTransform(LightTransform);

	std::unique_ptr<LightComponent> NewLightComponent = std::make_unique<LightComponent>();
	NewLightComponent->SetLightType(LightType::Directional);
	NewLightComponent->SetColor(LightColor);
	NewLightComponent->SetIntensity(LightIntensity);

	Actor* NewLightActorRaw = NewLightActor.get();
	NewLightActor->AddComponent(std::move(NewLightComponent));
	AddActor(std::move(NewLightActor));
	return NewLightActorRaw;
}

Actor* TestsBaseGame::SpawnFPSSpectateCameraActor(
	const DirectX::XMFLOAT3& CameraPosition,
	const DirectX::XMFLOAT3& CameraRotationEuler,
	float CameraMovementSpeedScale)
{
	std::unique_ptr<Actor> NewCameraActor = std::make_unique<Actor>();
	Transform CameraTransform;
	CameraTransform.Position = CameraPosition;
	CameraTransform.RotationEuler = CameraRotationEuler;
	NewCameraActor->SetTransform(CameraTransform);

	std::unique_ptr<FPSSpectateCameraComponent> NewCameraComponent = std::make_unique<FPSSpectateCameraComponent>();
	NewCameraComponent->SetMovementSpeedScale(CameraMovementSpeedScale);

	Actor* NewCameraActorRaw = NewCameraActor.get();
	NewCameraActor->AddComponent(std::move(NewCameraComponent));
	AddActor(std::move(NewCameraActor));
	return NewCameraActorRaw;
}
