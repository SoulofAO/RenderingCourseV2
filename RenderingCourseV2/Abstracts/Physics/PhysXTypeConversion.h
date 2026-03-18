#pragma once

#include "Abstracts/Core/Transform.h"
#include <physx/PxPhysicsAPI.h>
#include <directxmath.h>

class PhysXTypeConversion
{
public:
	static physx::PxVec3 ToPxVector(const DirectX::XMFLOAT3& SourceValue);
	static DirectX::XMFLOAT3 ToDirectXVector(const physx::PxVec3& SourceValue);
	static physx::PxQuat ToPxQuaternionFromEuler(const DirectX::XMFLOAT3& SourceEulerRotation);
	static DirectX::XMFLOAT3 ToDirectXEulerFromPxQuaternion(const physx::PxQuat& SourceQuaternion);
	static physx::PxTransform ToPxTransform(const Transform& SourceTransform);
	static Transform ToDirectXTransform(const physx::PxTransform& SourceTransform, const DirectX::XMFLOAT3& SourceScale);
};
