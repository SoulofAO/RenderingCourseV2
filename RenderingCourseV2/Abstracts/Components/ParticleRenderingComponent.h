#pragma once
#include "RenderingComponent.h"

class ParticleRenderingComponent : public RenderingComponent
{
public:
public:
    ParticleRenderingComponent();
    ~ParticleRenderingComponent() override;
    
    void Initialize() override;
    void Update(float DeltaTime) override;
    void Shutdown() override;
    
};
