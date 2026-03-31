#include "Abstracts/Subsystems/CameraSubsystem.h"
#include "Abstracts/Components/CameraComponent.h"
#include <algorithm>

CameraSubsystem::CameraSubsystem()
	: Subsystem()
	, ActiveCameraIndex(-1)
	, LastNonFallbackActiveCameraIndex(-1)
	, FallbackCamera(nullptr)
	, PossessedCamera(nullptr)
	, IsFallbackCameraForced(false)
{
}

CameraSubsystem::~CameraSubsystem() = default;

void CameraSubsystem::Update(float DeltaTime)
{
	Subsystem::Update(DeltaTime);
	UpdateCameraPossessionState();
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
	LastNonFallbackActiveCameraIndex = ActiveCameraIndex;

	UpdateCameraPossessionState();
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

	if (PossessedCamera == ExistingCamera)
	{
		PossessedCamera->Unposses();
		PossessedCamera = nullptr;
	}

	auto ExistingCameraIterator = std::find(Cameras.begin(), Cameras.end(), ExistingCamera);
	if (ExistingCameraIterator == Cameras.end())
	{
		return;
	}

	const int RemovedCameraIndex = static_cast<int>(std::distance(Cameras.begin(), ExistingCameraIterator));
	Cameras.erase(ExistingCameraIterator);

	if (LastNonFallbackActiveCameraIndex == RemovedCameraIndex)
	{
		LastNonFallbackActiveCameraIndex = -1;
	}
	else if (LastNonFallbackActiveCameraIndex > RemovedCameraIndex)
	{
		LastNonFallbackActiveCameraIndex -= 1;
	}

	if (Cameras.empty())
	{
		ActiveCameraIndex = -1;
		LastNonFallbackActiveCameraIndex = -1;
		UpdateCameraPossessionState();
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
	if (ActiveCameraIndex >= static_cast<int>(Cameras.size()))
	{
		ActiveCameraIndex = static_cast<int>(Cameras.size()) - 1;
	}
	if (LastNonFallbackActiveCameraIndex < 0)
	{
		LastNonFallbackActiveCameraIndex = ActiveCameraIndex;
	}
	if (LastNonFallbackActiveCameraIndex >= static_cast<int>(Cameras.size()))
	{
		LastNonFallbackActiveCameraIndex = static_cast<int>(Cameras.size()) - 1;
	}

	UpdateCameraPossessionState();
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
		LastNonFallbackActiveCameraIndex = ActiveCameraIndex;
		UpdateCameraPossessionState();
		return;
	}

	ActiveCameraIndex = (ActiveCameraIndex + 1) % static_cast<int>(Cameras.size());
	LastNonFallbackActiveCameraIndex = ActiveCameraIndex;
	UpdateCameraPossessionState();
}

void CameraSubsystem::SetActiveCameraIndex(int NewActiveCameraIndex)
{
	if (Cameras.empty())
	{
		ActiveCameraIndex = -1;
		LastNonFallbackActiveCameraIndex = -1;
		UpdateCameraPossessionState();
		return;
	}

	const int ClampedCameraIndex = (std::clamp)(NewActiveCameraIndex, 0, static_cast<int>(Cameras.size()) - 1);
	ActiveCameraIndex = ClampedCameraIndex;
	LastNonFallbackActiveCameraIndex = ActiveCameraIndex;
	UpdateCameraPossessionState();
}

CameraComponent* CameraSubsystem::GetActiveCamera() const
{
	if (IsFallbackCameraForced && FallbackCamera != nullptr)
	{
		return FallbackCamera;
	}

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
	UpdateCameraPossessionState();
}

CameraComponent* CameraSubsystem::GetFallbackCamera() const
{
	return FallbackCamera;
}

void CameraSubsystem::SetIsFallbackCameraForced(bool NewIsFallbackCameraForced)
{
	if (IsFallbackCameraForced == NewIsFallbackCameraForced)
	{
		return;
	}

	if (NewIsFallbackCameraForced)
	{
		if (ActiveCameraIndex >= 0 && ActiveCameraIndex < static_cast<int>(Cameras.size()))
		{
			LastNonFallbackActiveCameraIndex = ActiveCameraIndex;
		}
		IsFallbackCameraForced = true;
		UpdateCameraPossessionState();
		return;
	}

	IsFallbackCameraForced = false;
	if (Cameras.empty())
	{
		ActiveCameraIndex = -1;
	}
	else
	{
		if (LastNonFallbackActiveCameraIndex >= 0 && LastNonFallbackActiveCameraIndex < static_cast<int>(Cameras.size()))
		{
			ActiveCameraIndex = LastNonFallbackActiveCameraIndex;
		}
		else
		{
			ActiveCameraIndex = 0;
		}
	}
	UpdateCameraPossessionState();
}

bool CameraSubsystem::GetIsFallbackCameraForced() const
{
	return IsFallbackCameraForced;
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

void CameraSubsystem::UpdateCameraPossessionState()
{
	CameraComponent* ActiveCamera = GetActiveCamera();
	if (PossessedCamera == ActiveCamera)
	{
		return;
	}

	if (PossessedCamera != nullptr)
	{
		PossessedCamera->Unposses();
	}

	PossessedCamera = ActiveCamera;
	if (PossessedCamera != nullptr)
	{
		PossessedCamera->Posses();
	}
}
