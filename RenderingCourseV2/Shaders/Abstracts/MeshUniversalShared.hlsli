cbuffer TransformConstantBuffer : register(b0)
{
    float4x4 WorldViewProjectionMatrix;
    float4x4 WorldMatrix;
    float3 CameraWorldPosition;
    float Padding0;
};

cbuffer LightConstantBuffer : register(b1)
{
    float3 DirectionalLightDirection;
    float DirectionalLightIntensity;
    float4 DirectionalLightColor;
    float UseFullBrightnessWithoutLighting;
    float3 Padding1;
};

cbuffer MaterialConstantBuffer : register(b2)
{
    float4 BaseColor;
    float SpecularPower;
    float SpecularIntensity;
    float UseAlbedoTexture;
    float UseNormalTexture;
    float UseShadowedAlbedoTexture;
    float3 MaterialPadding0;
};

struct VS_IN
{
    float4 Position : POSITION0;
    float4 Color : COLOR0;
    float3 Normal : NORMAL0;
    float3 Tangent : TANGENT0;
    float2 TextureCoordinates : TEXCOORD0;
};

struct PS_IN
{
    float4 Position : SV_POSITION;
    float4 Color : COLOR;
    float3 WorldPosition : TEXCOORD1;
    float3 WorldNormal : TEXCOORD2;
    float2 TextureCoordinates : TEXCOORD0;
};

Texture2D AlbedoTexture : register(t0);
Texture2D ShadowedAlbedoTexture : register(t1);
Texture2D NormalTexture : register(t2);
Texture2D SpecularTexture : register(t3);
Texture2D EmissiveTexture : register(t4);
SamplerState DefaultSampler : register(s0);
