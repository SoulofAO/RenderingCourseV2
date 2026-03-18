#include "Abstracts/Components/CameraComponent.h"
#include "Abstracts/Core/Actor.h"
#include "Abstracts/Core/Game.h"
#include "Abstracts/Subsystems/CameraSubsystem.h"
#include <algorithm>

CameraComponent::CameraComponent()
	: ActorComponent()
	, ProjectionType(CameraProjectionType::Perspective)
	, FieldOfViewDegrees(60.0f)
	, NearPlane(0.1f)
	, FarPlane(1000.0f)
	, OrthographicWidth(8.0f)
	, OrthographicHeight(6.0f)
	, RegisterInCameraSubsystem(true)
{
}

CameraComponent::~CameraComponent() = default;

void CameraComponent::Posses()
{
}

void CameraComponent::Unposses()
{
}

void CameraComponent::Initialize()
{
	ActorComponent::Initialize();

	Game* OwningGame = GetOwningGame();
	if (OwningGame == nullptr)
	{
		return;
	}

	CameraSubsystem* ExistingCameraSubsystem = OwningGame->GetSubsystem<CameraSubsystem>();
	if (ExistingCameraSubsystem != nullptr && RegisterInCameraSubsystem)
	{
		ExistingCameraSubsystem->RegisterCamera(this);
	}
}

void CameraComponent::Shutdown()
{
	Game* OwningGame = GetOwningGame();
	if (OwningGame != nullptr)
	{
		CameraSubsystem* ExistingCameraSubsystem = OwningGame->GetSubsystem<CameraSubsystem>();
		if (ExistingCameraSubsystem != nullptr && RegisterInCameraSubsystem)
		{
			ExistingCameraSubsystem->UnregisterCamera(this);
		}
	}

	ActorComponent::Shutdown();
}

DirectX::XMMATRIX CameraComponent::GetViewMatrix() const
{
	Transform WorldTransform = GetWorldTransform();

	const float Pitch = WorldTransform.RotationEuler.x;
	const float Yaw = WorldTransform.RotationEuler.y;
	const float Roll = WorldTransform.RotationEuler.z;
	DirectX::XMMATRIX RotationMatrix = DirectX::XMMatrixRotationRollPitchYaw(Pitch, Yaw, Roll);

	DirectX::XMVECTOR ForwardDirection = DirectX::XMVector3TransformNormal(
		DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f),
		RotationMatrix);
	DirectX::XMVECTOR UpDirection = DirectX::XMVector3TransformNormal(
		DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f),
		RotationMatrix);
	DirectX::XMVECTOR CameraPosition = DirectX::XMVectorSet(
		WorldTransform.Position.x,
		WorldTransform.Position.y,
		WorldTransform.Position.z,
		1.0f);
	return DirectX::XMMatrixLookToLH(CameraPosition, ForwardDirection, UpDirection);
}

DirectX::XMMATRIX CameraComponent::GetProjectionMatrix(float AspectRatio) const
{
	float ClampedAspectRatio = AspectRatio;
	if (ClampedAspectRatio <= 0.0f)
	{
		ClampedAspectRatio = 1.0f;
	}

	if (ProjectionType == CameraProjectionType::Orthographic)
	{
		return DirectX::XMMatrixOrthographicLH(OrthographicWidth, OrthographicHeight, NearPlane, FarPlane);
	}

	return DirectX::XMMatrixPerspectiveFovLH(
		DirectX::XMConvertToRadians(FieldOfViewDegrees),
		ClampedAspectRatio,
		NearPlane,
		FarPlane);
}

DirectX::XMFLOAT3 CameraComponent::GetWorldPosition() const
{
	Transform WorldTransform = GetWorldTransform();
	return WorldTransform.Position;
}

void CameraComponent::SetProjectionType(CameraProjectionType NewProjectionType)
{
	ProjectionType = NewProjectionType;
}

CameraProjectionType CameraComponent::GetProjectionType() const
{
	return ProjectionType;
}

void CameraComponent::SetRegisterInCameraSubsystem(bool NewRegisterInCameraSubsystem)
{
	RegisterInCameraSubsystem = NewRegisterInCameraSubsystem;
}

bool CameraComponent::GetRegisterInCameraSubsystem() const
{
	return RegisterInCameraSubsystem;
}

void CameraComponent::SetFieldOfViewDegrees(float NewFieldOfViewDegrees)
{
	FieldOfViewDegrees = (std::clamp)(NewFieldOfViewDegrees, 5.0f, 175.0f);
}

void CameraComponent::SetNearPlane(float NewNearPlane)
{
	NearPlane = (std::max)(NewNearPlane, 0.001f);
}

void CameraComponent::SetFarPlane(float NewFarPlane)
{
	FarPlane = (std::max)(NewFarPlane, NearPlane + 0.01f);
}

void CameraComponent::SetOrthographicSize(float NewOrthographicWidth, float NewOrthographicHeight)
{
	OrthographicWidth = (std::max)(NewOrthographicWidth, 0.01f);
	OrthographicHeight = (std::max)(NewOrthographicHeight, 0.01f);
}

float CameraComponent::GetFieldOfViewDegrees() const
{
	return FieldOfViewDegrees;
}

float CameraComponent::GetNearPlane() const
{
	return NearPlane;
}

float CameraComponent::GetFarPlane() const
{
	return FarPlane;
}

float CameraComponent::GetOrthographicWidth() const
{
	return OrthographicWidth;
}

float CameraComponent::GetOrthographicHeight() const
{
	return OrthographicHeight;
}
