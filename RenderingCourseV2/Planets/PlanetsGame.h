#pragma once

#include "Abstracts/Core/Game.h"
#include <directxmath.h>
#include <string>
#include <vector>

class Actor;
class CameraComponent;
class FPSSpectateCameraComponent;
class OrbitCameraComponent;
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
	bool GetUseOrbitCamera() const;
	void SetUseOrbitCamera(bool NewUseOrbitCamera);

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
	Actor* GetDefaultOrbitCameraTargetActor() const;
	void UpdatePlanetaryOrbits(float DeltaTime);
	void UpdateCameraProjectionTypes();
	void SetOrbitSpeedScaleValues(float NewPlanetOrbitSpeedScale, float NewMoonOrbitSpeedScale);
	void SetOrbitRadiusScaleValues(float NewPlanetOrbitRadiusScale, float NewMoonOrbitRadiusScale);

	Actor* SunActor;
	OrbitCameraComponent* OrbitCameraForPlanets;
	FPSSpectateCameraComponent* FPSCameraComponentForPlanets;
	PlanetsUIRenderingComponent* PlanetsUIRenderingComponentInstance;
	std::vector<PlanetMoonOrbitData> PlanetMoonOrbitDataList;

	bool UseOrthographicProjectionForActiveCamera;
	bool UseOrbitCamera;
	float PlanetOrbitSpeedScale;
	float MoonOrbitSpeedScale;
	float PlanetOrbitRadiusScale;
	float MoonOrbitRadiusScale;
	float SunSelfRotationSpeed;
};
