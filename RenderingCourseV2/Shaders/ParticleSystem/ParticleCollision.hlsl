#include "Shaders/ParticleSystem/ParticleShared.hlsli"

Texture2D SceneDepthTexture : register(t1);
SamplerState SceneDepthSampler : register(s0);

cbuffer ParticleCollisionConstants : register(b1)
{
    float4x4 ViewProjectionMatrix;
    float4x4 InverseViewProjectionMatrix;
    float3 CameraWorldPosition;
    float DepthBias;
    float SurfaceOffset;
    float BounceRestitution;
    float NormalSampleDistanceScale;
    float Padding0;
    float2 ScreenSize;
    float2 Padding1;
};

float3 ReconstructWorldPositionFromDepth(float2 TextureCoordinates, float DepthValue)
{
    float2 ClipCoordinates = float2(TextureCoordinates.x * 2.0 - 1.0, 1.0 - TextureCoordinates.y * 2.0);
    float4 ClipPosition = float4(ClipCoordinates, DepthValue, 1.0);
    float4 WorldPosition = mul(ClipPosition, InverseViewProjectionMatrix);
    return WorldPosition.xyz / max(WorldPosition.w, 0.0001);
}

float3 SampleWorldPositionFromDepth(float2 TextureCoordinates)
{
    float DepthValue = SceneDepthTexture.SampleLevel(SceneDepthSampler, TextureCoordinates, 0).r;
    return ReconstructWorldPositionFromDepth(TextureCoordinates, DepthValue);
}

[numthreads(256, 1, 1)]
void Main(uint3 DispatchThreadId : SV_DispatchThreadID)
{
    uint Index = DispatchThreadId.x;

    if (Index >= ParticleCount)
    {
        return;
    }

    ParticleStructData Particle = ParticleStateReadWrite[Index];

    if (Particle.Active == 0)
    {
        ParticleStateReadWrite[Index] = Particle;
        return;
    }

    float4 ClipPosition = mul(float4(Particle.Position, 1.0), ViewProjectionMatrix);
    if (ClipPosition.w <= 0.0001)
    {
        ParticleStateReadWrite[Index] = Particle;
        return;
    }

    float3 NormalizedDeviceCoordinates = ClipPosition.xyz / ClipPosition.w;
    float2 TextureCoordinates = float2(NormalizedDeviceCoordinates.x * 0.5 + 0.5, 0.5 - NormalizedDeviceCoordinates.y * 0.5);

    if (TextureCoordinates.x < 0.0 || TextureCoordinates.x > 1.0 || TextureCoordinates.y < 0.0 || TextureCoordinates.y > 1.0)
    {
        ParticleStateReadWrite[Index] = Particle;
        return;
    }

    float SceneDepthSample = SceneDepthTexture.SampleLevel(SceneDepthSampler, TextureCoordinates, 0).r;
    float ParticleDepth = NormalizedDeviceCoordinates.z;

    if (ParticleDepth <= SceneDepthSample + DepthBias)
    {
        ParticleStateReadWrite[Index] = Particle;
        return;
    }

    float3 WorldSurfacePosition = ReconstructWorldPositionFromDepth(TextureCoordinates, SceneDepthSample);
    float3 TowardCamera = normalize(CameraWorldPosition - WorldSurfacePosition);
    Particle.Position = WorldSurfacePosition + TowardCamera * SurfaceOffset;

    float2 TexelSize = float2(1.0 / max(ScreenSize.x, 1.0), 1.0 / max(ScreenSize.y, 1.0));
    float2 OffsetX = float2(TexelSize.x * NormalSampleDistanceScale, 0.0);
    float2 OffsetY = float2(0.0, TexelSize.y * NormalSampleDistanceScale);
    float3 WorldPositionX = SampleWorldPositionFromDepth(TextureCoordinates + OffsetX);
    float3 WorldPositionY = SampleWorldPositionFromDepth(TextureCoordinates + OffsetY);
    float3 SurfaceTangentX = WorldPositionX - WorldSurfacePosition;
    float3 SurfaceTangentY = WorldPositionY - WorldSurfacePosition;
    float3 NormalCandidate = cross(SurfaceTangentX, SurfaceTangentY);
    float NormalLength = length(NormalCandidate);
    float3 SurfaceNormal = TowardCamera;
    if (NormalLength > 0.0001)
    {
        SurfaceNormal = NormalCandidate / NormalLength;
    }
    float3 FromCameraToSurface = WorldSurfacePosition - CameraWorldPosition;
    if (dot(SurfaceNormal, FromCameraToSurface) < 0.0)
    {
        SurfaceNormal = -SurfaceNormal;
    }

    Particle.Velocity = reflect(Particle.Velocity, SurfaceNormal) * BounceRestitution;

    ParticleStateReadWrite[Index] = Particle;
}
