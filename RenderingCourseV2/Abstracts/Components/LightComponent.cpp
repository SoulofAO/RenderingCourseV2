#include "Abstracts/Components/LightComponent.h"
#include <algorithm>

LightComponent::LightComponent()
	: ActorComponent()
	, Type(LightType::Directional)
	, Color(1.0f, 1.0f, 1.0f, 1.0f)
	, Intensity(1.0f)
	, Range(15.0f)
	, SpotInnerConeAngleDegrees(20.0f)
	, SpotOuterConeAngleDegrees(35.0f)
{
}

LightComponent::~LightComponent() = default;

LightType LightComponent::GetLightType() const
{
	return Type;
}

void LightComponent::SetLightType(LightType NewLightType)
{
	Type = NewLightType;
}

void LightComponent::SetColor(const DirectX::XMFLOAT4& NewColor)
{
	Color = NewColor;
}

const DirectX::XMFLOAT4& LightComponent::GetColor() const
{
	return Color;
}

void LightComponent::SetIntensity(float NewIntensity)
{
	Intensity = std::max(NewIntensity, 0.0f);
}

float LightComponent::GetIntensity() const
{
	return Intensity;
}

void LightComponent::SetRange(float NewRange)
{
	Range = std::max(NewRange, 0.1f);
}

float LightComponent::GetRange() const
{
	return Range;
}

void LightComponent::SetSpotConeAnglesDegrees(float NewInnerConeAngleDegrees, float NewOuterConeAngleDegrees)
{
	const float ClampedInnerConeAngleDegrees = (std::clamp)(NewInnerConeAngleDegrees, 0.1f, 89.0f);
	const float ClampedOuterConeAngleDegrees = (std::clamp)(NewOuterConeAngleDegrees, ClampedInnerConeAngleDegrees, 89.5f);
	SpotInnerConeAngleDegrees = ClampedInnerConeAngleDegrees;
	SpotOuterConeAngleDegrees = ClampedOuterConeAngleDegrees;
}

float LightComponent::GetSpotInnerConeAngleDegrees() const
{
	return SpotInnerConeAngleDegrees;
}

float LightComponent::GetSpotOuterConeAngleDegrees() const
{
	return SpotOuterConeAngleDegrees;
}

DirectX::XMFLOAT3 LightComponent::GetDirection() const
{
	Transform WorldTransform = GetWorldTransform();
	DirectX::XMMATRIX RotationMatrix = DirectX::XMMatrixRotationRollPitchYaw(
		WorldTransform.RotationEuler.x,
		WorldTransform.RotationEuler.y,
		WorldTransform.RotationEuler.z);
	DirectX::XMVECTOR ForwardDirection = DirectX::XMVector3TransformNormal(
		DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f),
		RotationMatrix);
	DirectX::XMFLOAT3 ResultDirection;
	DirectX::XMStoreFloat3(&ResultDirection, ForwardDirection);
	return ResultDirection;
}
