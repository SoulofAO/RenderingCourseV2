#pragma once

#include "Tests/TestsBaseGame.h"

class PhysicsComponent;

class PhysicsTestGame : public TestsBaseGame
{
public:
	PhysicsTestGame(LPCWSTR ApplicationName, int ScreenWidth, int ScreenHeight);
	~PhysicsTestGame() override;

protected:
	void BuildTestScene() override;
	void Update(float DeltaTime) override;

private:
	PhysicsComponent* RotatingSpherePhysicsComponent;
	DirectX::XMFLOAT3 RotatingSphereAngularImpulsePerSecond;
};
