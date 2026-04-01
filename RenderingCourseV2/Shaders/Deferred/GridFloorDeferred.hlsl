#include "../Abstracts/MeshUniversalShared.hlsli"

struct GBUFFER_OUT
{
	float4 Albedo : SV_Target0;
	float4 Normal : SV_Target1;
	float4 Material : SV_Target2;
};

PS_IN VSMain(VS_IN Input)
{
	PS_IN Output = (PS_IN)0;
	Output.Position = mul(Input.Position, WorldViewProjectionMatrix);
	Output.Color = Input.Color;
	Output.TextureCoordinates = Input.TextureCoordinates;

	float4 WorldPosition = mul(Input.Position, WorldMatrix);
	Output.WorldPosition = WorldPosition.xyz;
	Output.WorldNormal = normalize(mul(float4(Input.Normal, 0.0f), WorldMatrix).xyz);
	return Output;
}

GBUFFER_OUT PSMain(PS_IN Input)
{
	GBUFFER_OUT Output = (GBUFFER_OUT)0;

	float3 AlbedoColor = Input.Color.rgb * BaseColor.rgb;
	Output.Albedo = float4(AlbedoColor, BaseColor.a);
	Output.Normal = float4(normalize(Input.WorldNormal) * 0.5f + 0.5f, 1.0f);
	Output.Material = float4(8.0f, 0.0f, 0.0f, 1.0f);
	return Output;
}
