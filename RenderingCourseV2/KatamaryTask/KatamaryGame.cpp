#include "KatamaryTask/KatamaryGame.h"
#include "KatamaryTask/KatamaryUIRenderingComponent.h"
#include "KatamaryTask/KatamaryOrbitCameraComponent.h"
#include "Abstracts/Components/LightComponent.h"
#include "Abstracts/Components/MeshUniversalComponent.h"
#include "Abstracts/Components/PhysicsComponent.h"
#include "Abstracts/Core/Actor.h"
#include "Abstracts/Core/Transform.h"
#include "Abstracts/Others/PhysicsLibrary.h"
#include "Abstracts/Subsystems/CameraSubsystem.h"
#include "Abstracts/Subsystems/PhysicsSubsystem.h"
#include "Abstracts/Subsystems/SceneViewportSubsystem.h"
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <memory>

static constexpr std::uint32_t PhysicsCollisionLayerPlayer = 1u << 1u;
static constexpr std::uint32_t PhysicsCollisionLayerCollectibleUncollected = 1u << 2u;
static constexpr std::uint32_t PhysicsCollisionLayerCollectibleCollected = 1u << 3u;
static constexpr std::uint32_t PhysicsCollisionMaskPlayer = 0xFFFFFFFFu & ~PhysicsCollisionLayerCollectibleCollected;
static constexpr std::uint32_t PhysicsCollisionMaskCollectibleUncollected = 0xFFFFFFFFu;
static constexpr std::uint32_t PhysicsCollisionMaskCollectibleCollectedWeld =
	0xFFFFFFFFu & ~PhysicsCollisionLayerPlayer & ~PhysicsCollisionLayerCollectibleCollected;

KatamaryGame::KatamaryGame(LPCWSTR ApplicationName, int ScreenWidth, int ScreenHeight)
	: Game(ApplicationName, ScreenWidth, ScreenHeight)
	, PlayerActor(nullptr)
	, PlayerPhysicsComponent(nullptr)
	, GameplayCameraComponent(nullptr)
	, KatamaryUIRenderingComponentInstance(nullptr)
	, RandomNumberGenerator(std::random_device{}())
	, BasePlayerSphereColliderRadius(1.0f)
	, PlayerSphereColliderGrowthPerCollectible(0.08f)
	, PlayerMoveForce(120.0f)
	, PlayerJumpImpulse(400.0f)
	, PlayerJumpTraceExtraDistance(0.5f)
	, PlayerMaximumPlanarSpeed(9.0f)
	, LightProjectileLifetimeSeconds(4.0f)
	, LightProjectileSpeed(20.0f)
	, LightProjectileSpawnOffsetDistance(2.0f)
	, LightProjectileVisualScale(0.25f)
	, LightProjectileLightRange(8.0f)
	, LightProjectileLightIntensity(4.0f)
	, RoundDurationSeconds(60.0f)
	, RemainingTimeSeconds(60.0f)
	, MaximumActiveLightProjectileCount(10)
	, IsRoundFinished(false)
	, UseWeldCollectMode(false)
	, CollectedItemCount(0)
	, SpawnedCollectibleCount(15)
	,GlobalCollectibleMeshScale(0.5)
{
	
	CollectibleMeshPaths.push_back(MeshLocalData(
		"InputResources/Meshes/Katamary/Crate.fbx",
		"InputResources/Textures/Katamary/Crate/Crate_Albedo.png",
		DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		"InputResources/Textures/Katamary/Crate/Crate_Metalness.png",
		"InputResources/Textures/Katamary/Crate/Crate_Roughness.png",
		"InputResources/Textures/Katamary/Crate/Crate_Normal.png"));

	CollectibleMeshPaths.push_back(MeshLocalData(
		"InputResources/Meshes/Katamary/grass.fbx",
		"",
		DirectX::XMFLOAT4(0.36f, 0.79f, 0.41f, 1.0f),
		"",
		"",
		""));

	CollectibleMeshPaths.push_back(MeshLocalData(
		"InputResources/Meshes/Katamary/Tree.FBX",
		"InputResources/Textures/Katamary/Tree/Tree_Diffuse.png",
		DirectX::XMFLOAT4(0.65f, 0.64f, 0.58f, 1.0f),
		"InputResources/Textures/Katamary/Tree/Tree_Metalness.png",
		"InputResources/Textures/Katamary/Tree/Tree_Roughness.png",
		"InputResources/Textures/Katamary/Tree/Tree_normal.png"));

	CollectibleMeshPaths.push_back(MeshLocalData(
		"InputResources/Meshes/SimpleSphere.fbx",
		"",
		DirectX::XMFLOAT4(0.79f, 0.37f, 0.96f, 1.0f),
		"",
		"",
		""));

	CollectibleMeshPaths.push_back(MeshLocalData(
		"InputResources/Meshes/SimpleCube.fbx",
		"",
		DirectX::XMFLOAT4(0.74f, 0.83f, 0.91f, 1.0f),
		"",
		"",
		""));

	CollectibleMeshPaths.push_back(MeshLocalData(
		"InputResources/Meshes/SimpleCone.fbx",
		"",
		DirectX::XMFLOAT4(0.93f, 0.73f, 0.39f, 1.0f),
		"",
		"",
		""));
}

KatamaryGame::~KatamaryGame()
{
	PhysicsSubsystem* PhysicsSubsystemInstance = GetSubsystem<PhysicsSubsystem>();
	if (
		PhysicsSubsystemInstance != nullptr &&
		CollisionDetectedDelegateHandle.IsValid())
	{
		PhysicsSubsystemInstance->GetOnCollisionDetectedDelegate().Remove(CollisionDetectedDelegateHandle);
		CollisionDetectedDelegateHandle.Reset();
	}
}

int KatamaryGame::GetCollectedItemCount() const
{
	return CollectedItemCount;
}

float KatamaryGame::GetRemainingTimeSeconds() const
{
	return RemainingTimeSeconds;
}

bool KatamaryGame::GetIsRoundFinished() const
{
	return IsRoundFinished;
}

bool KatamaryGame::GetUseWeldCollectMode() const
{
	return UseWeldCollectMode;
}

void KatamaryGame::SetUseWeldCollectMode(bool NewUseWeldCollectMode)
{
	UseWeldCollectMode = NewUseWeldCollectMode;
	UpdatePlayerCollisionSphereRadius();
}

void KatamaryGame::BeginPlay()
{
	SetMouseInputMode(MouseInputMode::UIOnly);
	BuildScene();
	SceneViewportSubsystem* SceneViewportSubsystemInstance = GetSubsystem<SceneViewportSubsystem>();
	if (SceneViewportSubsystemInstance != nullptr)
	{
		SceneViewportSubsystemInstance->SetRenderPipelineType(RenderPipelineType::Deferred);
	}
	Game::BeginPlay();
}

void KatamaryGame::Update(float DeltaTime)
{
	HandleRoundTimer(DeltaTime);
	ProcessPendingCollectibleAttachments();
	UpdateLightProjectiles(DeltaTime);
	UpdateGameplayCamera();
	Game::Update(DeltaTime);
}

LRESULT KatamaryGame::MessageHandler(HWND WindowHandle, UINT Message, WPARAM WParam, LPARAM LParam)
{
	if (KatamaryUIRenderingComponentInstance != nullptr)
	{
		if (KatamaryUIRenderingComponentInstance->HandleMessage(WindowHandle, Message, WParam, LParam))
		{
			return 1;
		}
	}

	return Game::MessageHandler(WindowHandle, Message, WParam, LParam);
}

void KatamaryGame::BuildScene()
{
	PhysicsSubsystem* PhysicsSubsystemInstance = GetSubsystem<PhysicsSubsystem>();
	if (PhysicsSubsystemInstance != nullptr)
	{
		PhysicsSubsystemInstance->SetFixedDeltaTime(1.0f / 90.0f);
		if (CollisionDetectedDelegateHandle.IsValid())
		{
			PhysicsSubsystemInstance->GetOnCollisionDetectedDelegate().Remove(CollisionDetectedDelegateHandle);
			CollisionDetectedDelegateHandle.Reset();
		}
		CollisionDetectedDelegateHandle = PhysicsSubsystemInstance->GetOnCollisionDetectedDelegate().AddRaw(this, &KatamaryGame::HandlePhysicsCollisionDetected);
	}

	SpawnFloor();
	SpawnDirectionalLight();
	SpawnPlayer();
	SpawnGameplayCamera();
	SpawnCollectibles();
	SpawnUserInterface();
}

void KatamaryGame::SpawnDirectionalLight()
{
	std::unique_ptr<Actor> DirectionalLightActor = std::make_unique<Actor>();
	Transform DirectionalLightTransform;
	DirectionalLightTransform.RotationEuler = DirectX::XMFLOAT3(1.0f, 0.5f, 0.0f);
	DirectionalLightActor->SetTransform(DirectionalLightTransform);

	std::unique_ptr<LightComponent> DirectionalLightComponent = std::make_unique<LightComponent>();
	DirectionalLightComponent->SetLightType(LightType::Directional);
	DirectionalLightComponent->SetColor(DirectX::XMFLOAT4(1.0f, 0.96f, 0.9f, 1.0f));
	DirectionalLightComponent->SetIntensity(2.2f);

	DirectionalLightActor->AddComponent(std::move(DirectionalLightComponent));
	AddActor(std::move(DirectionalLightActor));
}

void KatamaryGame::SpawnGameplayCamera()
{
	std::unique_ptr<Actor> CameraActor = std::make_unique<Actor>();
	Transform CameraTransform;
	CameraTransform.Position = DirectX::XMFLOAT3(-8.0f, 7.0f, -8.0f);
	CameraTransform.RotationEuler = DirectX::XMFLOAT3(75.0f, 15.78f, 0.0f);
	CameraActor->SetTransform(CameraTransform);

	std::unique_ptr<KatamaryOrbitCameraComponent> CameraComponentInstance = std::make_unique<KatamaryOrbitCameraComponent>();
	CameraComponentInstance->SetFieldOfViewDegrees(65.0f);
	GameplayCameraComponent = CameraComponentInstance.get();
	CameraComponentInstance->SetOrbitTargetActor(PlayerActor);
	
	CameraActor->AddComponent(std::move(CameraComponentInstance));
	AddActor(std::move(CameraActor));
}

void KatamaryGame::SpawnFloor()
{
	std::unique_ptr<Actor> FloorActor = std::make_unique<Actor>();
	Transform FloorTransform;
	FloorTransform.Position = DirectX::XMFLOAT3(0.0f, -2.0f, 0.0f);
	FloorTransform.Scale = DirectX::XMFLOAT3(4.0f, 1.0f, 4.0f);
	FloorActor->SetTransform(FloorTransform);

	std::unique_ptr<MeshUniversalComponent> FloorMeshComponent = std::make_unique<MeshUniversalComponent>();
	FloorMeshComponent->ModelMeshPath = "InputResources/Meshes/BlockInterestingArena.fbx";
	FloorMeshComponent->BaseColor = DirectX::XMFLOAT4(0.2f, 0.25f, 0.3f, 1.0f);

	std::unique_ptr<PhysicsComponent> FloorPhysicsComponent = std::make_unique<PhysicsComponent>();
	FloorPhysicsComponent->SetIsStatic(true);
	FloorPhysicsComponent->SetUseGravity(false);
	FloorPhysicsComponent->EnableAutoTriangleMeshColliderFromMesh(true);
	
	FloorActor->AddComponent(std::move(FloorMeshComponent));
	FloorActor->AddComponent(std::move(FloorPhysicsComponent));
	AddActor(std::move(FloorActor));
}

void KatamaryGame::SpawnPlayer()
{
	std::unique_ptr<Actor> NewPlayerActor = std::make_unique<Actor>();
	Transform PlayerTransform;
	PlayerTransform.Position = DirectX::XMFLOAT3(0.0f, 10.0f, 0.0f);
	PlayerTransform.Scale = DirectX::XMFLOAT3(1.4f, 1.4f, 1.4f);
	NewPlayerActor->SetTransform(PlayerTransform);

	std::unique_ptr<MeshUniversalComponent> PlayerMeshComponent = std::make_unique<MeshUniversalComponent>();
	PlayerMeshComponent->ModelMeshPath = "InputResources/Meshes/SimpleSphere.fbx";
	PlayerMeshComponent->AlbedoTexturePath = "InputResources/Textures/NoiseTexture.png";
	PlayerMeshComponent->BaseColor = DirectX::XMFLOAT4(0.75f, 0.28f, 0.95f, 1.0f);

	std::unique_ptr<PhysicsComponent> NewPlayerPhysicsComponent = std::make_unique<PhysicsComponent>();
	NewPlayerPhysicsComponent->SetMass(6.0f);
	NewPlayerPhysicsComponent->SetUseGravity(true);
	NewPlayerPhysicsComponent->EnableAutoConvexColliderFromMesh(false);
	NewPlayerPhysicsComponent->EnableAutoTriangleMeshColliderFromMesh(false);
	NewPlayerPhysicsComponent->SetSphereCollider(1.0f);
	NewPlayerPhysicsComponent->SetLinearDamping(0.45f);
	NewPlayerPhysicsComponent->SetAngularDamping(0.1f);
	NewPlayerPhysicsComponent->SetRestitution(0.05f);
	NewPlayerPhysicsComponent->SetCollisionLayer(PhysicsCollisionLayerPlayer);
	NewPlayerPhysicsComponent->SetCollisionMask(PhysicsCollisionMaskPlayer);
	PlayerPhysicsComponent = NewPlayerPhysicsComponent.get();
	BasePlayerSphereColliderRadius = NewPlayerPhysicsComponent->GetSphereRadius();

	PlayerActor = NewPlayerActor.get();
	NewPlayerActor->AddComponent(std::move(PlayerMeshComponent));
	NewPlayerActor->AddComponent(std::move(NewPlayerPhysicsComponent));
	AddActor(std::move(NewPlayerActor));
	UpdatePlayerCollisionSphereRadius();
}

void KatamaryGame::SpawnCollectibles()
{
	if (CollectibleMeshPaths.empty())
	{
		return;
	}
	
	CollectiblePhysicsComponents.reserve(SpawnedCollectibleCount);
	std::vector<std::unique_ptr<Actor>> CollectibleActorsToSpawn;
	CollectibleActorsToSpawn.reserve(SpawnedCollectibleCount);
	for (int SpawnedCollectibleIndex = 0; SpawnedCollectibleIndex < SpawnedCollectibleCount; ++SpawnedCollectibleIndex)
	{
		const int RandomMeshPathIndex = static_cast<int>(GetRandomValueInRange(0.0f, static_cast<float>(CollectibleMeshPaths.size())));
		const int ClampedMeshPathIndex = (std::clamp)(RandomMeshPathIndex, 0, static_cast<int>(CollectibleMeshPaths.size()) - 1);
		const MeshLocalData& SelectedMeshLocalData = CollectibleMeshPaths[ClampedMeshPathIndex];

		float SpawnPositionX = 0.0f;
		float SpawnPositionZ = 0.0f;
		for (int AttemptIndex = 0; AttemptIndex < 10; ++AttemptIndex)
		{
			SpawnPositionX = GetRandomValueInRange(-18.0f, 18.0f);
			SpawnPositionZ = GetRandomValueInRange(-18.0f, 18.0f);
			const float SpawnDistanceToCenter = std::sqrt((SpawnPositionX * SpawnPositionX) + (SpawnPositionZ * SpawnPositionZ));
			if (SpawnDistanceToCenter > 4.0f)
			{
				break;
			}
		}
		

		std::unique_ptr<Actor> CollectibleActor = std::make_unique<Actor>();
		Transform CollectibleTransform;
		DirectX::XMFLOAT3 CollectibleSpawnPosition(SpawnPositionX, -0.9f, SpawnPositionZ);
		const DirectX::XMFLOAT3 TraceStart(SpawnPositionX, 50.0f, SpawnPositionZ);
		const DirectX::XMFLOAT3 TraceDirection(0.0f, -1.0f, 0.0f);
		PhysicsLineTraceHitResult SurfaceTraceHitResult = {};
		if (PhysicsLibrary::LineTrace(TraceStart, TraceDirection, 200.0f, SurfaceTraceHitResult) && SurfaceTraceHitResult.HasBlockingHit)
		{
			CollectibleSpawnPosition = SurfaceTraceHitResult.HitLocation;
		}

		CollectibleTransform.Position = CollectibleSpawnPosition;
		CollectibleTransform.Scale = DirectX::XMFLOAT3(GlobalCollectibleMeshScale, GlobalCollectibleMeshScale, GlobalCollectibleMeshScale);
		CollectibleActor->SetTransform(CollectibleTransform);

		std::unique_ptr<MeshUniversalComponent> CollectibleMeshComponent = std::make_unique<MeshUniversalComponent>();
		CollectibleMeshComponent->ModelMeshPath = SelectedMeshLocalData.MeshPath;
		CollectibleMeshComponent->BaseColor = SelectedMeshLocalData.DefaultAlbedoColor;
		CollectibleMeshComponent->AlbedoTexturePath = SelectedMeshLocalData.AlbedoTexture;
		CollectibleMeshComponent->NormalTexturePath = SelectedMeshLocalData.NormalTexture;
		CollectibleMeshComponent->SpecularTexturePath = SelectedMeshLocalData.MetallicTexture;
		CollectibleMeshComponent->EmissiveTexturePath = SelectedMeshLocalData.RoughnessTexture;
		

		std::unique_ptr<PhysicsComponent> CollectiblePhysicsComponent = std::make_unique<PhysicsComponent>();
		CollectiblePhysicsComponent->SetMass(0.8f);
		CollectiblePhysicsComponent->SetUseGravity(true);
		CollectiblePhysicsComponent->SetLinearDamping(0.4f);
		CollectiblePhysicsComponent->SetAngularDamping(0.25f);
		CollectiblePhysicsComponent->SetCollisionMode(PhysicsCollisionMode::Simulation);
		CollectiblePhysicsComponent->SetCollisionLayer(PhysicsCollisionLayerCollectibleUncollected);
		CollectiblePhysicsComponent->SetCollisionMask(PhysicsCollisionMaskCollectibleUncollected);
		CollectiblePhysicsComponent->EnableAutoConvexColliderFromMesh(true);
		CollectiblePhysicsComponents.push_back(CollectiblePhysicsComponent.get());

		CollectibleActor->AddComponent(std::move(CollectibleMeshComponent));
		CollectibleActor->AddComponent(std::move(CollectiblePhysicsComponent));
		CollectibleActorsToSpawn.push_back(std::move(CollectibleActor));
	}

	for (std::unique_ptr<Actor>& ExistingCollectibleActor : CollectibleActorsToSpawn)
	{
		AddActor(std::move(ExistingCollectibleActor));
	}
}

void KatamaryGame::SpawnUserInterface()
{
	std::unique_ptr<Actor> UserInterfaceActor = std::make_unique<Actor>();
	std::unique_ptr<KatamaryUIRenderingComponent> NewKatamaryUIRenderingComponent = std::make_unique<KatamaryUIRenderingComponent>();
	KatamaryUIRenderingComponentInstance = NewKatamaryUIRenderingComponent.get();
	UserInterfaceActor->AddComponent(std::move(NewKatamaryUIRenderingComponent));
	AddActor(std::move(UserInterfaceActor));
}

void KatamaryGame::HandleRoundTimer(float DeltaTime)
{
	if (IsRoundFinished || DeltaTime <= 0.0f)
	{
		return;
	}

	RemainingTimeSeconds -= DeltaTime;
	if (RemainingTimeSeconds <= 0.0f)
	{
		RemainingTimeSeconds = 0.0f;
		IsRoundFinished = true;
	}
}

void KatamaryGame::HandlePlayerMovementInput(float DeltaTime, float MovementInputForward, float MovementInputRight)
{
	if (IsRoundFinished || PlayerPhysicsComponent == nullptr || DeltaTime <= 0.0f)
	{
		return;
	}

	DirectX::XMFLOAT3 DesiredPlanarMovementDirection = BuildCameraRelativeMovementDirection(MovementInputForward, MovementInputRight);
	const float DesiredDirectionLengthSquared =
		(DesiredPlanarMovementDirection.x * DesiredPlanarMovementDirection.x) +
		(DesiredPlanarMovementDirection.z * DesiredPlanarMovementDirection.z);

	if (DesiredDirectionLengthSquared > 0.0001f)
	{
		const float DesiredDirectionLength = std::sqrt(DesiredDirectionLengthSquared);
		if (DesiredDirectionLength > 0.0001f)
		{
			DesiredPlanarMovementDirection.x /= DesiredDirectionLength;
			DesiredPlanarMovementDirection.z /= DesiredDirectionLength;
		}

		const DirectX::XMFLOAT3 ForceValue(
			DesiredPlanarMovementDirection.x * PlayerMoveForce,
			0.0f,
			DesiredPlanarMovementDirection.z * PlayerMoveForce);
		PlayerPhysicsComponent->AddForce(ForceValue);
	}

	DirectX::XMFLOAT3 CurrentVelocity = PlayerPhysicsComponent->GetVelocity();
	const float CurrentPlanarSpeedSquared =
		(CurrentVelocity.x * CurrentVelocity.x) +
		(CurrentVelocity.z * CurrentVelocity.z);
	const float PlayerMaximumPlanarSpeedSquared = PlayerMaximumPlanarSpeed * PlayerMaximumPlanarSpeed;
	if (CurrentPlanarSpeedSquared > PlayerMaximumPlanarSpeedSquared)
	{
		const float CurrentPlanarSpeed = std::sqrt(CurrentPlanarSpeedSquared);
		if (CurrentPlanarSpeed > 0.0001f)
		{
			const float SpeedScale = PlayerMaximumPlanarSpeed / CurrentPlanarSpeed;
			CurrentVelocity.x *= SpeedScale;
			CurrentVelocity.z *= SpeedScale;
			PlayerPhysicsComponent->SetVelocity(CurrentVelocity);
		}
	}
}

void KatamaryGame::HandlePlayerJumpInput()
{
	if (IsRoundFinished || PlayerPhysicsComponent == nullptr)
	{
		return;
	}
	if (IsPlayerNearSurfaceForJump() == false)
	{
		return;
	}

	const DirectX::XMFLOAT3 JumpImpulseValue(0.0f, PlayerJumpImpulse, 0.0f);
	PlayerPhysicsComponent->ApplyImpulse(JumpImpulseValue);
}

void KatamaryGame::HandleLightShotInput()
{
	if (IsRoundFinished || PlayerActor == nullptr)
	{
		return;
	}

	SpawnLightProjectile();
}

bool KatamaryGame::IsPlayerNearSurfaceForJump() const
{
	if (PlayerActor == nullptr || PlayerPhysicsComponent == nullptr)
	{
		return false;
	}

	const DirectX::XMFLOAT3 PlayerPosition = PlayerActor->GetLocation(ETransformSpace::World);
	const DirectX::XMFLOAT3 PlayerScale = PlayerActor->GetScale(ETransformSpace::World);
	const float PlayerMaximumScaleAxis = (std::max)(
		(std::max)(std::fabs(PlayerScale.x), std::fabs(PlayerScale.y)),
		std::fabs(PlayerScale.z));
	const float EffectivePlayerSphereRadius = (std::max)(0.01f, PlayerPhysicsComponent->GetSphereRadius() * PlayerMaximumScaleAxis);
	const float AllowedDistanceToSurface = EffectivePlayerSphereRadius + PlayerJumpTraceExtraDistance;
	std::vector<PhysicsComponent*> IgnoredPhysicsComponents;
	IgnoredPhysicsComponents.push_back(PlayerPhysicsComponent);

	PhysicsLineTraceHitResult SurfaceTraceHitResult = {};
	const bool HasSurfaceTraceHit = PhysicsLibrary::LineTrace(
		PlayerPosition,
		DirectX::XMFLOAT3(0.0f, -1.0f, 0.0f),
		AllowedDistanceToSurface,
		IgnoredPhysicsComponents,
		SurfaceTraceHitResult);
	
	if (HasSurfaceTraceHit == false || SurfaceTraceHitResult.HasBlockingHit == false)
	{
		return false;
	}

	return SurfaceTraceHitResult.HitDistance <= AllowedDistanceToSurface;
}

void KatamaryGame::UpdateGameplayCamera()
{
	if (GameplayCameraComponent == nullptr || PlayerActor == nullptr)
	{
		return;
	}

	Actor* CameraActor = GameplayCameraComponent->GetOwningActor();
	if (CameraActor == nullptr)
	{
		return;
	}

	const DirectX::XMFLOAT3 PlayerPosition = PlayerActor->GetLocation();
	const DirectX::XMFLOAT3 CameraOffset(-8.0f, 7.0f, -8.0f);
	const DirectX::XMFLOAT3 CameraPosition(
		PlayerPosition.x + CameraOffset.x,
		PlayerPosition.y + CameraOffset.y,
		PlayerPosition.z + CameraOffset.z);

	const DirectX::XMFLOAT3 ViewDirection(
		PlayerPosition.x - CameraPosition.x,
		PlayerPosition.y - CameraPosition.y,
		PlayerPosition.z - CameraPosition.z);
	const float ViewDirectionPlanarLength = std::sqrt(
		(ViewDirection.x * ViewDirection.x) +
		(ViewDirection.z * ViewDirection.z));
	const float CameraYaw = std::atan2(ViewDirection.x, ViewDirection.z);
	const float CameraPitch = std::atan2(ViewDirection.y, (std::max)(0.0001f, ViewDirectionPlanarLength));

	Transform CameraTransform;
	CameraTransform.Position = CameraPosition;
	CameraTransform.RotationEuler = DirectX::XMFLOAT3(CameraPitch, CameraYaw, 0.0f);
	CameraActor->SetTransform(CameraTransform);
}

void KatamaryGame::UpdateLightProjectiles(float DeltaTime)
{
	if (DeltaTime <= 0.0f || ActiveLightProjectiles.empty())
	{
		return;
	}

	for (size_t LightProjectileIndex = 0; LightProjectileIndex < ActiveLightProjectiles.size();)
	{
		LightProjectileData& ExistingLightProjectileData = ActiveLightProjectiles[LightProjectileIndex];
		ExistingLightProjectileData.RemainingLifetimeSeconds -= DeltaTime;
		if (ExistingLightProjectileData.RemainingLifetimeSeconds <= 0.0f)
		{
			DeactivateLightProjectile(ExistingLightProjectileData);
			ActiveLightProjectiles.erase(ActiveLightProjectiles.begin() + static_cast<std::ptrdiff_t>(LightProjectileIndex));
			continue;
		}

		++LightProjectileIndex;
	}
}

void KatamaryGame::SpawnLightProjectile()
{
	if (MaximumActiveLightProjectileCount > 0 && static_cast<int>(ActiveLightProjectiles.size()) >= MaximumActiveLightProjectileCount)
	{
		DeactivateLightProjectile(ActiveLightProjectiles.front());
		ActiveLightProjectiles.erase(ActiveLightProjectiles.begin());
	}

	const DirectX::XMFLOAT3 LightShotDirection = BuildLightShotDirection();
	const DirectX::XMFLOAT3 PlayerWorldLocation = PlayerActor->GetLocation(ETransformSpace::World);
	const DirectX::XMFLOAT3 ProjectileSpawnLocation(
		PlayerWorldLocation.x + (LightShotDirection.x * LightProjectileSpawnOffsetDistance),
		PlayerWorldLocation.y + (LightShotDirection.y * LightProjectileSpawnOffsetDistance),
		PlayerWorldLocation.z + (LightShotDirection.z * LightProjectileSpawnOffsetDistance));

	std::unique_ptr<Actor> LightProjectileActor = std::make_unique<Actor>();
	Transform LightProjectileTransform;
	LightProjectileTransform.Position = ProjectileSpawnLocation;
	LightProjectileTransform.Scale = DirectX::XMFLOAT3(LightProjectileVisualScale, LightProjectileVisualScale, LightProjectileVisualScale);
	LightProjectileActor->SetTransform(LightProjectileTransform);

	std::unique_ptr<MeshUniversalComponent> LightProjectileMeshComponent = std::make_unique<MeshUniversalComponent>();
	LightProjectileMeshComponent->ModelMeshPath = "InputResources/Meshes/SimpleSphere.fbx";
	LightProjectileMeshComponent->BaseColor = DirectX::XMFLOAT4(1.0f, 0.9f, 0.35f, 1.0f);

	std::unique_ptr<PhysicsComponent> LightProjectilePhysicsComponent = std::make_unique<PhysicsComponent>();
	LightProjectilePhysicsComponent->SetMass(0.25f);
	LightProjectilePhysicsComponent->SetUseGravity(false);
	LightProjectilePhysicsComponent->SetLinearDamping(0.0f);
	LightProjectilePhysicsComponent->SetAngularDamping(0.0f);
	LightProjectilePhysicsComponent->EnableAutoConvexColliderFromMesh(false);
	LightProjectilePhysicsComponent->EnableAutoTriangleMeshColliderFromMesh(false);
	LightProjectilePhysicsComponent->SetSphereCollider(0.25f);
	LightProjectilePhysicsComponent->SetCollisionMode(PhysicsCollisionMode::Simulation);
	LightProjectilePhysicsComponent->SetCollisionLayer(PhysicsCollisionLayerCollectibleCollected);
	LightProjectilePhysicsComponent->SetCollisionMask(0xFFFFFFFFu);

	const DirectX::XMFLOAT3 ProjectileVelocity(
		LightShotDirection.x * LightProjectileSpeed,
		LightShotDirection.y * LightProjectileSpeed,
		LightShotDirection.z * LightProjectileSpeed);
	LightProjectilePhysicsComponent->SetVelocity(ProjectileVelocity);

	std::unique_ptr<LightComponent> LightProjectileLightComponent = std::make_unique<LightComponent>();
	LightProjectileLightComponent->SetLightType(LightType::Point);
	LightProjectileLightComponent->SetColor(DirectX::XMFLOAT4(1.0f, 0.92f, 0.55f, 1.0f));
	LightProjectileLightComponent->SetIntensity(LightProjectileLightIntensity);
	LightProjectileLightComponent->SetRange(LightProjectileLightRange);

	MeshUniversalComponent* LightProjectileMeshComponentRawPointer = LightProjectileMeshComponent.get();
	PhysicsComponent* LightProjectilePhysicsComponentRawPointer = LightProjectilePhysicsComponent.get();
	LightComponent* LightProjectileLightComponentRawPointer = LightProjectileLightComponent.get();
	Actor* LightProjectileActorRawPointer = LightProjectileActor.get();

	LightProjectileActor->AddComponent(std::move(LightProjectileMeshComponent));
	LightProjectileActor->AddComponent(std::move(LightProjectilePhysicsComponent));
	LightProjectileActor->AddComponent(std::move(LightProjectileLightComponent));
	AddActor(std::move(LightProjectileActor));

	LightProjectileData NewLightProjectileData = {};
	NewLightProjectileData.ProjectileActor = LightProjectileActorRawPointer;
	NewLightProjectileData.ProjectilePhysicsComponent = LightProjectilePhysicsComponentRawPointer;
	NewLightProjectileData.ProjectileMeshComponent = LightProjectileMeshComponentRawPointer;
	NewLightProjectileData.ProjectileLightComponent = LightProjectileLightComponentRawPointer;
	NewLightProjectileData.RemainingLifetimeSeconds = LightProjectileLifetimeSeconds;
	ActiveLightProjectiles.push_back(NewLightProjectileData);
}

void KatamaryGame::DeactivateLightProjectile(LightProjectileData& ExistingLightProjectileData)
{
	if (ExistingLightProjectileData.ProjectileLightComponent != nullptr)
	{
		ExistingLightProjectileData.ProjectileLightComponent->SetIsActive(false);
	}

	if (ExistingLightProjectileData.ProjectileMeshComponent != nullptr)
	{
		ExistingLightProjectileData.ProjectileMeshComponent->SetIsActive(false);
	}

	if (ExistingLightProjectileData.ProjectilePhysicsComponent != nullptr)
	{
		ExistingLightProjectileData.ProjectilePhysicsComponent->ClearForces();
		ExistingLightProjectileData.ProjectilePhysicsComponent->SetVelocity(DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f));
		ExistingLightProjectileData.ProjectilePhysicsComponent->SetAngularVelocity(DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f));
		ExistingLightProjectileData.ProjectilePhysicsComponent->SetCollisionMode(PhysicsCollisionMode::Trigger);
		ExistingLightProjectileData.ProjectilePhysicsComponent->SetIsStatic(true);
		ExistingLightProjectileData.ProjectilePhysicsComponent->SetIsActive(false);
	}

	ExistingLightProjectileData.ProjectileActor = nullptr;
	ExistingLightProjectileData.ProjectilePhysicsComponent = nullptr;
	ExistingLightProjectileData.ProjectileMeshComponent = nullptr;
	ExistingLightProjectileData.ProjectileLightComponent = nullptr;
	ExistingLightProjectileData.RemainingLifetimeSeconds = 0.0f;
}

DirectX::XMFLOAT3 KatamaryGame::BuildCameraRelativeMovementDirection(float MovementInputForward, float MovementInputRight) const
{
	float CameraYaw = 0.0f;
	CameraSubsystem* CameraSubsystemInstance = GetSubsystem<CameraSubsystem>();
	if (CameraSubsystemInstance != nullptr)
	{
		CameraComponent* ActiveCameraComponent = CameraSubsystemInstance->GetActiveCamera();
		if (ActiveCameraComponent != nullptr)
		{
			Actor* ActiveCameraActor = ActiveCameraComponent->GetOwningActor();
			if (ActiveCameraActor != nullptr)
			{
				CameraYaw = ActiveCameraActor->GetRotation(ETransformSpace::World).y;
			}
		}
	}

	const DirectX::XMFLOAT3 CameraForwardDirection(std::sin(CameraYaw), 0.0f, std::cos(CameraYaw));
	const DirectX::XMFLOAT3 CameraRightDirection(std::cos(CameraYaw), 0.0f, -std::sin(CameraYaw));
	return DirectX::XMFLOAT3(
		(CameraForwardDirection.x * MovementInputForward) + (CameraRightDirection.x * MovementInputRight),
		0.0f,
		(CameraForwardDirection.z * MovementInputForward) + (CameraRightDirection.z * MovementInputRight));
}

DirectX::XMFLOAT3 KatamaryGame::BuildLightShotDirection() const
{
	if (PlayerActor == nullptr)
	{
		return DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f);
	}

	DirectX::XMFLOAT3 CameraWorldLocation = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
	CameraSubsystem* CameraSubsystemInstance = GetSubsystem<CameraSubsystem>();
	if (CameraSubsystemInstance != nullptr)
	{
		CameraComponent* ActiveCameraComponent = CameraSubsystemInstance->GetActiveCamera();
		if (ActiveCameraComponent != nullptr && ActiveCameraComponent->GetOwningActor() != nullptr)
		{
			CameraWorldLocation = ActiveCameraComponent->GetOwningActor()->GetLocation(ETransformSpace::World);
		}
	}

	const DirectX::XMFLOAT3 PlayerWorldLocation = PlayerActor->GetLocation(ETransformSpace::World);
	DirectX::XMFLOAT3 ShotDirection(
		PlayerWorldLocation.x - CameraWorldLocation.x,
		PlayerWorldLocation.y - CameraWorldLocation.y,
		PlayerWorldLocation.z - CameraWorldLocation.z);
	const float ShotDirectionLengthSquared =
		(ShotDirection.x * ShotDirection.x) +
		(ShotDirection.y * ShotDirection.y) +
		(ShotDirection.z * ShotDirection.z);
	if (ShotDirectionLengthSquared <= 0.0001f)
	{
		return DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f);
	}

	const float InverseShotDirectionLength = 1.0f / std::sqrt(ShotDirectionLengthSquared);
	ShotDirection.x *= InverseShotDirectionLength;
	ShotDirection.y *= InverseShotDirectionLength;
	ShotDirection.z *= InverseShotDirectionLength;
	return ShotDirection;
}

void KatamaryGame::HandlePhysicsCollisionDetected(
	PhysicsComponent* FirstPhysicsComponent,
	PhysicsComponent* SecondPhysicsComponent,
	const DirectX::XMFLOAT3&,
	float)
{
	if (IsRoundFinished || PlayerPhysicsComponent == nullptr)
	{
		return;
	}

	const bool IsFirstUncollectedCollectible =
		IsCollectiblePhysicsComponent(FirstPhysicsComponent) &&
		CollectedCollectiblePhysicsComponents.find(FirstPhysicsComponent) == CollectedCollectiblePhysicsComponents.end();
	const bool IsSecondUncollectedCollectible =
		IsCollectiblePhysicsComponent(SecondPhysicsComponent) &&
		CollectedCollectiblePhysicsComponents.find(SecondPhysicsComponent) == CollectedCollectiblePhysicsComponents.end();

	PhysicsComponent* CollectorPhysicsComponent = nullptr;
	PhysicsComponent* CandidateCollectiblePhysicsComponent = nullptr;
	if (IsCollectorPhysicsComponent(FirstPhysicsComponent) && IsSecondUncollectedCollectible)
	{
		CollectorPhysicsComponent = FirstPhysicsComponent;
		CandidateCollectiblePhysicsComponent = SecondPhysicsComponent;
	}
	else if (IsCollectorPhysicsComponent(SecondPhysicsComponent) && IsFirstUncollectedCollectible)
	{
		CollectorPhysicsComponent = SecondPhysicsComponent;
		CandidateCollectiblePhysicsComponent = FirstPhysicsComponent;
	}

	if (CollectorPhysicsComponent == nullptr || CandidateCollectiblePhysicsComponent == nullptr)
	{
		return;
	}

	if (CollectedCollectiblePhysicsComponents.find(CandidateCollectiblePhysicsComponent) != CollectedCollectiblePhysicsComponents.end())
	{
		return;
	}

	PendingCollectiblePhysicsComponents.insert(CandidateCollectiblePhysicsComponent);
}

void KatamaryGame::TryAttachCollectibleToPlayer(PhysicsComponent* CandidateCollectiblePhysicsComponent)
{
	if (
		CandidateCollectiblePhysicsComponent == nullptr ||
		PlayerPhysicsComponent == nullptr)
	{
		return;
	}

	if (CollectedCollectiblePhysicsComponents.find(CandidateCollectiblePhysicsComponent) != CollectedCollectiblePhysicsComponents.end())
	{
		return;
	}

	Actor* CandidateCollectibleActor = CandidateCollectiblePhysicsComponent->GetOwningActor();
	if (PlayerActor == nullptr || CandidateCollectibleActor == nullptr)
	{
		return;
	}

	if (UseWeldCollectMode)
	{
		CandidateCollectiblePhysicsComponent->ClearForces();
		CandidateCollectiblePhysicsComponent->SetIsStatic(false);
		CandidateCollectiblePhysicsComponent->SetUseGravity(true);
		CandidateCollectiblePhysicsComponent->SetCollisionMode(PhysicsCollisionMode::Simulation);
		CandidateCollectiblePhysicsComponent->SetCollisionLayer(PhysicsCollisionLayerCollectibleCollected);
		CandidateCollectiblePhysicsComponent->SetCollisionMask(PhysicsCollisionMaskCollectibleCollectedWeld);
		CandidateCollectiblePhysicsComponent->SetVelocity(PlayerPhysicsComponent->GetVelocity());
		CandidateCollectiblePhysicsComponent->SetAngularVelocity(PlayerPhysicsComponent->GetAngularVelocity());

		if (PlayerPhysicsComponent->WeldWithComponent(CandidateCollectiblePhysicsComponent, true) == false)
		{
			return;
		}
	}
	else
	{
		CandidateCollectiblePhysicsComponent->ClearForces();
		CandidateCollectiblePhysicsComponent->SetUseGravity(false);
		CandidateCollectiblePhysicsComponent->SetVelocity(DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f));
		CandidateCollectiblePhysicsComponent->SetAngularVelocity(DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f));
		CandidateCollectiblePhysicsComponent->SetCollisionMode(PhysicsCollisionMode::Trigger);
		CandidateCollectiblePhysicsComponent->SetIsStatic(true);

		const Transform CandidateCollectibleWorldTransform = CandidateCollectibleActor->GetTransform(ETransformSpace::World);
		CandidateCollectibleActor->AttachToActor(PlayerActor);
		CandidateCollectibleActor->SetTransform(CandidateCollectibleWorldTransform, ETransformSpace::World);
	}

	CollectedCollectiblePhysicsComponents.insert(CandidateCollectiblePhysicsComponent);
	CollectedItemCount += 1;
	UpdatePlayerCollisionSphereRadius();
}

void KatamaryGame::ProcessPendingCollectibleAttachments()
{
	if (PendingCollectiblePhysicsComponents.empty())
	{
		return;
	}

	std::vector<PhysicsComponent*> PendingCollectiblesToAttach;
	PendingCollectiblesToAttach.reserve(PendingCollectiblePhysicsComponents.size());
	for (PhysicsComponent* ExistingPendingCollectiblePhysicsComponent : PendingCollectiblePhysicsComponents)
	{
		PendingCollectiblesToAttach.push_back(ExistingPendingCollectiblePhysicsComponent);
	}
	PendingCollectiblePhysicsComponents.clear();

	for (PhysicsComponent* ExistingPendingCollectiblePhysicsComponent : PendingCollectiblesToAttach)
	{
		TryAttachCollectibleToPlayer(ExistingPendingCollectiblePhysicsComponent);
	}
}

void KatamaryGame::UpdatePlayerCollisionSphereRadius()
{
	if (PlayerPhysicsComponent == nullptr)
	{
		return;
	}

	float TargetSphereColliderRadius = BasePlayerSphereColliderRadius;
	if (UseWeldCollectMode == false)
	{
		TargetSphereColliderRadius = BasePlayerSphereColliderRadius + (PlayerSphereColliderGrowthPerCollectible * static_cast<float>(CollectedItemCount));
	}
	const float CurrentSphereColliderRadius = PlayerPhysicsComponent->GetSphereRadius();
	if (std::fabs(CurrentSphereColliderRadius - TargetSphereColliderRadius) <= 0.0001f)
	{
		return;
	}

	PlayerPhysicsComponent->SetSphereCollider(TargetSphereColliderRadius);
}

bool KatamaryGame::IsCollectiblePhysicsComponent(const PhysicsComponent* CandidatePhysicsComponent) const
{
	if (CandidatePhysicsComponent == nullptr)
	{
		return false;
	}

	const auto ExistingCollectiblePhysicsComponentIterator = std::find(
		CollectiblePhysicsComponents.begin(),
		CollectiblePhysicsComponents.end(),
		CandidatePhysicsComponent);
	return ExistingCollectiblePhysicsComponentIterator != CollectiblePhysicsComponents.end();
}

bool KatamaryGame::IsCollectorPhysicsComponent(const PhysicsComponent* CandidatePhysicsComponent) const
{
	if (CandidatePhysicsComponent == nullptr)
	{
		return false;
	}

	if (CandidatePhysicsComponent == PlayerPhysicsComponent)
	{
		return true;
	}

	for (const PhysicsComponent* ExistingCollectedCollectiblePhysicsComponent : CollectedCollectiblePhysicsComponents)
	{
		if (ExistingCollectedCollectiblePhysicsComponent == CandidatePhysicsComponent)
		{
			return true;
		}
	}

	return false;
}

float KatamaryGame::GetRandomValueInRange(float MinimumValue, float MaximumValue)
{
	std::uniform_real_distribution<float> RandomDistribution(MinimumValue, MaximumValue);
	return RandomDistribution(RandomNumberGenerator);
}
