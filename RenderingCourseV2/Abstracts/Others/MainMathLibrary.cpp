#include "Abstracts/Others/MainMathLibrary.h"
#include <cmath>

DirectX::XMFLOAT3 MainMathLibrary::DirectionToRotationEuler(const DirectX::XMFLOAT3& Direction)
{
	const DirectX::XMVECTOR DirectionVector = DirectX::XMLoadFloat3(&Direction);
	const DirectX::XMVECTOR DirectionLengthSquared = DirectX::XMVector3LengthSq(DirectionVector);
	if (DirectX::XMVectorGetX(DirectionLengthSquared) <= 0.000001f)
	{
		return DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
	}

	DirectX::XMFLOAT3 NormalizedDirection;
	DirectX::XMStoreFloat3(&NormalizedDirection, DirectX::XMVector3Normalize(DirectionVector));

	const float HorizontalLength = std::sqrt(
		NormalizedDirection.x * NormalizedDirection.x +
		NormalizedDirection.z * NormalizedDirection.z);
	const float Pitch = std::atan2(NormalizedDirection.y, HorizontalLength);
	const float Yaw = std::atan2(NormalizedDirection.x, NormalizedDirection.z);
	const float Roll = 0.0f;

	return DirectX::XMFLOAT3(Pitch, Yaw, Roll);
}

DirectX::XMFLOAT3 MainMathLibrary::RotationEulerToForwardVector(const DirectX::XMFLOAT3& RotationEuler)
{
	const DirectX::XMMATRIX RotationMatrix = DirectX::XMMatrixRotationRollPitchYaw(
		RotationEuler.x,
		RotationEuler.y,
		RotationEuler.z);
	const DirectX::XMVECTOR ForwardDirection = DirectX::XMVector3TransformNormal(
		DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f),
		RotationMatrix);

	DirectX::XMFLOAT3 NormalizedForwardDirection;
	DirectX::XMStoreFloat3(&NormalizedForwardDirection, DirectX::XMVector3Normalize(ForwardDirection));
	return NormalizedForwardDirection;
}

DirectX::XMFLOAT3 MainMathLibrary::RotationEulerToUpVector(const DirectX::XMFLOAT3& RotationEuler)
{
	const DirectX::XMMATRIX RotationMatrix = DirectX::XMMatrixRotationRollPitchYaw(
		RotationEuler.x,
		RotationEuler.y,
		RotationEuler.z);
	const DirectX::XMVECTOR UpDirection = DirectX::XMVector3TransformNormal(
		DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f),
		RotationMatrix);

	DirectX::XMFLOAT3 NormalizedUpDirection;
	DirectX::XMStoreFloat3(&NormalizedUpDirection, DirectX::XMVector3Normalize(UpDirection));
	return NormalizedUpDirection;
}

DirectX::XMFLOAT3 MainMathLibrary::RotationEulerToRightVector(const DirectX::XMFLOAT3& RotationEuler)
{
	const DirectX::XMMATRIX RotationMatrix = DirectX::XMMatrixRotationRollPitchYaw(
		RotationEuler.x,
		RotationEuler.y,
		RotationEuler.z);
	const DirectX::XMVECTOR RightDirection = DirectX::XMVector3TransformNormal(
		DirectX::XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f),
		RotationMatrix);

	DirectX::XMFLOAT3 NormalizedRightDirection;
	DirectX::XMStoreFloat3(&NormalizedRightDirection, DirectX::XMVector3Normalize(RightDirection));
	return NormalizedRightDirection;
}
