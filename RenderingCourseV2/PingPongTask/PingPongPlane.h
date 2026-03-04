#pragma once
#include "Abstracts/MeshUniversalComponent.h"
class PingPongPlane : public MeshUniversalComponent
{
	virtual void Initialize() override;
	virtual void Update(float DeltaTime) override;
};

