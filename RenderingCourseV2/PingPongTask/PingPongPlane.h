#pragma once
#include "Abstracts/MeshUniversalComponent.h"

class PingPongPlane : public MeshUniversalComponent
{
public:
	PingPongPlane(Game* GameInstance);

public:
	virtual void Initialize() override;
	virtual void Update(float DeltaTime) override;
};

