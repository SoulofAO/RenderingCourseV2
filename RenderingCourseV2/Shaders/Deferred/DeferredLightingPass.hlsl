Texture2D GBufferAlbedo : register(t0);
Texture2D GBufferNormal : register(t1);
Texture2D GBufferMaterial : register(t2);
Texture2D GBufferDepth : register(t3);
SamplerState GBufferSampler : register(s0);

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
	float3 Padding0;
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

	float3 AmbientColor = Albedo.rgb * 0.15f;
	float3 DiffuseColor = Albedo.rgb * Diffuse * DirectionalLightColor.rgb * DirectionalLightIntensity;
	float3 SpecularColor = Specular * DirectionalLightColor.rgb * DirectionalLightIntensity;
	return float4(AmbientColor + DiffuseColor + SpecularColor, Albedo.a);
}
