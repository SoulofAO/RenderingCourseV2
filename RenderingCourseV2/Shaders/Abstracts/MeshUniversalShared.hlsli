cbuffer TransformConstantBuffer : register(b0)
{
    float4x4 WorldViewProjectionMatrix;
};

struct VS_IN
{
    float4 Position : POSITION0;
    float4 Color : COLOR0;
    float2 TextureCoordinates : TEXCOORD0;
};

struct PS_IN
{
    float4 Position : SV_POSITION;
    float4 Color : COLOR;
    float2 TextureCoordinates : TEXCOORD0;
};
