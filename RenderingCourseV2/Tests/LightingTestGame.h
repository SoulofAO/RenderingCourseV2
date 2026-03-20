#pragma once

#include "Tests/TestsBaseGame.h"

class LightingTestGame : public TestsBaseGame
{
public:
	LightingTestGame(LPCWSTR ApplicationName, int ScreenWidth, int ScreenHeight);
	~LightingTestGame() override;

protected:
	void BuildTestScene() override;

private:
	void SpawnPointLightActor(
		const DirectX::XMFLOAT3& Position,
		const DirectX::XMFLOAT4& LightColor,
		float LightIntensity,
		float LightRange);
	void SpawnSpotLightActor(
		const DirectX::XMFLOAT3& Position,
		const DirectX::XMFLOAT3& RotationEuler,
		const DirectX::XMFLOAT4& LightColor,
		float LightIntensity,
		float LightRange,
		float InnerConeAngleDegrees,
		float OuterConeAngleDegrees);
};
