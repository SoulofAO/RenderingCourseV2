Texture2D GBufferAlbedo : register(t0);
Texture2D GBufferNormal : register(t1);
Texture2D GBufferMaterial : register(t2);
Texture2D GBufferDepth : register(t3);
Texture2DArray ShadowDepthTexture : register(t4);
SamplerState GBufferSampler : register(s0);
SamplerComparisonState ShadowComparisonSampler : register(s1);

cbuffer DeferredCameraBuffer : register(b0)
{
	float4x4 ViewMatrix;
	float4x4 InverseViewProjectionMatrix;
	float3 CameraWorldPosition;
	float Padding0;
};

struct DeferredPointLightData
{
	float3 Position;
	float Intensity;
	float4 Color;
	float Range;
	float3 Padding0;
};

struct DeferredSpotLightData
{
	float3 Position;
	float Intensity;
	float4 Color;
	float3 Direction;
	float Range;
	float InnerConeAngleCosine;
	float OuterConeAngleCosine;
	float2 Padding0;
};

cbuffer DeferredLightBuffer : register(b1)
{
	float3 DirectionalLightDirection;
	float DirectionalLightIntensity;
	float4 DirectionalLightColor;
	float PointLightCountValue;
	float SpotLightCountValue;
	float2 LightCountPadding0;
	DeferredPointLightData PointLights[16];
	DeferredSpotLightData SpotLights[16];
	float UseFullBrightnessWithoutLighting;
	float ShadowBias;
	float ShadowStrength;
	float ShadowMapTexelSize;
	float4 CascadeSplitDepths;
	float ShadowCascadeCountValue;
	float3 ShadowCascadeCountValuePadding;
	float4x4 CascadeViewProjectionMatrices[4];
	float DeferredDebugBufferViewMode;
	float3 DeferredDebugBufferViewModePadding;
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
	float2 ClipCoordinates = float2(TextureCoordinates.x * 2.0f - 1.0f, 1.0f - TextureCoordinates.y * 2.0f);
	float4 ClipPosition = float4(ClipCoordinates, DepthValue, 1.0f);
	float4 WorldPosition = mul(ClipPosition, InverseViewProjectionMatrix);
	return WorldPosition.xyz / max(WorldPosition.w, 0.0001f);
}

float CalculateShadowVisibility(float3 WorldPosition)
{
	float ViewSpaceDepth = mul(float4(WorldPosition, 1.0f), ViewMatrix).z;
	ViewSpaceDepth = max(ViewSpaceDepth, 0.0f);
	int MaximumCascadeIndex = clamp((int)ShadowCascadeCountValue - 1, 0, 3);
	float MaximumShadowDepth = CascadeSplitDepths[MaximumCascadeIndex];
	if (ViewSpaceDepth > MaximumShadowDepth)
	{
		return 1.0f;
	}
	int CascadeIndex = 0;
	if (MaximumCascadeIndex >= 1 && ViewSpaceDepth > CascadeSplitDepths.x)
	{
		CascadeIndex = 1;
	}
	if (MaximumCascadeIndex >= 2 && ViewSpaceDepth > CascadeSplitDepths.y)
	{
		CascadeIndex = 2;
	}
	if (MaximumCascadeIndex >= 3 && ViewSpaceDepth > CascadeSplitDepths.z)
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

float3 CalculatePointLightContribution(
	float3 WorldPosition,
	float3 NormalDirection,
	float3 ViewDirection,
	float3 AlbedoColor,
	float SpecularPower,
	float SpecularIntensity,
	DeferredPointLightData PointLightData)
{
	float3 SurfaceToLightVector = PointLightData.Position - WorldPosition;
	float SurfaceToLightDistance = length(SurfaceToLightVector);
	float SafeRange = max(PointLightData.Range, 0.001f);
	if (SurfaceToLightDistance >= SafeRange)
	{
		return float3(0.0f, 0.0f, 0.0f);
	}

	float3 LightDirection = SurfaceToLightVector / max(SurfaceToLightDistance, 0.0001f);
	float3 HalfDirection = normalize(LightDirection + ViewDirection);
	float Diffuse = max(dot(NormalDirection, LightDirection), 0.0f);
	float Specular = pow(max(dot(NormalDirection, HalfDirection), 0.0f), SpecularPower) * SpecularIntensity;
	float RangeAttenuation = saturate(1.0f - (SurfaceToLightDistance / SafeRange));
	RangeAttenuation *= RangeAttenuation;
	float3 DiffuseColor = AlbedoColor * Diffuse * PointLightData.Color.rgb * PointLightData.Intensity * RangeAttenuation;
	float3 SpecularColor = Specular * PointLightData.Color.rgb * PointLightData.Intensity * RangeAttenuation;
	return DiffuseColor + SpecularColor;
}

float3 CalculateSpotLightContribution(
	float3 WorldPosition,
	float3 NormalDirection,
	float3 ViewDirection,
	float3 AlbedoColor,
	float SpecularPower,
	float SpecularIntensity,
	DeferredSpotLightData SpotLightData)
{
	float3 SurfaceToLightVector = SpotLightData.Position - WorldPosition;
	float SurfaceToLightDistance = length(SurfaceToLightVector);
	float SafeRange = max(SpotLightData.Range, 0.001f);
	if (SurfaceToLightDistance >= SafeRange)
	{
		return float3(0.0f, 0.0f, 0.0f);
	}

	float3 LightDirection = SurfaceToLightVector / max(SurfaceToLightDistance, 0.0001f);
	float3 SpotDirection = normalize(SpotLightData.Direction);
	float3 LightToSurfaceDirection = normalize(WorldPosition - SpotLightData.Position);
	float ConeCosine = dot(SpotDirection, LightToSurfaceDirection);
	float ConeAttenuation = smoothstep(SpotLightData.OuterConeAngleCosine, SpotLightData.InnerConeAngleCosine, ConeCosine);
	if (ConeAttenuation <= 0.0001f)
	{
		return float3(0.0f, 0.0f, 0.0f);
	}

	float3 HalfDirection = normalize(LightDirection + ViewDirection);
	float Diffuse = max(dot(NormalDirection, LightDirection), 0.0f);
	float Specular = pow(max(dot(NormalDirection, HalfDirection), 0.0f), SpecularPower) * SpecularIntensity;
	float RangeAttenuation = saturate(1.0f - (SurfaceToLightDistance / SafeRange));
	RangeAttenuation *= RangeAttenuation;
	float Attenuation = RangeAttenuation * ConeAttenuation;
	float3 DiffuseColor = AlbedoColor * Diffuse * SpotLightData.Color.rgb * SpotLightData.Intensity * Attenuation;
	float3 SpecularColor = Specular * SpotLightData.Color.rgb * SpotLightData.Intensity * Attenuation;
	return DiffuseColor + SpecularColor;
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

	if (DeferredDebugBufferViewMode > 0.5f && DeferredDebugBufferViewMode < 1.5f)
	{
		return float4(Albedo.rgb, 1.0f);
	}
	if (DeferredDebugBufferViewMode > 1.5f && DeferredDebugBufferViewMode < 2.5f)
	{
		return float4(NormalDirection * 0.5f + 0.5f, 1.0f);
	}
	if (DeferredDebugBufferViewMode > 2.5f && DeferredDebugBufferViewMode < 3.5f)
	{
		float MaterialSpecularPowerNormalized = saturate(SpecularPower / 128.0f);
		float MaterialSpecularIntensityNormalized = saturate(SpecularIntensity);
		return float4(MaterialSpecularPowerNormalized, MaterialSpecularIntensityNormalized, 0.0f, 1.0f);
	}
	if (DeferredDebugBufferViewMode > 3.5f && DeferredDebugBufferViewMode < 4.5f)
	{
		return float4(DepthValue, DepthValue, DepthValue, 1.0f);
	}
	if (UseFullBrightnessWithoutLighting > 0.5f)
	{
		return Albedo;
	}

	float ShadowVisibility = CalculateShadowVisibility(WorldPosition);
	if (DeferredDebugBufferViewMode > 4.5f && DeferredDebugBufferViewMode < 5.5f)
	{
		return float4(ShadowVisibility, ShadowVisibility, ShadowVisibility, 1.0f);
	}
	float3 AmbientColor = Albedo.rgb * 0.15f;
	float3 DirectionalDiffuseColor = Albedo.rgb * Diffuse * DirectionalLightColor.rgb * DirectionalLightIntensity * ShadowVisibility;
	float3 DirectionalSpecularColor = Specular * DirectionalLightColor.rgb * DirectionalLightIntensity * ShadowVisibility;
	float3 AdditionalLightsColor = float3(0.0f, 0.0f, 0.0f);
	int PointLightCount = clamp((int)PointLightCountValue, 0, 16);
	[loop]
	for (int PointLightIndex = 0; PointLightIndex < PointLightCount; ++PointLightIndex)
	{
		AdditionalLightsColor += CalculatePointLightContribution(
			WorldPosition,
			NormalDirection,
			ViewDirection,
			Albedo.rgb,
			SpecularPower,
			SpecularIntensity,
			PointLights[PointLightIndex]);
	}
	int SpotLightCount = clamp((int)SpotLightCountValue, 0, 16);
	[loop]
	for (int SpotLightIndex = 0; SpotLightIndex < SpotLightCount; ++SpotLightIndex)
	{
		AdditionalLightsColor += CalculateSpotLightContribution(
			WorldPosition,
			NormalDirection,
			ViewDirection,
			Albedo.rgb,
			SpecularPower,
			SpecularIntensity,
			SpotLights[SpotLightIndex]);
	}
	return float4(AmbientColor + DirectionalDiffuseColor + DirectionalSpecularColor + AdditionalLightsColor, Albedo.a);
}
