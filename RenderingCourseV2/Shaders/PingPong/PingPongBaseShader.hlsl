#include "../Abstracts/MeshUniversalShared.hlsli"

PS_IN VSMain( VS_IN input )
{
	PS_IN output = (PS_IN)0;
	
	output.Position = input.Position;
	output.Color= input.Color;
	
	return output;
}

float4 PSMain( PS_IN input ) : SV_Target
{
	float4 col = input.Color;
	return col;
}