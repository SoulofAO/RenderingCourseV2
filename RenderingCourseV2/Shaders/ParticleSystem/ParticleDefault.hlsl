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
        if (Particle.LifetimeSeconds > 0.0 && Particle.AgeTime >= Particle.LifetimeSeconds)
        {
            Particle.Active = 0u;
        }
    }

    ParticleStateReadWrite[Index] = Particle;
}

