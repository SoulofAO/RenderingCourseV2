#pragma once

#include <directxmath.h>

class MainMathLibrary
{
public:
	static DirectX::XMFLOAT3 DirectionToRotationEuler(const DirectX::XMFLOAT3& Direction);
	static DirectX::XMFLOAT3 RotationEulerToForwardVector(const DirectX::XMFLOAT3& RotationEuler);
	static DirectX::XMFLOAT3 RotationEulerToUpVector(const DirectX::XMFLOAT3& RotationEuler);
	static DirectX::XMFLOAT3 RotationEulerToRightVector(const DirectX::XMFLOAT3& RotationEuler);
};
