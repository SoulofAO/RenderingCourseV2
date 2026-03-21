#include "Shaders/ParticleSystem/ParticleStructShared.hlsli"

RWStructuredBuffer<ParticleStructData> ParticleStateReadWrite : register(u0);

cbuffer ParticleConstants : register(b0)
{
    uint ParticleCount;
    float3 GravityDirection;
    float DeltaTime;
};
