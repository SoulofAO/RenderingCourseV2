#include "MeshUniversalShared.hlsli"

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

float4 PSMain(PS_IN Input) : SV_Target
{
	float3 NormalDirection = normalize(Input.WorldNormal);
	float3 LightDirection = normalize(-DirectionalLightDirection);
	float3 ViewDirection = normalize(CameraWorldPosition - Input.WorldPosition);
	float3 HalfDirection = normalize(LightDirection + ViewDirection);

	float DiffuseFactor = max(dot(NormalDirection, LightDirection), 0.0f);
	float SpecularFactor = pow(max(dot(NormalDirection, HalfDirection), 0.0f), SpecularPower) * SpecularIntensity;

	float4 TextureColor = UseAlbedoTexture > 0.5f ? AlbedoTexture.Sample(DefaultSampler, Input.TextureCoordinates) : float4(1.0f, 1.0f, 1.0f, 1.0f);
	float3 BaseSurfaceColor = Input.Color.rgb * BaseColor.rgb * TextureColor.rgb;
	if (UseFullBrightnessWithoutLighting > 0.5f)
	{
		return float4(BaseSurfaceColor, BaseColor.a * TextureColor.a);
	}

	float3 AmbientColor = BaseSurfaceColor * 0.15f;
	float3 DiffuseColor = BaseSurfaceColor * DiffuseFactor * DirectionalLightColor.rgb * DirectionalLightIntensity;
	float3 SpecularColor = SpecularFactor * DirectionalLightColor.rgb * DirectionalLightIntensity;
	float3 FinalColor = AmbientColor + DiffuseColor + SpecularColor;
	return float4(FinalColor, BaseColor.a * TextureColor.a);
}
