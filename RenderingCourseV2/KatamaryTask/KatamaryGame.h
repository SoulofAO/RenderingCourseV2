#pragma once

#include "Abstracts/Core/Game.h"
#include "Abstracts/Core/MulticastDelegate.h"
#include <directxmath.h>
#include <random>
#include <string>
#include <unordered_set>
#include <vector>


struct MeshLocalData
{
	std::string MeshPath;
	std::string AlbedoTexture;
	DirectX::XMFLOAT4 DefaultAlbedoColor;
	std::string MetallicTexture;
	std::string RoughnessTexture;
	std::string NormalTexture;
	

	
	MeshLocalData(
	std::string InMeshPath,
	std::string InAlbedoTexture,
	DirectX::XMFLOAT4 InDefaultAlbedoColor,
	std::string InMetallicTexture,
	std::string InRoughnessTexture,
	std::string InNormalTexture)
	: MeshPath(std::move(InMeshPath))
	, AlbedoTexture(std::move(InAlbedoTexture))
	, DefaultAlbedoColor(InDefaultAlbedoColor)
	, MetallicTexture(std::move(InMetallicTexture))
	, RoughnessTexture(std::move(InRoughnessTexture))
	, NormalTexture(std::move(InNormalTexture))
	{
	}
};

class Actor;
class CameraComponent;
class KatamaryUIRenderingComponent;
class PhysicsComponent;
class MeshUniversalComponent;
class LightComponent;

struct LightProjectileData
{
	Actor* ProjectileActor;
	PhysicsComponent* ProjectilePhysicsComponent;
	MeshUniversalComponent* ProjectileMeshComponent;
	LightComponent* ProjectileLightComponent;
	float RemainingLifetimeSeconds;
};

class KatamaryGame : public Game
{
public:
	KatamaryGame(LPCWSTR ApplicationName, int ScreenWidth, int ScreenHeight);
	~KatamaryGame() override;

	int GetCollectedItemCount() const;
	float GetRemainingTimeSeconds() const;
	bool GetIsRoundFinished() const;
	bool GetUseWeldCollectMode() const;
	void SetUseWeldCollectMode(bool NewUseWeldCollectMode);
	void HandlePlayerMovementInput(float DeltaTime, float MovementInputForward, float MovementInputRight);
	void HandlePlayerJumpInput();
	void HandleLightShotInput();

protected:
	void BeginPlay() override;
	void Update(float DeltaTime) override;
	LRESULT MessageHandler(HWND WindowHandle, UINT Message, WPARAM WParam, LPARAM LParam) override;

private:
	void BuildScene();
	void SpawnGameplayCamera();
	void SpawnDirectionalLight();
	void SpawnFloor();
	void SpawnPlayer();
	void SpawnCollectibles();
	void SpawnUserInterface();
	void HandleRoundTimer(float DeltaTime);
	void ProcessPendingCollectibleAttachments();
	void UpdatePlayerCollisionSphereRadius();
	bool IsPlayerNearSurfaceForJump() const;
	void UpdateGameplayCamera();
	void UpdateLightProjectiles(float DeltaTime);
	void SpawnLightProjectile();
	void DeactivateLightProjectile(LightProjectileData& ExistingLightProjectileData);
	DirectX::XMFLOAT3 BuildCameraRelativeMovementDirection(float MovementInputForward, float MovementInputRight) const;
	DirectX::XMFLOAT3 BuildLightShotDirection() const;
	void HandlePhysicsCollisionDetected(
		PhysicsComponent* FirstPhysicsComponent,
		PhysicsComponent* SecondPhysicsComponent,
		const DirectX::XMFLOAT3& ContactNormal,
		float PenetrationDepth);
	void TryAttachCollectibleToPlayer(PhysicsComponent* CandidateCollectiblePhysicsComponent);
	bool IsCollectiblePhysicsComponent(const PhysicsComponent* CandidatePhysicsComponent) const;
	bool IsCollectorPhysicsComponent(const PhysicsComponent* CandidatePhysicsComponent) const;
	float GetRandomValueInRange(float MinimumValue, float MaximumValue);

	Actor* PlayerActor;
	PhysicsComponent* PlayerPhysicsComponent;
	CameraComponent* GameplayCameraComponent;
	KatamaryUIRenderingComponent* KatamaryUIRenderingComponentInstance;
	std::vector<PhysicsComponent*> CollectiblePhysicsComponents;
	std::unordered_set<PhysicsComponent*> CollectedCollectiblePhysicsComponents;
	std::unordered_set<PhysicsComponent*> PendingCollectiblePhysicsComponents;
	DelegateHandle CollisionDetectedDelegateHandle;
	std::mt19937 RandomNumberGenerator;
	std::vector<MeshLocalData> CollectibleMeshPaths;

	float GlobalCollectibleMeshScale;
	float BasePlayerSphereColliderRadius;
	float PlayerSphereColliderGrowthPerCollectible;
	float PlayerMoveForce;
	float PlayerJumpImpulse;
	float PlayerJumpTraceExtraDistance;
	float PlayerMaximumPlanarSpeed;
	float LightProjectileLifetimeSeconds;
	float LightProjectileSpeed;
	float LightProjectileSpawnOffsetDistance;
	float LightProjectileVisualScale;
	float LightProjectileLightRange;
	float LightProjectileLightIntensity;
	float RoundDurationSeconds;
	float RemainingTimeSeconds;
	int MaximumActiveLightProjectileCount;
	bool IsRoundFinished;
	bool UseWeldCollectMode;
	int CollectedItemCount;
	int SpawnedCollectibleCount;
	std::vector<LightProjectileData> ActiveLightProjectiles;
};
