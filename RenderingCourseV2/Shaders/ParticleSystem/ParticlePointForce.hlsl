#include "Shaders/ParticleSystem/ParticleShared.hlsli"

cbuffer PointForceConstants : register(b1)
{
    float4 WorldPointPositionAndStrength;
    float4 FalloffConfiguration;
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
        float3 WorldPointPosition = WorldPointPositionAndStrength.xyz;
        float Strength = WorldPointPositionAndStrength.w;
        float MinimumDistance = FalloffConfiguration.x;
        float FalloffPower = FalloffConfiguration.y;
        float3 Offset = WorldPointPosition - Particle.Position;
        float DistanceSquared = dot(Offset, Offset);
        float MinimumDistanceSquared = MinimumDistance * MinimumDistance;
        float SoftenedDistanceSquared = max(DistanceSquared, MinimumDistanceSquared);
        float3 Direction = Offset / sqrt(DistanceSquared + 1e-8);
        float AccelerationMagnitude = Strength / pow(SoftenedDistanceSquared, FalloffPower);
        Particle.Velocity += Direction * AccelerationMagnitude * DeltaTime;
    }

    ParticleStateReadWrite[Index] = Particle;
}
