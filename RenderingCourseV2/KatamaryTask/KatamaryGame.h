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

class KatamaryGame : public Game
{
public:
	KatamaryGame(LPCWSTR ApplicationName, int ScreenWidth, int ScreenHeight);
	~KatamaryGame() override;

	int GetCollectedItemCount() const;
	float GetRemainingTimeSeconds() const;
	bool GetIsRoundFinished() const;

protected:
	void BeginPlay() override;
	void Update(float DeltaTime) override;
	LRESULT MessageHandler(HWND WindowHandle, UINT Message, WPARAM WParam, LPARAM LParam) override;

private:
	void BuildScene();
	void SpawnGameplayCamera();
	void SpawnFloor();
	void SpawnPlayer();
	void SpawnCollectibles();
	void SpawnUserInterface();
	void HandleRoundTimer(float DeltaTime);
	void HandlePlayerMovement(float DeltaTime);
	void UpdateGameplayCamera();
	DirectX::XMFLOAT3 BuildCameraRelativeMovementDirection() const;
	void HandlePhysicsOverlapBegin(PhysicsComponent* FirstPhysicsComponent, PhysicsComponent* SecondPhysicsComponent);
	void TryAttachCollectibleToPlayer(PhysicsComponent* CandidateCollectiblePhysicsComponent);
	bool IsCollectiblePhysicsComponent(const PhysicsComponent* CandidatePhysicsComponent) const;
	float GetRandomValueInRange(float MinimumValue, float MaximumValue);

	Actor* PlayerActor;
	PhysicsComponent* PlayerPhysicsComponent;
	CameraComponent* GameplayCameraComponent;
	KatamaryUIRenderingComponent* KatamaryUIRenderingComponentInstance;
	std::vector<PhysicsComponent*> CollectiblePhysicsComponents;
	std::unordered_set<PhysicsComponent*> CollectedCollectiblePhysicsComponents;
	DelegateHandle OverlapBeginDelegateHandle;
	std::mt19937 RandomNumberGenerator;
	std::vector<MeshLocalData> CollectibleMeshPaths;

	float PlayerMoveForce;
	float PlayerMaximumPlanarSpeed;
	float RoundDurationSeconds;
	float RemainingTimeSeconds;
	bool IsRoundFinished;
	int CollectedItemCount;
};
