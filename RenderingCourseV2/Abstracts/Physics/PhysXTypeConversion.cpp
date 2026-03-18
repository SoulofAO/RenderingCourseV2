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
	const float ValueXX = SourceQuaternion.x * SourceQuaternion.x;
	const float ValueYY = SourceQuaternion.y * SourceQuaternion.y;
	const float ValueZZ = SourceQuaternion.z * SourceQuaternion.z;
	const float ValueXY = SourceQuaternion.x * SourceQuaternion.y;
	const float ValueXZ = SourceQuaternion.x * SourceQuaternion.z;
	const float ValueYZ = SourceQuaternion.y * SourceQuaternion.z;
	const float ValueWX = SourceQuaternion.w * SourceQuaternion.x;
	const float ValueWY = SourceQuaternion.w * SourceQuaternion.y;
	const float ValueWZ = SourceQuaternion.w * SourceQuaternion.z;

	const float SinPitch = 2.0f * (ValueWX + ValueYZ);
	const float CosPitch = 1.0f - 2.0f * (ValueXX + ValueYY);
	const float PitchValue = std::atan2(SinPitch, CosPitch);

	const float SinYaw = 2.0f * (ValueWY - ValueXZ);
	const float ClampedSinYaw = std::clamp(SinYaw, -1.0f, 1.0f);
	const float YawValue = std::asin(ClampedSinYaw);

	const float SinRoll = 2.0f * (ValueWZ + ValueXY);
	const float CosRoll = 1.0f - 2.0f * (ValueYY + ValueZZ);
	const float RollValue = std::atan2(SinRoll, CosRoll);

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
