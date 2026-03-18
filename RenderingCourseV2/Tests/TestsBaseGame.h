#pragma once

#include "Abstracts/Core/Game.h"
#include <directxmath.h>

class Actor;

class TestsBaseGame : public Game
{
public:
	TestsBaseGame(LPCWSTR ApplicationName, int ScreenWidth, int ScreenHeight);
	~TestsBaseGame() override;

protected:
	void BeginPlay() override;
	virtual void BuildTestScene() = 0;

	Actor* SpawnDirectionalLightActor(
		const DirectX::XMFLOAT3& RotationEuler,
		const DirectX::XMFLOAT4& LightColor,
		float LightIntensity);
	Actor* SpawnFPSSpectateCameraActor(
		const DirectX::XMFLOAT3& CameraPosition,
		const DirectX::XMFLOAT3& CameraRotationEuler,
		float CameraMovementSpeedScale);
};
