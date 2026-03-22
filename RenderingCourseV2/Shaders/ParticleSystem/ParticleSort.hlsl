#include "Shaders/ParticleSystem/ParticleStructShared.hlsli"

struct ParticleSortData
{
    float SortKey;
    uint OriginalIndex;
};

cbuffer FillParticleSortConstants : register(b0)
{
    float3 CameraWorldPosition;
    float PaddingFill0;
    uint MaxParticleCount;
    uint PaddedParticleCount;
    uint2 PaddingFill1;
};

StructuredBuffer<ParticleStructData> ParticleStateReadOnly : register(t0);
RWStructuredBuffer<ParticleSortData> ParticleSortDataReadWrite : register(u0);

[numthreads(256, 1, 1)]
void FillParticleSortKeys(uint3 DispatchThreadId : SV_DispatchThreadID)
{
    uint ThreadIndex = DispatchThreadId.x;
    if (ThreadIndex >= PaddedParticleCount)
    {
        return;
    }

    if (ThreadIndex >= MaxParticleCount)
    {
        ParticleSortDataReadWrite[ThreadIndex].SortKey = 1e30;
        ParticleSortDataReadWrite[ThreadIndex].OriginalIndex = ThreadIndex;
        return;
    }

    ParticleStructData Particle = ParticleStateReadOnly[ThreadIndex];
    if (Particle.Active == 0u)
    {
        ParticleSortDataReadWrite[ThreadIndex].SortKey = 1e30;
        ParticleSortDataReadWrite[ThreadIndex].OriginalIndex = ThreadIndex;
        return;
    }

    float3 Delta = Particle.Position - CameraWorldPosition;
    float DistanceSquared = dot(Delta, Delta);
    ParticleSortDataReadWrite[ThreadIndex].SortKey = -DistanceSquared;
    ParticleSortDataReadWrite[ThreadIndex].OriginalIndex = ThreadIndex;
}

cbuffer BitonicSortConstants : register(b0)
{
    uint ElementCount;
    uint PhaseK;
    uint PhaseJ;
    uint PaddingBitonic;
};

[numthreads(256, 1, 1)]
void BitonicSortStep(uint3 DispatchThreadId : SV_DispatchThreadID)
{
    uint Index = DispatchThreadId.x;
    if (Index >= ElementCount)
    {
        return;
    }

    uint Partner = Index ^ PhaseJ;
    if (Partner <= Index)
    {
        return;
    }

    if (Partner >= ElementCount)
    {
        return;
    }

    ParticleSortData ValueIndex = ParticleSortDataReadWrite[Index];
    ParticleSortData ValuePartner = ParticleSortDataReadWrite[Partner];
    bool Ascending = ((Index & PhaseK) == 0u);
    bool Swap = false;
    if (Ascending)
    {
        if (ValueIndex.SortKey > ValuePartner.SortKey)
        {
            Swap = true;
        }
        else if (ValueIndex.SortKey == ValuePartner.SortKey && ValueIndex.OriginalIndex > ValuePartner.OriginalIndex)
        {
            Swap = true;
        }
    }
    else
    {
        if (ValueIndex.SortKey < ValuePartner.SortKey)
        {
            Swap = true;
        }
        else if (ValueIndex.SortKey == ValuePartner.SortKey && ValueIndex.OriginalIndex < ValuePartner.OriginalIndex)
        {
            Swap = true;
        }
    }

    if (Swap)
    {
        ParticleSortDataReadWrite[Index] = ValuePartner;
        ParticleSortDataReadWrite[Partner] = ValueIndex;
    }
}
