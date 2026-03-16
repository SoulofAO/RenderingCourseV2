#include "../Abstracts/MeshUniversalShared.hlsli"

PS_IN VSMain( VS_IN input )
{
	PS_IN output = (PS_IN)0;
	
	output.Position = mul(input.Position, WorldViewProjectionMatrix);
	output.Color = input.Color;
	output.TextureCoordinates = input.TextureCoordinates;
	
	return output;
}

float4 PSMain( PS_IN input ) : SV_Target
{
	float2 SphereCoordinates = input.TextureCoordinates * 2.0f - 1.0f;
	float RadiusSquared = dot(SphereCoordinates, SphereCoordinates);
	if (RadiusSquared > 1.0f)
	{
		discard;
	}

	float3 FinalColor = input.Color.rgb;
	return float4(FinalColor, 1.0f);
}