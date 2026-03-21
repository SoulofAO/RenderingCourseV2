#include "Shaders/ParticleSystem/ParticleShared.hlsli"

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
        Particle.Position += Particle.Velocity * DeltaTime;
        Particle.AgeTime += DeltaTime;
    }

    ParticleStateReadWrite[Index] = Particle;
}

