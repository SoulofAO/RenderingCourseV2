#include "Planets/PlanetsGame.h"
#include "Planets/PlanetsUIRenderingComponent.h"
#include "Abstracts/Components/CameraComponent.h"
#include "Abstracts/Components/MeshUniversalComponent.h"
#include "Abstracts/Core/Actor.h"
#include "Abstracts/Core/Transform.h"
#include "Abstracts/Subsystems/CameraSubsystem.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <memory>

PlanetsGame::PlanetsGame(LPCWSTR ApplicationName, int ScreenWidth, int ScreenHeight)
	: Game(ApplicationName, ScreenWidth, ScreenHeight)
	, SunActor(nullptr)
	, OrbitCameraPivotActor(nullptr)
	, OrbitCameraComponent(nullptr)
	, FallbackCameraComponentForPlanets(nullptr)
	, PlanetsUIRenderingComponentInstance(nullptr)
	, UseOrthographicProjectionForActiveCamera(false)
	, PlanetOrbitSpeedScale(1.0f)
	, MoonOrbitSpeedScale(1.0f)
	, PlanetOrbitRadiusScale(1.0f)
	, MoonOrbitRadiusScale(1.0f)
	, OrbitCameraYawSpeed(0.25f)
	, SunSelfRotationSpeed(0.2f)
{
}

PlanetsGame::~PlanetsGame() = default;

bool PlanetsGame::GetUseOrthographicProjectionForActiveCamera() const
{
	return UseOrthographicProjectionForActiveCamera;
}

void PlanetsGame::SetUseOrthographicProjectionForActiveCamera(bool NewUseOrthographicProjectionForActiveCamera)
{
	UseOrthographicProjectionForActiveCamera = NewUseOrthographicProjectionForActiveCamera;
}

float PlanetsGame::GetPlanetOrbitSpeedScale() const
{
	return PlanetOrbitSpeedScale;
}

void PlanetsGame::SetPlanetOrbitSpeedScale(float NewPlanetOrbitSpeedScale)
{
	SetOrbitSpeedScaleValues(NewPlanetOrbitSpeedScale, MoonOrbitSpeedScale);
}

float PlanetsGame::GetMoonOrbitSpeedScale() const
{
	return MoonOrbitSpeedScale;
}

void PlanetsGame::SetMoonOrbitSpeedScale(float NewMoonOrbitSpeedScale)
{
	SetOrbitSpeedScaleValues(PlanetOrbitSpeedScale, NewMoonOrbitSpeedScale);
}

float PlanetsGame::GetPlanetOrbitRadiusScale() const
{
	return PlanetOrbitRadiusScale;
}

void PlanetsGame::SetPlanetOrbitRadiusScale(float NewPlanetOrbitRadiusScale)
{
	SetOrbitRadiusScaleValues(NewPlanetOrbitRadiusScale, MoonOrbitRadiusScale);
}

float PlanetsGame::GetMoonOrbitRadiusScale() const
{
	return MoonOrbitRadiusScale;
}

void PlanetsGame::SetMoonOrbitRadiusScale(float NewMoonOrbitRadiusScale)
{
	SetOrbitRadiusScaleValues(PlanetOrbitRadiusScale, NewMoonOrbitRadiusScale);
}

void PlanetsGame::BeginPlay()
{
	SunActor = CreateCelestialActor(
		"G:/RenderingCourseV2/InputResources/Meshes/SimpleSphere.fbx",
		DirectX::XMFLOAT3(2.2f, 2.2f, 2.2f),
		DirectX::XMFLOAT4(1.0f, 0.82f, 0.25f, 1.0f));

	SpawnPlanetsAndMoons();

	OrbitCameraPivotActor = CreateCelestialActor(
		"G:/RenderingCourseV2/InputResources/Meshes/SimpleSphere.fbx",
		DirectX::XMFLOAT3(0.001f, 0.001f, 0.001f),
		DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f));
	if (OrbitCameraPivotActor != nullptr && SunActor != nullptr)
	{
		OrbitCameraPivotActor->AttachToActor(SunActor);
	}

	std::unique_ptr<Actor> OrbitCameraActor = std::make_unique<Actor>();
	Transform OrbitCameraTransform;
	OrbitCameraTransform.Position = DirectX::XMFLOAT3(0.0f, 8.0f, -24.0f);
	OrbitCameraTransform.RotationEuler = DirectX::XMFLOAT3(0.25f, 0.0f, 0.0f);
	OrbitCameraActor->SetTransform(OrbitCameraTransform);
	std::unique_ptr<CameraComponent> NewOrbitCameraComponent = std::make_unique<CameraComponent>();
	OrbitCameraComponent = NewOrbitCameraComponent.get();
	OrbitCameraActor->AddComponent(std::move(NewOrbitCameraComponent));
	if (OrbitCameraPivotActor != nullptr)
	{
		OrbitCameraActor->AttachToActor(OrbitCameraPivotActor);
	}
	AddActor(std::move(OrbitCameraActor));

	std::unique_ptr<Actor> PlanetsUIActor = std::make_unique<Actor>();
	std::unique_ptr<PlanetsUIRenderingComponent> NewPlanetsUIRenderingComponent = std::make_unique<PlanetsUIRenderingComponent>();
	PlanetsUIRenderingComponentInstance = NewPlanetsUIRenderingComponent.get();
	PlanetsUIActor->AddComponent(std::move(NewPlanetsUIRenderingComponent));
	AddActor(std::move(PlanetsUIActor));

	Game::BeginPlay();

	FallbackCameraComponentForPlanets = FallbackCameraComponentInstance;
	if (FallbackCameraComponentForPlanets != nullptr)
	{
		Actor* FallbackCameraActorInstance = FallbackCameraComponentForPlanets->GetOwningActor();
		if (FallbackCameraActorInstance != nullptr)
		{
			Transform FallbackCameraTransform = FallbackCameraActorInstance->GetLocalTransform();
			FallbackCameraTransform.Position = DirectX::XMFLOAT3(0.0f, 3.5f, -20.0f);
			FallbackCameraTransform.RotationEuler = DirectX::XMFLOAT3(0.1f, 0.0f, 0.0f);
			FallbackCameraActorInstance->SetTransform(FallbackCameraTransform);
		}
	}

	UpdateCameraProjectionTypes();
}

void PlanetsGame::Update(float DeltaTime)
{
	UpdatePlanetaryOrbits(DeltaTime);
	UpdateCameraProjectionTypes();
	Game::Update(DeltaTime);
}

LRESULT PlanetsGame::MessageHandler(HWND WindowHandle, UINT Message, WPARAM WParam, LPARAM LParam)
{
	if (PlanetsUIRenderingComponentInstance != nullptr)
	{
		if (PlanetsUIRenderingComponentInstance->HandleMessage(WindowHandle, Message, WParam, LParam))
		{
			return 1;
		}
	}

	return Game::MessageHandler(WindowHandle, Message, WParam, LParam);
}

Actor* PlanetsGame::CreateCelestialActor(
	const std::string& ModelMeshPath,
	const DirectX::XMFLOAT3& ActorScale,
	const DirectX::XMFLOAT4& ActorColor)
{
	std::unique_ptr<Actor> NewActor = std::make_unique<Actor>();
	Transform ActorTransform;
	ActorTransform.Position = DirectX::XMFLOAT3(0.0f, 0.0f, 4.0f);
	ActorTransform.Scale = ActorScale;
	NewActor->SetTransform(ActorTransform);

	std::unique_ptr<MeshUniversalComponent> NewMeshUniversalComponent = std::make_unique<MeshUniversalComponent>();
	NewMeshUniversalComponent->ModelMeshPath = ModelMeshPath;
	NewMeshUniversalComponent->BaseColor = ActorColor;
	Actor* NewActorRaw = NewActor.get();
	NewActor->AddComponent(std::move(NewMeshUniversalComponent));
	AddActor(std::move(NewActor));
	return NewActorRaw;
}

void PlanetsGame::SpawnPlanetsAndMoons()
{
	PlanetMoonOrbitDataList.clear();

	const std::array<DirectX::XMFLOAT4, 5> PlanetColors =
	{
		DirectX::XMFLOAT4(0.20f, 0.50f, 1.00f, 1.0f),
		DirectX::XMFLOAT4(0.95f, 0.45f, 0.20f, 1.0f),
		DirectX::XMFLOAT4(0.35f, 0.90f, 0.40f, 1.0f),
		DirectX::XMFLOAT4(0.60f, 0.50f, 1.00f, 1.0f),
		DirectX::XMFLOAT4(0.95f, 0.85f, 0.30f, 1.0f)
	};
	const std::array<DirectX::XMFLOAT4, 5> MoonColors =
	{
		DirectX::XMFLOAT4(0.70f, 0.72f, 0.78f, 1.0f),
		DirectX::XMFLOAT4(0.65f, 0.68f, 0.76f, 1.0f),
		DirectX::XMFLOAT4(0.72f, 0.74f, 0.80f, 1.0f),
		DirectX::XMFLOAT4(0.78f, 0.70f, 0.66f, 1.0f),
		DirectX::XMFLOAT4(0.70f, 0.77f, 0.73f, 1.0f)
	};

	for (size_t PlanetIndex = 0; PlanetIndex < PlanetColors.size(); ++PlanetIndex)
	{
		const bool ShouldUseCubeMeshForPlanet = (PlanetIndex % 2) == 0;
		const std::string PlanetModelMeshPath = ShouldUseCubeMeshForPlanet
			? "G:/RenderingCourseV2/InputResources/Meshes/SimpleCube.fbx"
			: "G:/RenderingCourseV2/InputResources/Meshes/SimpleSphere.fbx";
		const std::string MoonModelMeshPath = ShouldUseCubeMeshForPlanet
			? "G:/RenderingCourseV2/InputResources/Meshes/SimpleSphere.fbx"
			: "G:/RenderingCourseV2/InputResources/Meshes/SimpleCube.fbx";

		const float PlanetScaleValue = 0.55f + (static_cast<float>(PlanetIndex) * 0.08f);
		Actor* PlanetActor = CreateCelestialActor(
			PlanetModelMeshPath,
			DirectX::XMFLOAT3(PlanetScaleValue, PlanetScaleValue, PlanetScaleValue),
			PlanetColors[PlanetIndex]);
		if (PlanetActor != nullptr && SunActor != nullptr)
		{
			PlanetActor->AttachToActor(SunActor);
		}

		const float MoonScaleValue = 0.22f + (static_cast<float>(PlanetIndex) * 0.02f);
		Actor* MoonActor = CreateCelestialActor(
			MoonModelMeshPath,
			DirectX::XMFLOAT3(MoonScaleValue, MoonScaleValue, MoonScaleValue),
			MoonColors[PlanetIndex]);
		if (MoonActor != nullptr && PlanetActor != nullptr)
		{
			MoonActor->AttachToActor(PlanetActor);
		}

		PlanetMoonOrbitData NewPlanetMoonOrbitData;
		NewPlanetMoonOrbitData.PlanetActor = PlanetActor;
		NewPlanetMoonOrbitData.MoonActor = MoonActor;
		NewPlanetMoonOrbitData.PlanetOrbitAngle = static_cast<float>(PlanetIndex) * 1.1f;
		NewPlanetMoonOrbitData.MoonOrbitAngle = static_cast<float>(PlanetIndex) * 2.0f;
		NewPlanetMoonOrbitData.PlanetOrbitSpeed = 0.15f + (static_cast<float>(PlanetIndex) * 0.08f);
		NewPlanetMoonOrbitData.MoonOrbitSpeed = 0.65f + (static_cast<float>(PlanetIndex) * 0.14f);
		NewPlanetMoonOrbitData.PlanetSelfRotationSpeed = 0.45f + (static_cast<float>(PlanetIndex) * 0.10f);
		NewPlanetMoonOrbitData.MoonSelfRotationSpeed = 0.90f + (static_cast<float>(PlanetIndex) * 0.14f);
		NewPlanetMoonOrbitData.PlanetOrbitRadiusMultiplier = 3.0f + (static_cast<float>(PlanetIndex) * 1.25f);
		NewPlanetMoonOrbitData.MoonOrbitRadiusMultiplier = 0.7f + (static_cast<float>(PlanetIndex) * 0.12f);
		PlanetMoonOrbitDataList.push_back(NewPlanetMoonOrbitData);
	}
}

void PlanetsGame::UpdatePlanetaryOrbits(float DeltaTime)
{
	if (SunActor != nullptr)
	{
		Transform SunLocalTransform = SunActor->GetLocalTransform();
		SunLocalTransform.RotationEuler.y += SunSelfRotationSpeed * DeltaTime;
		SunActor->SetTransform(SunLocalTransform);
	}

	if (OrbitCameraPivotActor != nullptr)
	{
		Transform OrbitCameraPivotLocalTransform = OrbitCameraPivotActor->GetLocalTransform();
		OrbitCameraPivotLocalTransform.RotationEuler.y += OrbitCameraYawSpeed * DeltaTime;
		OrbitCameraPivotActor->SetTransform(OrbitCameraPivotLocalTransform);
	}

	for (PlanetMoonOrbitData& ExistingPlanetMoonOrbitData : PlanetMoonOrbitDataList)
	{
		if (ExistingPlanetMoonOrbitData.PlanetActor != nullptr)
		{
			ExistingPlanetMoonOrbitData.PlanetOrbitAngle += ExistingPlanetMoonOrbitData.PlanetOrbitSpeed * PlanetOrbitSpeedScale * DeltaTime;
			const float PlanetOrbitRadius = ExistingPlanetMoonOrbitData.PlanetOrbitRadiusMultiplier * PlanetOrbitRadiusScale;
			const float PlanetPositionX = std::cos(ExistingPlanetMoonOrbitData.PlanetOrbitAngle) * PlanetOrbitRadius;
			const float PlanetPositionZ = std::sin(ExistingPlanetMoonOrbitData.PlanetOrbitAngle) * PlanetOrbitRadius;

			Transform PlanetLocalTransform = ExistingPlanetMoonOrbitData.PlanetActor->GetLocalTransform();
			PlanetLocalTransform.Position = DirectX::XMFLOAT3(PlanetPositionX, 0.0f, PlanetPositionZ);
			PlanetLocalTransform.RotationEuler.y += ExistingPlanetMoonOrbitData.PlanetSelfRotationSpeed * DeltaTime;
			ExistingPlanetMoonOrbitData.PlanetActor->SetTransform(PlanetLocalTransform);
		}

		if (ExistingPlanetMoonOrbitData.MoonActor != nullptr)
		{
			ExistingPlanetMoonOrbitData.MoonOrbitAngle += ExistingPlanetMoonOrbitData.MoonOrbitSpeed * MoonOrbitSpeedScale * DeltaTime;
			const float MoonOrbitRadius = ExistingPlanetMoonOrbitData.MoonOrbitRadiusMultiplier * MoonOrbitRadiusScale;
			const float MoonPositionX = std::cos(ExistingPlanetMoonOrbitData.MoonOrbitAngle) * MoonOrbitRadius;
			const float MoonPositionZ = std::sin(ExistingPlanetMoonOrbitData.MoonOrbitAngle) * MoonOrbitRadius;

			Transform MoonLocalTransform = ExistingPlanetMoonOrbitData.MoonActor->GetLocalTransform();
			MoonLocalTransform.Position = DirectX::XMFLOAT3(MoonPositionX, 0.0f, MoonPositionZ);
			MoonLocalTransform.RotationEuler.y += ExistingPlanetMoonOrbitData.MoonSelfRotationSpeed * DeltaTime;
			ExistingPlanetMoonOrbitData.MoonActor->SetTransform(MoonLocalTransform);
		}
	}
}

void PlanetsGame::UpdateCameraProjectionTypes()
{
	const CameraProjectionType NewProjectionType = UseOrthographicProjectionForActiveCamera
		? CameraProjectionType::Orthographic
		: CameraProjectionType::Perspective;

	if (FallbackCameraComponentForPlanets != nullptr)
	{
		FallbackCameraComponentForPlanets->SetProjectionType(NewProjectionType);
	}

	if (OrbitCameraComponent != nullptr)
	{
		OrbitCameraComponent->SetProjectionType(NewProjectionType);
	}
}

void PlanetsGame::SetOrbitSpeedScaleValues(float NewPlanetOrbitSpeedScale, float NewMoonOrbitSpeedScale)
{
	PlanetOrbitSpeedScale = (std::max)(0.0f, NewPlanetOrbitSpeedScale);
	MoonOrbitSpeedScale = (std::max)(0.0f, NewMoonOrbitSpeedScale);
}

void PlanetsGame::SetOrbitRadiusScaleValues(float NewPlanetOrbitRadiusScale, float NewMoonOrbitRadiusScale)
{
	PlanetOrbitRadiusScale = (std::max)(0.15f, NewPlanetOrbitRadiusScale);
	MoonOrbitRadiusScale = (std::max)(0.15f, NewMoonOrbitRadiusScale);
}
