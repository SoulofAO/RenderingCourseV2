#include "Shaders/ParticleSystem/ParticleRenderShared.hlsli"

ParticleDrawPixelInput VSMain(ParticleDrawVertexInput Input)
{
    ParticleDrawPixelInput Output = (ParticleDrawPixelInput)0;
    if (Input.InstanceId >= ParticleDrawCount)
    {
        Output.Position = float4(0.0, 0.0, -1.0, 1.0);
        Output.Color = float4(0.0, 0.0, 0.0, 0.0);
        Output.TextureCoordinates = float2(0.0, 0.0);
        return Output;
    }
    ParticleStructData Particle = ParticleStateReadOnly[Input.InstanceId];
    if (Particle.Active == 0)
    {
        Output.Position = float4(0.0, 0.0, -1.0, 1.0);
        Output.Color = float4(0.0, 0.0, 0.0, 0.0);
        Output.TextureCoordinates = float2(0.0, 0.0);
        return Output;
    }
    float2 Corner = float2((Input.VertexId << 1) & 2, Input.VertexId & 2);
    float2 Direction = Corner * 2.0 - 1.0;
    float BillboardSize = Particle.SizeWorld > 0.0001 ? Particle.SizeWorld : ParticleSize;
    float3 WorldPosition = Particle.Position + CameraRightWorld * Direction.x * BillboardSize + CameraUpWorld * Direction.y * BillboardSize;
    float4 ClipPosition = mul(float4(WorldPosition, 1.0), ViewProjectionMatrix);
    Output.Position = ClipPosition;
    Output.Color = Particle.Color;
    Output.TextureCoordinates = Corner;
    return Output;
}

float4 PSMain(ParticleDrawPixelInput Input) : SV_Target
{
    if (Input.Color.a <= 0.0)
    {
        discard;
    }
    float2 SphereCoordinates = Input.TextureCoordinates * 2.0 - 1.0;
    float RadiusSquared = dot(SphereCoordinates, SphereCoordinates);
    float Alpha = (RadiusSquared * -1 + 1) * Input.Color.a;
    float3 FinalColor = Input.Color.rgb;
    return float4(FinalColor, Alpha);
}
