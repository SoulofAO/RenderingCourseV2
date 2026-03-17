#include "../Abstracts/MeshUniversalShared.hlsli"

PS_IN VSMain( VS_IN input )
{
	PS_IN output = (PS_IN)0;
	
	output.Position = mul(input.Position, WorldViewProjectionMatrix);
	output.Color= input.Color;
	output.TextureCoordinates = input.TextureCoordinates;
    float4 WorldPosition = mul(input.Position, WorldMatrix);
    output.WorldPosition = WorldPosition.xyz;
    output.WorldNormal = normalize(mul(float4(input.Normal, 0.0f), WorldMatrix).xyz);
	
	return output;
}

float4 PSMain( PS_IN input ) : SV_Target
{
	float4 col = input.Color;
	return col;
}