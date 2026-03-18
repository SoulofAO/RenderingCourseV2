#pragma once

#include <directxmath.h>
#include <cmath>

enum class ETransformSpace
{
	Local,
	World
};

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
		const DirectX::XMMATRIX LocalMatrix = LocalTransform.ToMatrix();
		const DirectX::XMMATRIX ParentMatrix = ParentTransform.ToMatrix();
		const DirectX::XMMATRIX CombinedMatrix = LocalMatrix * ParentMatrix;
		return FromMatrix(CombinedMatrix);
	}

	static Transform MakeRelative(const Transform& ParentTransform, const Transform& WorldTransform)
	{
		const DirectX::XMMATRIX ParentMatrix = ParentTransform.ToMatrix();
		const DirectX::XMMATRIX ParentInverseMatrix = DirectX::XMMatrixInverse(nullptr, ParentMatrix);
		const DirectX::XMMATRIX WorldMatrix = WorldTransform.ToMatrix();
		const DirectX::XMMATRIX RelativeMatrix = WorldMatrix * ParentInverseMatrix;
		return FromMatrix(RelativeMatrix);
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

private:
	static Transform FromMatrix(const DirectX::XMMATRIX& MatrixValue)
	{
		Transform ResultTransform;

		DirectX::XMVECTOR ScaleVector;
		DirectX::XMVECTOR RotationQuaternion;
		DirectX::XMVECTOR TranslationVector;
		const bool IsDecompositionSucceeded = DirectX::XMMatrixDecompose(
			&ScaleVector,
			&RotationQuaternion,
			&TranslationVector,
			MatrixValue);
		if (IsDecompositionSucceeded == false)
		{
			return ResultTransform;
		}

		DirectX::XMStoreFloat3(&ResultTransform.Scale, ScaleVector);
		DirectX::XMStoreFloat3(&ResultTransform.Position, TranslationVector);

		const DirectX::XMMATRIX RotationMatrix = DirectX::XMMatrixRotationQuaternion(RotationQuaternion);
		ResultTransform.RotationEuler = ExtractRotationEulerFromMatrix(RotationMatrix);
		return ResultTransform;
	}

	static DirectX::XMFLOAT3 ExtractRotationEulerFromMatrix(const DirectX::XMMATRIX& RotationMatrix)
	{
		DirectX::XMFLOAT4X4 RotationMatrixValues;
		DirectX::XMStoreFloat4x4(&RotationMatrixValues, RotationMatrix);

		const float ClampedPitchInput = ClampValue(-RotationMatrixValues._32, -1.0f, 1.0f);
		const float Pitch = std::asin(ClampedPitchInput);
		const float CosPitch = std::cos(Pitch);

		float Yaw = 0.0f;
		float Roll = 0.0f;
		if (std::fabs(CosPitch) > 0.0001f)
		{
			Yaw = std::atan2(RotationMatrixValues._31, RotationMatrixValues._33);
			Roll = std::atan2(RotationMatrixValues._12, RotationMatrixValues._22);
		}
		else
		{
			Yaw = std::atan2(-RotationMatrixValues._13, RotationMatrixValues._11);
		}

		return DirectX::XMFLOAT3(Pitch, Yaw, Roll);
	}

	static float ClampValue(float Value, float MinValue, float MaxValue)
	{
		if (Value < MinValue)
		{
			return MinValue;
		}
		if (Value > MaxValue)
		{
			return MaxValue;
		}
		return Value;
	}
};
