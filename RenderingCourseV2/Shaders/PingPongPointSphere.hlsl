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

PS_IN VSMain( VS_IN input )
{
	PS_IN output = (PS_IN)0;
	
	output.Position = input.Position;
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

	float SphereDepth = sqrt(1.0f - RadiusSquared);
	float3 SphereNormal = normalize(float3(SphereCoordinates.x, -SphereCoordinates.y, SphereDepth));
	float3 LightDirection = normalize(float3(-0.4f, 0.6f, 1.0f));
	float DiffuseIntensity = saturate(dot(SphereNormal, LightDirection));
	float AmbientIntensity = 0.2f;
	float LightIntensity = AmbientIntensity + DiffuseIntensity * 0.8f;

	float3 FinalColor = input.Color.rgb * LightIntensity;
	return float4(FinalColor, 1.0f);
}