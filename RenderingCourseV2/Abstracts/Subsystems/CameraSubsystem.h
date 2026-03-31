#pragma once

#include "Abstracts/Subsystems/Subsystem.h"
#include <directxmath.h>
#include <vector>

class CameraComponent;

class CameraSubsystem : public Subsystem
{
public:
	CameraSubsystem();
	~CameraSubsystem() override;

	void Update(float DeltaTime) override;

	void RegisterCamera(CameraComponent* NewCamera);
	void UnregisterCamera(CameraComponent* ExistingCamera);
	void CycleActiveCamera();
	void SetActiveCameraIndex(int NewActiveCameraIndex);
	void SetFallbackCamera(CameraComponent* NewFallbackCamera);
	CameraComponent* GetFallbackCamera() const;
	void SetIsFallbackCameraForced(bool NewIsFallbackCameraForced);
	bool GetIsFallbackCameraForced() const;

	CameraComponent* GetActiveCamera() const;
	int GetCameraCount() const;
	int GetActiveCameraIndex() const;

	DirectX::XMMATRIX GetActiveViewMatrix() const;
	DirectX::XMMATRIX GetActiveProjectionMatrix(float AspectRatio) const;
	DirectX::XMFLOAT3 GetActiveCameraPosition() const;

private:
	void UpdateCameraPossessionState();

	std::vector<CameraComponent*> Cameras;
	int ActiveCameraIndex;
	int LastNonFallbackActiveCameraIndex;
	CameraComponent* FallbackCamera;
	CameraComponent* PossessedCamera;
	bool IsFallbackCameraForced;
};
