#pragma once

#include "Abstracts/Core/Game.h"
#include <directxmath.h>
#include <string>
#include <vector>

class Actor;
class CameraComponent;
class PlanetsUIRenderingComponent;

class PlanetsGame : public Game
{
public:
	PlanetsGame(LPCWSTR ApplicationName, int ScreenWidth, int ScreenHeight);
	~PlanetsGame() override;

	bool GetUseOrthographicProjectionForActiveCamera() const;
	void SetUseOrthographicProjectionForActiveCamera(bool NewUseOrthographicProjectionForActiveCamera);

	float GetPlanetOrbitSpeedScale() const;
	void SetPlanetOrbitSpeedScale(float NewPlanetOrbitSpeedScale);
	float GetMoonOrbitSpeedScale() const;
	void SetMoonOrbitSpeedScale(float NewMoonOrbitSpeedScale);
	float GetPlanetOrbitRadiusScale() const;
	void SetPlanetOrbitRadiusScale(float NewPlanetOrbitRadiusScale);
	float GetMoonOrbitRadiusScale() const;
	void SetMoonOrbitRadiusScale(float NewMoonOrbitRadiusScale);

protected:
	void BeginPlay() override;
	void Update(float DeltaTime) override;
	LRESULT MessageHandler(HWND WindowHandle, UINT Message, WPARAM WParam, LPARAM LParam) override;

private:
	struct PlanetMoonOrbitData
	{
		Actor* PlanetActor;
		Actor* MoonActor;
		float PlanetOrbitAngle;
		float MoonOrbitAngle;
		float PlanetOrbitSpeed;
		float MoonOrbitSpeed;
		float PlanetSelfRotationSpeed;
		float MoonSelfRotationSpeed;
		float PlanetOrbitRadiusMultiplier;
		float MoonOrbitRadiusMultiplier;
	};

	Actor* CreateCelestialActor(
		const std::string& ModelMeshPath,
		const DirectX::XMFLOAT3& ActorScale,
		const DirectX::XMFLOAT4& ActorColor);
	void SpawnPlanetsAndMoons();
	void UpdatePlanetaryOrbits(float DeltaTime);
	void UpdateCameraProjectionTypes();
	void SetOrbitSpeedScaleValues(float NewPlanetOrbitSpeedScale, float NewMoonOrbitSpeedScale);
	void SetOrbitRadiusScaleValues(float NewPlanetOrbitRadiusScale, float NewMoonOrbitRadiusScale);

	Actor* SunActor;
	Actor* OrbitCameraPivotActor;
	CameraComponent* OrbitCameraComponent;
	CameraComponent* FallbackCameraComponentForPlanets;
	PlanetsUIRenderingComponent* PlanetsUIRenderingComponentInstance;
	std::vector<PlanetMoonOrbitData> PlanetMoonOrbitDataList;

	bool UseOrthographicProjectionForActiveCamera;
	float PlanetOrbitSpeedScale;
	float MoonOrbitSpeedScale;
	float PlanetOrbitRadiusScale;
	float MoonOrbitRadiusScale;
	float OrbitCameraYawSpeed;
	float SunSelfRotationSpeed;
};
