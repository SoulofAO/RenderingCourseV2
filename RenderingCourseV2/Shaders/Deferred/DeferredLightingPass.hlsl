Texture2D GBufferAlbedo : register(t0);
Texture2D GBufferNormal : register(t1);
Texture2D GBufferMaterial : register(t2);
Texture2D GBufferDepth : register(t3);
Texture2DArray ShadowDepthTexture : register(t4);
SamplerState GBufferSampler : register(s0);
SamplerComparisonState ShadowComparisonSampler : register(s1);

cbuffer DeferredCameraBuffer : register(b0)
{
	float4x4 InverseViewProjectionMatrix;
	float3 CameraWorldPosition;
	float Padding0;
};

cbuffer DeferredLightBuffer : register(b1)
{
	float3 DirectionalLightDirection;
	float DirectionalLightIntensity;
	float4 DirectionalLightColor;
	float UseFullBrightnessWithoutLighting;
	float ShadowBias;
	float ShadowStrength;
	float ShadowMapTexelSize;
	float4 CascadeSplitDepths;
	float4x4 CascadeViewProjectionMatrices[4];
};

struct VS_OUT
{
	float4 Position : SV_POSITION;
	float2 TextureCoordinates : TEXCOORD0;
};

VS_OUT VSMain(uint VertexId : SV_VertexID)
{
	VS_OUT Output = (VS_OUT)0;
	float2 Position = float2((VertexId << 1) & 2, VertexId & 2);
	Output.TextureCoordinates = Position;
	Output.Position = float4(Position * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f), 0.0f, 1.0f);
	return Output;
}

float3 ReconstructWorldPosition(float2 TextureCoordinates, float DepthValue)
{
	float4 ClipPosition = float4(TextureCoordinates * 2.0f - 1.0f, DepthValue, 1.0f);
	float4 WorldPosition = mul(ClipPosition, InverseViewProjectionMatrix);
	return WorldPosition.xyz / max(WorldPosition.w, 0.0001f);
}

float CalculateShadowVisibility(float3 WorldPosition)
{
	float CameraDistance = length(WorldPosition - CameraWorldPosition);
	int CascadeIndex = 0;
	if (CameraDistance > CascadeSplitDepths.x)
	{
		CascadeIndex = 1;
	}
	if (CameraDistance > CascadeSplitDepths.y)
	{
		CascadeIndex = 2;
	}
	if (CameraDistance > CascadeSplitDepths.z)
	{
		CascadeIndex = 3;
	}

	float4 LightClipPosition = mul(float4(WorldPosition, 1.0f), CascadeViewProjectionMatrices[CascadeIndex]);
	float3 LightNormalizedDeviceCoordinates = LightClipPosition.xyz / max(LightClipPosition.w, 0.0001f);
	float2 ShadowTextureCoordinates = LightNormalizedDeviceCoordinates.xy * float2(0.5f, -0.5f) + 0.5f;
	float ShadowDepth = LightNormalizedDeviceCoordinates.z;

	if (
		ShadowTextureCoordinates.x < 0.0f ||
		ShadowTextureCoordinates.x > 1.0f ||
		ShadowTextureCoordinates.y < 0.0f ||
		ShadowTextureCoordinates.y > 1.0f ||
		ShadowDepth < 0.0f ||
		ShadowDepth > 1.0f)
	{
		return 1.0f;
	}

	float2 ShadowSampleOffsets[9] =
	{
		float2(-1.0f, -1.0f),
		float2(0.0f, -1.0f),
		float2(1.0f, -1.0f),
		float2(-1.0f, 0.0f),
		float2(0.0f, 0.0f),
		float2(1.0f, 0.0f),
		float2(-1.0f, 1.0f),
		float2(0.0f, 1.0f),
		float2(1.0f, 1.0f)
	};

	float ShadowVisibility = 0.0f;
	[unroll]
	for (int SampleIndex = 0; SampleIndex < 9; ++SampleIndex)
	{
		const float2 OffsetTextureCoordinates = ShadowTextureCoordinates + (ShadowSampleOffsets[SampleIndex] * ShadowMapTexelSize);
		ShadowVisibility += ShadowDepthTexture.SampleCmpLevelZero(
			ShadowComparisonSampler,
			float3(OffsetTextureCoordinates, CascadeIndex),
			ShadowDepth - ShadowBias);
	}
	ShadowVisibility /= 9.0f;
	return lerp(1.0f, ShadowVisibility, ShadowStrength);
}

float4 PSMain(VS_OUT Input) : SV_Target
{
	float4 Albedo = GBufferAlbedo.Sample(GBufferSampler, Input.TextureCoordinates);
	float4 EncodedNormal = GBufferNormal.Sample(GBufferSampler, Input.TextureCoordinates);
	float4 Material = GBufferMaterial.Sample(GBufferSampler, Input.TextureCoordinates);
	float DepthValue = GBufferDepth.Sample(GBufferSampler, Input.TextureCoordinates).r;

	float3 NormalDirection = normalize(EncodedNormal.xyz * 2.0f - 1.0f);
	float3 WorldPosition = ReconstructWorldPosition(Input.TextureCoordinates, DepthValue);
	float3 LightDirection = normalize(-DirectionalLightDirection);
	float3 ViewDirection = normalize(CameraWorldPosition - WorldPosition);
	float3 HalfDirection = normalize(LightDirection + ViewDirection);

	float Diffuse = max(dot(NormalDirection, LightDirection), 0.0f);
	float SpecularPower = Material.x;
	float SpecularIntensity = Material.y;
	float Specular = pow(max(dot(NormalDirection, HalfDirection), 0.0f), SpecularPower) * SpecularIntensity;
	if (UseFullBrightnessWithoutLighting > 0.5f)
	{
		return Albedo;
	}

	float ShadowVisibility = CalculateShadowVisibility(WorldPosition);
	float3 AmbientColor = Albedo.rgb * 0.15f;
	float3 DiffuseColor = Albedo.rgb * Diffuse * DirectionalLightColor.rgb * DirectionalLightIntensity * ShadowVisibility;
	float3 SpecularColor = Specular * DirectionalLightColor.rgb * DirectionalLightIntensity * ShadowVisibility;
	return float4(AmbientColor + DiffuseColor + SpecularColor, Albedo.a);
}
