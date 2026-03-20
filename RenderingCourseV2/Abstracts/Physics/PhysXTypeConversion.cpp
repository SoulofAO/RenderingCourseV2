#include "Abstracts/Physics/PhysXTypeConversion.h"
#include <algorithm>
#include <cmath>

physx::PxVec3 PhysXTypeConversion::ToPxVector(const DirectX::XMFLOAT3& SourceValue)
{
	return physx::PxVec3(SourceValue.x, SourceValue.y, SourceValue.z);
}

DirectX::XMFLOAT3 PhysXTypeConversion::ToDirectXVector(const physx::PxVec3& SourceValue)
{
	return DirectX::XMFLOAT3(SourceValue.x, SourceValue.y, SourceValue.z);
}

physx::PxQuat PhysXTypeConversion::ToPxQuaternionFromEuler(const DirectX::XMFLOAT3& SourceEulerRotation)
{
	const DirectX::XMVECTOR QuaternionVector = DirectX::XMQuaternionRotationRollPitchYaw(
		SourceEulerRotation.x,
		SourceEulerRotation.y,
		SourceEulerRotation.z);
	DirectX::XMFLOAT4 QuaternionValue;
	DirectX::XMStoreFloat4(&QuaternionValue, QuaternionVector);
	return physx::PxQuat(QuaternionValue.x, QuaternionValue.y, QuaternionValue.z, QuaternionValue.w);
}

DirectX::XMFLOAT3 PhysXTypeConversion::ToDirectXEulerFromPxQuaternion(const physx::PxQuat& SourceQuaternion)
{
	const DirectX::XMVECTOR RotationQuaternionVector = DirectX::XMVectorSet(
		SourceQuaternion.x,
		SourceQuaternion.y,
		SourceQuaternion.z,
		SourceQuaternion.w);
	const DirectX::XMMATRIX RotationMatrix = DirectX::XMMatrixRotationQuaternion(RotationQuaternionVector);

	DirectX::XMFLOAT4X4 RotationMatrixValues;
	DirectX::XMStoreFloat4x4(&RotationMatrixValues, RotationMatrix);

	const float ClampedPitchInput = std::clamp(-RotationMatrixValues._32, -1.0f, 1.0f);
	const float PitchValue = std::asin(ClampedPitchInput);
	const float CosPitchValue = std::cos(PitchValue);

	float YawValue = 0.0f;
	float RollValue = 0.0f;
	if (std::fabs(CosPitchValue) > 0.0001f)
	{
		YawValue = std::atan2(RotationMatrixValues._31, RotationMatrixValues._33);
		RollValue = std::atan2(RotationMatrixValues._12, RotationMatrixValues._22);
	}
	else
	{
		YawValue = std::atan2(-RotationMatrixValues._13, RotationMatrixValues._11);
	}

	return DirectX::XMFLOAT3(PitchValue, YawValue, RollValue);
}

physx::PxTransform PhysXTypeConversion::ToPxTransform(const Transform& SourceTransform)
{
	const physx::PxVec3 PositionValue = ToPxVector(SourceTransform.Position);
	const physx::PxQuat RotationValue = ToPxQuaternionFromEuler(SourceTransform.RotationEuler);
	return physx::PxTransform(PositionValue, RotationValue);
}

Transform PhysXTypeConversion::ToDirectXTransform(const physx::PxTransform& SourceTransform, const DirectX::XMFLOAT3& SourceScale)
{
	Transform ResultTransform;
	ResultTransform.Position = ToDirectXVector(SourceTransform.p);
	ResultTransform.RotationEuler = ToDirectXEulerFromPxQuaternion(SourceTransform.q);
	ResultTransform.Scale = SourceScale;
	return ResultTransform;
}
