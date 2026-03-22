#include "Shaders/ParticleSystem/ParticleStructShared.hlsli"

struct ParticleSortData
{
    float SortKey;
    uint OriginalIndex;
};

StructuredBuffer<ParticleStructData> ParticleStateReadOnly : register(t0);
StructuredBuffer<ParticleSortData> ParticleSortReadOnly : register(t1);

cbuffer ParticleMaterialConstants : register(b0)
{
    float4x4 ViewProjectionMatrix;
    float3 CameraRightWorld;
    float ParticleSize;
    float3 CameraUpWorld;
    float Padding0;
    uint ParticleDrawCount;
    uint UseParticleSort;
    uint2 Padding1;
};

struct ParticleDrawVertexInput
{
    uint VertexId : SV_VertexID;
    uint InstanceId : SV_InstanceID;
};

struct ParticleDrawPixelInput
{
    float4 Position : SV_POSITION;
    float4 Color : COLOR0;
    float2 TextureCoordinates : TEXCOORD0;
};
