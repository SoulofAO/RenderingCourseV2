#include "Planets/PlanetsGame.h"
#include "Planets/OrbitCameraComponent.h"
#include "Planets/PlanetsUIRenderingComponent.h"
#include "Abstracts/Components/CameraComponent.h"
#include "Abstracts/Components/FPSSpectateCameraComponent.h"
#include "Abstracts/Components/MeshUniversalComponent.h"
#include "Abstracts/Components/PhysicsComponent.h"
#include "Abstracts/Core/Actor.h"
#include "Abstracts/Core/Transform.h"
#include "Abstracts/Others/PhysicsLibrary.h"
#include "Abstracts/Subsystems/CameraSubsystem.h"
#include "Abstracts/Subsystems/PhysicsSubsystem.h"
#include <iostream>
#include <algorithm>
#include <array>
#include <cmath>
#include <memory>

PlanetsGame::PlanetsGame(LPCWSTR ApplicationName, int ScreenWidth, int ScreenHeight)
	: Game(ApplicationName, ScreenWidth, ScreenHeight)
	, SunActor(nullptr)
	, OrbitCameraForPlanets(nullptr)
	, FPSCameraComponentForPlanets(nullptr)
	, PlanetsUIRenderingComponentInstance(nullptr)
	, PhysicsCollisionDetectedEventCount(0)
	, UseOrthographicProjectionForActiveCamera(false)
	, UseOrbitCamera(true)
	, TeleportFPSSpectateCameraToOrbitCameraOnSwitch(true)
	, PlanetOrbitSpeedScale(1.0f)
	, MoonOrbitSpeedScale(1.0f)
	, PlanetOrbitRadiusScale(1.0f)
	, MoonOrbitRadiusScale(1.0f)
	, SunSelfRotationSpeed(0.2f)
{
}

PlanetsGame::~PlanetsGame()
{
	PhysicsSubsystem* PhysicsSubsystemInstance = GetSubsystem<PhysicsSubsystem>();
	if (
		PhysicsSubsystemInstance != nullptr &&
		PhysicsCollisionDetectedDelegateHandle.IsValid())
	{
		PhysicsSubsystemInstance->GetOnCollisionDetectedDelegate().Remove(PhysicsCollisionDetectedDelegateHandle);
		PhysicsCollisionDetectedDelegateHandle.Reset();
	}
}

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

bool PlanetsGame::GetUseOrbitCamera() const
{
	return UseOrbitCamera;
}

bool PlanetsGame::GetTeleportFPSSpectateCameraToOrbitCameraOnSwitch() const
{
	return TeleportFPSSpectateCameraToOrbitCameraOnSwitch;
}

void PlanetsGame::SetTeleportFPSSpectateCameraToOrbitCameraOnSwitch(bool NewTeleportFPSSpectateCameraToOrbitCameraOnSwitch)
{
	TeleportFPSSpectateCameraToOrbitCameraOnSwitch = NewTeleportFPSSpectateCameraToOrbitCameraOnSwitch;
}

void PlanetsGame::SetUseOrbitCamera(bool NewUseOrbitCamera)
{
	if (
		NewUseOrbitCamera == false &&
		TeleportFPSSpectateCameraToOrbitCameraOnSwitch &&
		OrbitCameraForPlanets != nullptr &&
		FPSCameraComponentForPlanets != nullptr)
	{
		Actor* OrbitCameraActor = OrbitCameraForPlanets->GetOwningActor();
		Actor* FPSSpectateCameraActor = FPSCameraComponentForPlanets->GetOwningActor();
		if (OrbitCameraActor != nullptr && FPSSpectateCameraActor != nullptr)
		{
			const Transform OrbitCameraWorldTransform = OrbitCameraActor->GetTransform(ETransformSpace::World);
			FPSSpectateCameraActor->SetTransform(OrbitCameraWorldTransform, ETransformSpace::World);
		}
	}

	UseOrbitCamera = NewUseOrbitCamera;

	CameraSubsystem* CameraSystem = GetSubsystem<CameraSubsystem>();
	if (CameraSystem == nullptr)
	{
		return;
	}

	CameraComponent* TargetCameraComponent = UseOrbitCamera
		? static_cast<CameraComponent*>(OrbitCameraForPlanets)
		: static_cast<CameraComponent*>(FPSCameraComponentForPlanets);
	if (TargetCameraComponent == nullptr)
	{
		return;
	}

	if (CameraSystem->GetActiveCamera() == TargetCameraComponent)
	{
		return;
	}

	const int MaximumSwitchAttempts = CameraSystem->GetCameraCount() + 1;
	for (int SwitchAttemptIndex = 0; SwitchAttemptIndex < MaximumSwitchAttempts; ++SwitchAttemptIndex)
	{
		CameraSystem->CycleActiveCamera();
		if (CameraSystem->GetActiveCamera() == TargetCameraComponent)
		{
			return;
		}
	}
}

void PlanetsGame::BeginPlay()
{
	PhysicsSubsystem* PhysicsSubsystemInstance = GetSubsystem<PhysicsSubsystem>();
	if (
		PhysicsSubsystemInstance != nullptr &&
		PhysicsCollisionDetectedDelegateHandle.IsValid() == false)
	{
		PhysicsCollisionDetectedDelegateHandle = PhysicsSubsystemInstance->GetOnCollisionDetectedDelegate().AddRaw(
			this,
			&PlanetsGame::HandlePhysicsCollisionDetected);
	}

	SunActor = CreateCelestialActor(
		"InputResources/Meshes/SimpleSphere.fbx",
		DirectX::XMFLOAT3(2.2f, 2.2f, 2.2f),
		DirectX::XMFLOAT4(1.0f, 0.82f, 0.25f, 1.0f));

	SpawnPlanetsAndMoons();

	std::unique_ptr<Actor> OrbitCameraActor = std::make_unique<Actor>();
	std::unique_ptr<OrbitCameraComponent> NewOrbitCameraComponent = std::make_unique<OrbitCameraComponent>();
	OrbitCameraForPlanets = NewOrbitCameraComponent.get();
	Actor* OrbitCameraTargetActor = GetDefaultOrbitCameraTargetActor();
	OrbitCameraActor->AddComponent(std::move(NewOrbitCameraComponent));
	OrbitCameraForPlanets->SetOrbitTargetActor(OrbitCameraTargetActor);
	AddActor(std::move(OrbitCameraActor));

	std::unique_ptr<Actor> FPSCameraActor = std::make_unique<Actor>();
	Transform FPSCameraTransform;
	FPSCameraTransform.Position = DirectX::XMFLOAT3(0.0f, 3.5f, -20.0f);
	FPSCameraTransform.RotationEuler = DirectX::XMFLOAT3(0.1f, 0.0f, 0.0f);
	FPSCameraActor->SetTransform(FPSCameraTransform);
	std::unique_ptr<FPSSpectateCameraComponent> NewFPSCameraComponent = std::make_unique<FPSSpectateCameraComponent>();
	FPSCameraComponentForPlanets = NewFPSCameraComponent.get();
	FPSCameraActor->AddComponent(std::move(NewFPSCameraComponent));
	AddActor(std::move(FPSCameraActor));

	std::unique_ptr<Actor> PlanetsUIActor = std::make_unique<Actor>();
	std::unique_ptr<PlanetsUIRenderingComponent> NewPlanetsUIRenderingComponent = std::make_unique<PlanetsUIRenderingComponent>();
	PlanetsUIRenderingComponentInstance = NewPlanetsUIRenderingComponent.get();
	PlanetsUIActor->AddComponent(std::move(NewPlanetsUIRenderingComponent));
	AddActor(std::move(PlanetsUIActor));

	Game::BeginPlay();
	SetUseOrbitCamera(UseOrbitCamera);

	UpdateCameraProjectionTypes();
}

void PlanetsGame::Update(float DeltaTime)
{
	UpdatePlanetaryOrbits(DeltaTime);
	UpdateCameraProjectionTypes();
	Game::Update(DeltaTime);
}

void PlanetsGame::HandleCelestialBodySelectionFromInputDevice(int MousePositionX, int MousePositionY)
{
	HandleCelestialBodySelection(MousePositionX, MousePositionY);
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
	std::unique_ptr<PhysicsComponent> NewPhysicsComponent = std::make_unique<PhysicsComponent>();
	NewPhysicsComponent->SetIsStatic(true);
	NewPhysicsComponent->SetUseGravity(false);
	PhysicsComponent* NewPhysicsComponentInstance = NewPhysicsComponent.get();
	Actor* NewActorRaw = NewActor.get();
	NewActor->AddComponent(std::move(NewMeshUniversalComponent));
	NewActor->AddComponent(std::move(NewPhysicsComponent));
	AddActor(std::move(NewActor));
	if (NewPhysicsComponentInstance != nullptr)
	{
		FocusableCelestialPhysicsComponents.push_back(NewPhysicsComponentInstance);
		physx::PxRigidActor* PhysicsActor = NewPhysicsComponentInstance->GetPhysicsActor();
		if (PhysicsActor != nullptr)
		{
			FocusableCelestialBodyMap[PhysicsActor] = NewActorRaw;
		}
	}
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
			? "InputResources/Meshes/SimpleCube.fbx"
			: "InputResources/Meshes/SimpleSphere.fbx";
		const std::string MoonModelMeshPath = ShouldUseCubeMeshForPlanet
			? "InputResources/Meshes/SimpleSphere.fbx"
			: "InputResources/Meshes/SimpleCube.fbx";

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
		NewPlanetMoonOrbitData.PlanetOrbitRadiusMultiplier = 6.0f + (static_cast<float>(PlanetIndex) * 4.0f);
		NewPlanetMoonOrbitData.MoonOrbitRadiusMultiplier = 2.0f + (static_cast<float>(PlanetIndex) * 1.0f);
		PlanetMoonOrbitDataList.push_back(NewPlanetMoonOrbitData);
	}
}

Actor* PlanetsGame::GetDefaultOrbitCameraTargetActor() const
{
	for (const PlanetMoonOrbitData& ExistingPlanetMoonOrbitData : PlanetMoonOrbitDataList)
	{
		if (ExistingPlanetMoonOrbitData.MoonActor != nullptr)
		{
			return ExistingPlanetMoonOrbitData.MoonActor;
		}
	}

	for (const PlanetMoonOrbitData& ExistingPlanetMoonOrbitData : PlanetMoonOrbitDataList)
	{
		if (ExistingPlanetMoonOrbitData.PlanetActor != nullptr)
		{
			return ExistingPlanetMoonOrbitData.PlanetActor;
		}
	}

	return SunActor;
}

void PlanetsGame::UpdatePlanetaryOrbits(float DeltaTime)
{
	if (SunActor != nullptr)
	{
		Transform SunWorldTransform = SunActor->GetTransform();
		SunWorldTransform.RotationEuler.y += SunSelfRotationSpeed * DeltaTime;
		SunActor->SetTransform(SunWorldTransform);
	}

	for (PlanetMoonOrbitData& ExistingPlanetMoonOrbitData : PlanetMoonOrbitDataList)
	{
		if (ExistingPlanetMoonOrbitData.PlanetActor != nullptr)
		{
			ExistingPlanetMoonOrbitData.PlanetOrbitAngle += ExistingPlanetMoonOrbitData.PlanetOrbitSpeed * PlanetOrbitSpeedScale * DeltaTime;
			const float PlanetOrbitRadius = ExistingPlanetMoonOrbitData.PlanetOrbitRadiusMultiplier * PlanetOrbitRadiusScale;
			const float PlanetPositionX = std::cos(ExistingPlanetMoonOrbitData.PlanetOrbitAngle) * PlanetOrbitRadius;
			const float PlanetPositionZ = std::sin(ExistingPlanetMoonOrbitData.PlanetOrbitAngle) * PlanetOrbitRadius;

			Transform PlanetLocalTransform = ExistingPlanetMoonOrbitData.PlanetActor->GetTransform(ETransformSpace::Local);
			PlanetLocalTransform.Position = DirectX::XMFLOAT3(PlanetPositionX, 0.0f, PlanetPositionZ);
			PlanetLocalTransform.RotationEuler.y += ExistingPlanetMoonOrbitData.PlanetSelfRotationSpeed * DeltaTime;
			ExistingPlanetMoonOrbitData.PlanetActor->SetTransform(PlanetLocalTransform, ETransformSpace::Local);
		}

		if (ExistingPlanetMoonOrbitData.MoonActor != nullptr)
		{
			ExistingPlanetMoonOrbitData.MoonOrbitAngle += ExistingPlanetMoonOrbitData.MoonOrbitSpeed * MoonOrbitSpeedScale * DeltaTime;
			const float MoonOrbitRadius = ExistingPlanetMoonOrbitData.MoonOrbitRadiusMultiplier * MoonOrbitRadiusScale;
			const float MoonPositionX = std::cos(ExistingPlanetMoonOrbitData.MoonOrbitAngle) * MoonOrbitRadius;
			const float MoonPositionZ = std::sin(ExistingPlanetMoonOrbitData.MoonOrbitAngle) * MoonOrbitRadius;

			Transform MoonLocalTransform = ExistingPlanetMoonOrbitData.MoonActor->GetTransform(ETransformSpace::Local);
			MoonLocalTransform.Position = DirectX::XMFLOAT3(MoonPositionX, 0.0f, MoonPositionZ);
			MoonLocalTransform.RotationEuler.y += ExistingPlanetMoonOrbitData.MoonSelfRotationSpeed * DeltaTime;
			ExistingPlanetMoonOrbitData.MoonActor->SetTransform(MoonLocalTransform, ETransformSpace::Local);
		}
	}
}

void PlanetsGame::UpdateCameraProjectionTypes()
{
	const CameraProjectionType NewProjectionType = UseOrthographicProjectionForActiveCamera
		? CameraProjectionType::Orthographic
		: CameraProjectionType::Perspective;

	if (FPSCameraComponentForPlanets != nullptr)
	{
		FPSCameraComponentForPlanets->SetProjectionType(NewProjectionType);
	}

	if (OrbitCameraForPlanets != nullptr)
	{
		OrbitCameraForPlanets->SetProjectionType(NewProjectionType);
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

void PlanetsGame::HandleCelestialBodySelection(int MousePositionX, int MousePositionY)
{
	if (OrbitCameraForPlanets == nullptr)
	{
		return;
	}

	DirectX::XMFLOAT3 TraceStart;
	DirectX::XMFLOAT3 TraceDirection;
	const bool HasValidTraceRay = PhysicsLibrary::BuildLineTraceFromMousePosition(
		MousePositionX,
		MousePositionY,
		TraceStart,
		TraceDirection);
	if (HasValidTraceRay == false)
	{
		return;
	}

	PhysicsLineTraceHitResult HitResult;
	const float TraceLength = 5000.0f;
	const bool HasLineTraceHit = PhysicsLibrary::LineTrace(
		TraceStart,
		TraceDirection,
		TraceLength,
		HitResult);
	if (HasLineTraceHit == false || HitResult.HitActor == nullptr)
	{
		return;
	}

	RefreshFocusableCelestialBodyMap();
	const auto FocusedBodyIterator = FocusableCelestialBodyMap.find(HitResult.HitActor);
	if (FocusedBodyIterator == FocusableCelestialBodyMap.end() || FocusedBodyIterator->second == nullptr)
	{
		return;
	}

	OrbitCameraForPlanets->SetOrbitTargetActor(FocusedBodyIterator->second);
}

void PlanetsGame::RefreshFocusableCelestialBodyMap()
{
	FocusableCelestialBodyMap.clear();
	for (PhysicsComponent* ExistingPhysicsComponent : FocusableCelestialPhysicsComponents)
	{
		if (ExistingPhysicsComponent == nullptr)
		{
			continue;
		}

		Actor* ExistingActor = ExistingPhysicsComponent->GetOwningActor();
		if (ExistingActor == nullptr)
		{
			continue;
		}

		physx::PxRigidActor* ExistingPhysicsActor = ExistingPhysicsComponent->GetPhysicsActor();
		if (ExistingPhysicsActor == nullptr)
		{
			continue;
		}

		FocusableCelestialBodyMap[ExistingPhysicsActor] = ExistingActor;
	}
}

void PlanetsGame::HandlePhysicsCollisionDetected(
	PhysicsComponent* FirstPhysicsComponent,
	PhysicsComponent* SecondPhysicsComponent,
	const DirectX::XMFLOAT3& CollisionNormal,
	float CollisionPenetrationDepth)
{
	PhysicsCollisionDetectedEventCount += 1;
	std::cout
		<< "Collision Event #" << PhysicsCollisionDetectedEventCount
		<< " FirstComponent: " << FirstPhysicsComponent
		<< " SecondComponent: " << SecondPhysicsComponent
		<< " Normal(" << CollisionNormal.x << ", " << CollisionNormal.y << ", " << CollisionNormal.z << ")"
		<< " PenetrationDepth: " << CollisionPenetrationDepth
		<< std::endl;
}
