#include "Shaders/ParticleSystem/ParticleShared.hlsli"

cbuffer SpawnRateConstants : register(b1)
{
    uint SpawnCount;
    uint BaseIndex;
    uint MaxParticleCount;
    uint Padding0;
    float3 EmitterWorldPosition;
    float Padding1;
    float3 InitialVelocity;
    float Padding2;
    float4 EmitterColor;
    float SpawnSizeWorldMinimum;
    float SpawnSizeWorldMaximum;
    float PaddingSize0;
    float PaddingSize1;
};

[numthreads(256, 1, 1)]
void Main(uint3 DispatchThreadId : SV_DispatchThreadID)
{
    uint GlobalIndex = DispatchThreadId.x;
    if (GlobalIndex >= SpawnCount)
    {
        return;
    }

    uint Denominator = MaxParticleCount;
    if (Denominator < 1u)
    {
        Denominator = 1u;
    }
    uint ParticleIndex = (BaseIndex + GlobalIndex) % Denominator;

    float RandomValue0 = frac(sin(float(GlobalIndex * 7919u + BaseIndex) * 12.9898) * 43758.5453);
    float RandomValue1 = frac(sin(float(GlobalIndex * 21341u + BaseIndex) * 78.233) * 43758.5453);
    float RandomValue2 = frac(sin(float(GlobalIndex * 3571u + BaseIndex) * 93.989) * 43758.5453);
    float RandomValueSize = frac(sin(float(GlobalIndex * 9241u + BaseIndex) * 19.123) * 37158.121);

    float3 SpreadOffset = float3(
        (RandomValue0 - 0.5) * 0.65,
        (RandomValue1 - 0.5) * 0.65,
        (RandomValue2 - 0.5) * 0.65);

    ParticleStructData Particle;
    Particle.Position = EmitterWorldPosition + SpreadOffset;
    Particle.PositionPadding = 0.0;
    Particle.Velocity = InitialVelocity + float3(RandomValue0 - 0.5, RandomValue1 - 0.5, RandomValue2 - 0.5) * 0.55;
    Particle.VelocityPadding = 0.0;
    Particle.Color = EmitterColor;
    Particle.AgeTime = 0.0;
    Particle.SizeWorld = lerp(SpawnSizeWorldMinimum, SpawnSizeWorldMaximum, RandomValueSize);
    Particle.Active = 1u;
    Particle.PaddingTail = 0u;

    ParticleStateReadWrite[ParticleIndex] = Particle;
}
