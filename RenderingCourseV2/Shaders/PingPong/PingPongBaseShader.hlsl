#include "../Abstracts/MeshUniversalShared.hlsli"

PS_IN VSMain( VS_IN input )
{
	PS_IN output = (PS_IN)0;
	
	output.Position = mul(input.Position, WorldViewProjectionMatrix);
	output.Color= input.Color;
	output.TextureCoordinates = input.TextureCoordinates;
	
	return output;
}

float4 PSMain( PS_IN input ) : SV_Target
{
	float4 col = input.Color;
	return col;
}