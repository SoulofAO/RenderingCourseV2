#pragma once
#include "GameComponent.h"
class PingPong : public GameComponent
{
	virtual void Initialize() = 0;
	virtual void Update(float DeltaTime) = 0;
	virtual void Draw() = 0;
	virtual void DestroyResources() = 0;
};

