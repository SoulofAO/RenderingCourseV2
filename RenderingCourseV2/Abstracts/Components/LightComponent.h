#pragma once

#include "Abstracts/Components/ActorComponent.h"
#include <directxmath.h>

enum class LightType
{
	Directional,
	Point,
	Spot
};

class LightComponent : public ActorComponent
{
public:
	LightComponent();
	~LightComponent() override;

	LightType GetLightType() const;
	void SetLightType(LightType NewLightType);

	void SetColor(const DirectX::XMFLOAT4& NewColor);
	const DirectX::XMFLOAT4& GetColor() const;

	void SetIntensity(float NewIntensity);
	float GetIntensity() const;
	void SetRange(float NewRange);
	float GetRange() const;
	void SetSpotConeAnglesDegrees(float NewInnerConeAngleDegrees, float NewOuterConeAngleDegrees);
	float GetSpotInnerConeAngleDegrees() const;
	float GetSpotOuterConeAngleDegrees() const;

	DirectX::XMFLOAT3 GetDirection() const;

private:
	LightType Type;
	DirectX::XMFLOAT4 Color;
	float Intensity;
	float Range;
	float SpotInnerConeAngleDegrees;
	float SpotOuterConeAngleDegrees;
};
