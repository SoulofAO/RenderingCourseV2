#include "Abstracts/Others/PhysicsLibrary.h"
#include "Abstracts/Core/Game.h"
#include "Abstracts/Components/CameraComponent.h"
#include "Abstracts/Physics/PhysXTypeConversion.h"
#include "Abstracts/Subsystems/CameraSubsystem.h"
#include "Abstracts/Subsystems/PhysicsSubsystem.h"
#include <directxmath.h>

bool PhysicsLibrary::BuildLineTraceFromMousePosition(
	int MousePositionX,
	int MousePositionY,
	DirectX::XMFLOAT3& TraceStart,
	DirectX::XMFLOAT3& TraceDirection)
{
	TraceStart = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
	TraceDirection = DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f);

	if (GlobalGame == nullptr)
	{
		return false;
	}

	CameraSubsystem* CameraSubsystemInstance = GlobalGame->GetSubsystem<CameraSubsystem>();
	if (CameraSubsystemInstance == nullptr)
	{
		return false;
	}

	CameraComponent* ActiveCameraComponent = CameraSubsystemInstance->GetActiveCamera();
	if (ActiveCameraComponent == nullptr)
	{
		return false;
	}

	const int ScreenWidth = GlobalGame->GetScreenWidth();
	const int ScreenHeight = GlobalGame->GetScreenHeight();
	if (ScreenWidth <= 0 || ScreenHeight <= 0)
	{
		return false;
	}

	const float ScreenWidthFloat = static_cast<float>(ScreenWidth);
	const float ScreenHeightFloat = static_cast<float>(ScreenHeight);
	const float NormalizedDeviceCoordinatePositionX = ((2.0f * static_cast<float>(MousePositionX)) / ScreenWidthFloat) - 1.0f;
	const float NormalizedDeviceCoordinatePositionY = 1.0f - ((2.0f * static_cast<float>(MousePositionY)) / ScreenHeightFloat);
	const float AspectRatio = ScreenWidthFloat / ScreenHeightFloat;

	const DirectX::XMMATRIX ViewMatrix = ActiveCameraComponent->GetViewMatrix();
	const DirectX::XMMATRIX ProjectionMatrix = ActiveCameraComponent->GetProjectionMatrix(AspectRatio);
	const DirectX::XMMATRIX InverseViewProjectionMatrix = DirectX::XMMatrixInverse(nullptr, ViewMatrix * ProjectionMatrix);

	DirectX::XMVECTOR NearClipPosition = DirectX::XMVectorSet(
		NormalizedDeviceCoordinatePositionX,
		NormalizedDeviceCoordinatePositionY,
		0.0f,
		1.0f);
	DirectX::XMVECTOR FarClipPosition = DirectX::XMVectorSet(
		NormalizedDeviceCoordinatePositionX,
		NormalizedDeviceCoordinatePositionY,
		1.0f,
		1.0f);

	DirectX::XMVECTOR NearWorldPosition = DirectX::XMVector4Transform(NearClipPosition, InverseViewProjectionMatrix);
	DirectX::XMVECTOR FarWorldPosition = DirectX::XMVector4Transform(FarClipPosition, InverseViewProjectionMatrix);
	NearWorldPosition = DirectX::XMVectorScale(NearWorldPosition, 1.0f / DirectX::XMVectorGetW(NearWorldPosition));
	FarWorldPosition = DirectX::XMVectorScale(FarWorldPosition, 1.0f / DirectX::XMVectorGetW(FarWorldPosition));

	const DirectX::XMVECTOR TraceDirectionVector = DirectX::XMVector3Normalize(
		DirectX::XMVectorSubtract(FarWorldPosition, NearWorldPosition));
	DirectX::XMStoreFloat3(&TraceStart, NearWorldPosition);
	DirectX::XMStoreFloat3(&TraceDirection, TraceDirectionVector);
	return true;
}

bool PhysicsLibrary::LineTrace(
	const DirectX::XMFLOAT3& TraceStart,
	const DirectX::XMFLOAT3& TraceDirection,
	float TraceLength,
	PhysicsLineTraceHitResult& HitResult)
{
	HitResult.HasBlockingHit = false;
	HitResult.HitLocation = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
	HitResult.HitNormal = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
	HitResult.HitDistance = 0.0f;
	HitResult.HitActor = nullptr;
	HitResult.HitShape = nullptr;

	if (TraceLength <= 0.0f)
	{
		return false;
	}

	if (GlobalGame == nullptr)
	{
		return false;
	}

	PhysicsSubsystem* PhysicsSubsystemInstance = GlobalGame->GetSubsystem<PhysicsSubsystem>();
	if (PhysicsSubsystemInstance == nullptr)
	{
		return false;
	}

	physx::PxScene* PhysicsScene = PhysicsSubsystemInstance->GetScene();
	if (PhysicsScene == nullptr)
	{
		return false;
	}

	const DirectX::XMVECTOR TraceDirectionVector = DirectX::XMLoadFloat3(&TraceDirection);
	const DirectX::XMVECTOR TraceDirectionLengthSquared = DirectX::XMVector3LengthSq(TraceDirectionVector);
	if (DirectX::XMVectorGetX(TraceDirectionLengthSquared) <= 0.000001f)
	{
		return false;
	}

	DirectX::XMFLOAT3 NormalizedTraceDirection;
	DirectX::XMStoreFloat3(
		&NormalizedTraceDirection,
		DirectX::XMVector3Normalize(TraceDirectionVector));

	const physx::PxVec3 TraceStartPhysX = PhysXTypeConversion::ToPxVector(TraceStart);
	const physx::PxVec3 TraceDirectionPhysX = PhysXTypeConversion::ToPxVector(NormalizedTraceDirection);
	const physx::PxHitFlags QueryHitFlags = physx::PxHitFlag::ePOSITION | physx::PxHitFlag::eNORMAL;
	physx::PxRaycastBuffer RaycastHitBuffer;

	const bool HasRaycastHit = PhysicsScene->raycast(
		TraceStartPhysX,
		TraceDirectionPhysX,
		TraceLength,
		RaycastHitBuffer,
		QueryHitFlags);
	if (HasRaycastHit == false || RaycastHitBuffer.hasBlock == false)
	{
		return false;
	}

	const physx::PxRaycastHit& BlockingHit = RaycastHitBuffer.block;
	HitResult.HasBlockingHit = true;
	HitResult.HitLocation = PhysXTypeConversion::ToDirectXVector(BlockingHit.position);
	HitResult.HitNormal = PhysXTypeConversion::ToDirectXVector(BlockingHit.normal);
	HitResult.HitDistance = BlockingHit.distance;
	HitResult.HitActor = BlockingHit.actor;
	HitResult.HitShape = BlockingHit.shape;
	return true;
}
