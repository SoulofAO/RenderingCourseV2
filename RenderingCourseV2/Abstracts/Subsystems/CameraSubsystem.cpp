#include "Abstracts/Subsystems/CameraSubsystem.h"
#include "Abstracts/Components/CameraComponent.h"
#include <algorithm>

CameraSubsystem::CameraSubsystem()
	: Subsystem()
	, ActiveCameraIndex(-1)
	, FallbackCamera(nullptr)
{
}

CameraSubsystem::~CameraSubsystem() = default;

void CameraSubsystem::Update(float DeltaTime)
{
	Subsystem::Update(DeltaTime);
}

void CameraSubsystem::RegisterCamera(CameraComponent* NewCamera)
{
	if (NewCamera == nullptr)
	{
		return;
	}

	auto ExistingCameraIterator = std::find(Cameras.begin(), Cameras.end(), NewCamera);
	if (ExistingCameraIterator != Cameras.end())
	{
		return;
	}

	Cameras.push_back(NewCamera);
	if (ActiveCameraIndex < 0)
	{
		ActiveCameraIndex = 0;
	}
}

void CameraSubsystem::UnregisterCamera(CameraComponent* ExistingCamera)
{
	if (ExistingCamera == nullptr)
	{
		return;
	}

	if (FallbackCamera == ExistingCamera)
	{
		FallbackCamera = nullptr;
	}

	auto ExistingCameraIterator = std::find(Cameras.begin(), Cameras.end(), ExistingCamera);
	if (ExistingCameraIterator == Cameras.end())
	{
		return;
	}

	const int RemovedCameraIndex = static_cast<int>(std::distance(Cameras.begin(), ExistingCameraIterator));
	Cameras.erase(ExistingCameraIterator);

	if (Cameras.empty())
	{
		ActiveCameraIndex = -1;
		return;
	}

	if (ActiveCameraIndex >= RemovedCameraIndex)
	{
		ActiveCameraIndex -= 1;
	}

	if (ActiveCameraIndex < 0)
	{
		ActiveCameraIndex = 0;
	}
}

void CameraSubsystem::CycleActiveCamera()
{
	if (Cameras.empty())
	{
		ActiveCameraIndex = -1;
		return;
	}

	if (ActiveCameraIndex < 0)
	{
		ActiveCameraIndex = 0;
		return;
	}

	ActiveCameraIndex = (ActiveCameraIndex + 1) % static_cast<int>(Cameras.size());
}

CameraComponent* CameraSubsystem::GetActiveCamera() const
{
	if (ActiveCameraIndex < 0 || ActiveCameraIndex >= static_cast<int>(Cameras.size()))
	{
		return FallbackCamera;
	}

	return Cameras[ActiveCameraIndex];
}

int CameraSubsystem::GetCameraCount() const
{
	return static_cast<int>(Cameras.size());
}

int CameraSubsystem::GetActiveCameraIndex() const
{
	return ActiveCameraIndex;
}

void CameraSubsystem::SetFallbackCamera(CameraComponent* NewFallbackCamera)
{
	FallbackCamera = NewFallbackCamera;
}

CameraComponent* CameraSubsystem::GetFallbackCamera() const
{
	return FallbackCamera;
}

DirectX::XMMATRIX CameraSubsystem::GetActiveViewMatrix() const
{
	CameraComponent* ActiveCamera = GetActiveCamera();
	if (ActiveCamera == nullptr)
	{
		return DirectX::XMMatrixIdentity();
	}

	return ActiveCamera->GetViewMatrix();
}

DirectX::XMMATRIX CameraSubsystem::GetActiveProjectionMatrix(float AspectRatio) const
{
	CameraComponent* ActiveCamera = GetActiveCamera();
	if (ActiveCamera == nullptr)
	{
		return DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(60.0f), AspectRatio, 0.1f, 1000.0f);
	}

	return ActiveCamera->GetProjectionMatrix(AspectRatio);
}

DirectX::XMFLOAT3 CameraSubsystem::GetActiveCameraPosition() const
{
	CameraComponent* ActiveCamera = GetActiveCamera();
	if (ActiveCamera == nullptr)
	{
		return DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
	}

	return ActiveCamera->GetWorldPosition();
}
