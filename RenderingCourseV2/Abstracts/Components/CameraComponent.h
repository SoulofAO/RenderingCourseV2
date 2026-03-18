#pragma once

#include "Abstracts/Components/ActorComponent.h"
#include <directxmath.h>

enum class CameraProjectionType
{
	Perspective,
	Orthographic
};

class CameraComponent : public ActorComponent
{
public:
	CameraComponent();
	~CameraComponent() override;

	void Initialize() override;
	void Shutdown() override;
	virtual void Posses();
	virtual void Unposses();

	DirectX::XMMATRIX GetViewMatrix() const;
	DirectX::XMMATRIX GetProjectionMatrix(float AspectRatio) const;
	DirectX::XMFLOAT3 GetWorldPosition() const;

	void SetProjectionType(CameraProjectionType NewProjectionType);
	CameraProjectionType GetProjectionType() const;
	void SetRegisterInCameraSubsystem(bool NewRegisterInCameraSubsystem);
	bool GetRegisterInCameraSubsystem() const;

	void SetFieldOfViewDegrees(float NewFieldOfViewDegrees);
	void SetNearPlane(float NewNearPlane);
	void SetFarPlane(float NewFarPlane);
	void SetOrthographicSize(float NewOrthographicWidth, float NewOrthographicHeight);

	float GetFieldOfViewDegrees() const;
	float GetNearPlane() const;
	float GetFarPlane() const;
	float GetOrthographicWidth() const;
	float GetOrthographicHeight() const;

private:
	CameraProjectionType ProjectionType;
	float FieldOfViewDegrees;
	float NearPlane;
	float FarPlane;
	float OrthographicWidth;
	float OrthographicHeight;
	bool RegisterInCameraSubsystem;
};
