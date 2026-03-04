#pragma once
#include "Abstracts/MeshUniversalComponent.h"

class PingPongSphere : public MeshUniversalComponent
{
public:
	PingPongSphere(Game* GameInstance);

public:
	virtual void Initialize() override;
	virtual void Update(float DeltaTime) override;
};

