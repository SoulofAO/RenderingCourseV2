#pragma once

#include <directxmath.h>

struct Transform
{
	DirectX::XMFLOAT3 Position;
	DirectX::XMFLOAT3 RotationEuler;
	DirectX::XMFLOAT3 Scale;

	Transform()
		: Position(0.0f, 0.0f, 0.0f)
		, RotationEuler(0.0f, 0.0f, 0.0f)
		, Scale(1.0f, 1.0f, 1.0f)
	{
	}

	static Transform Combine(const Transform& ParentTransform, const Transform& LocalTransform)
	{
		Transform CombinedTransform;
		CombinedTransform.Position = DirectX::XMFLOAT3(
			ParentTransform.Position.x + LocalTransform.Position.x,
			ParentTransform.Position.y + LocalTransform.Position.y,
			ParentTransform.Position.z + LocalTransform.Position.z);
		CombinedTransform.RotationEuler = DirectX::XMFLOAT3(
			ParentTransform.RotationEuler.x + LocalTransform.RotationEuler.x,
			ParentTransform.RotationEuler.y + LocalTransform.RotationEuler.y,
			ParentTransform.RotationEuler.z + LocalTransform.RotationEuler.z);
		CombinedTransform.Scale = DirectX::XMFLOAT3(
			ParentTransform.Scale.x * LocalTransform.Scale.x,
			ParentTransform.Scale.y * LocalTransform.Scale.y,
			ParentTransform.Scale.z * LocalTransform.Scale.z);
		return CombinedTransform;
	}

	DirectX::XMMATRIX ToMatrix() const
	{
		DirectX::XMMATRIX ScaleMatrix = DirectX::XMMatrixScaling(Scale.x, Scale.y, Scale.z);
		DirectX::XMMATRIX RotationMatrix = DirectX::XMMatrixRotationRollPitchYaw(
			RotationEuler.x,
			RotationEuler.y,
			RotationEuler.z);
		DirectX::XMMATRIX TranslationMatrix = DirectX::XMMatrixTranslation(
			Position.x,
			Position.y,
			Position.z);
		return ScaleMatrix * RotationMatrix * TranslationMatrix;
	}
};
