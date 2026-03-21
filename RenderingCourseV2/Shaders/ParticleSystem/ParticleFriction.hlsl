#include "Shaders/ParticleSystem/ParticleShared.hlsli"

cbuffer FrictionConstants : register(b1)
{
    float4 LinearFrictionData;
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
        float LinearFriction = LinearFrictionData.x;
        float VelocityMagnitude = length(Particle.Velocity);
        if (VelocityMagnitude > 0.0001)
        {
            float3 VelocityDirection = Particle.Velocity / VelocityMagnitude;
            float DampedMagnitude = max(0.0, VelocityMagnitude - LinearFriction * DeltaTime);
            Particle.Velocity = VelocityDirection * DampedMagnitude;
        }
    }

    ParticleStateReadWrite[Index] = Particle;
}
