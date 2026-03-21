#include "Shaders/ParticleSystem/ParticleShared.hlsli"

cbuffer LinearForceConstants : register(b1)
{
    float3 LinearAcceleration;
    float Padding0;
};

[numthreads(256, 1, 1)]
void Main(uint3 DispatchThreadId : SV_DispatchThreadID)
{
    uint Index = DispatchThreadId.x;

    if (Index >= ParticleCount)
    {
        return;
    }

    ParticleStructData Particle = ParticleStateReadWrite[Index];

    if (Particle.Active != 0)
    {
        Particle.Velocity += LinearAcceleration * DeltaTime;
    }

    ParticleStateReadWrite[Index] = Particle;
}
