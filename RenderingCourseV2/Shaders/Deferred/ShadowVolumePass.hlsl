cbuffer ShadowVolumeTransformBuffer : register(b0)
{
	float4x4 WorldViewProjectionMatrix;
};

struct VS_IN
{
	float4 Position : POSITION0;
};

struct VS_OUT
{
	float4 Position : SV_POSITION;
};

VS_OUT VSMain(VS_IN Input)
{
	VS_OUT Output = (VS_OUT)0;
	Output.Position = mul(Input.Position, WorldViewProjectionMatrix);
	return Output;
}
