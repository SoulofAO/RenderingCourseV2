struct ParticleStructData
{
    float3 Position;
    float PositionPadding;
    float3 Velocity;
    float VelocityPadding;
    float4 Color;
    float AgeTime;
    float SizeWorld;
    float LifetimeSeconds;
    uint Active;
    uint SpawnId;
    uint2 DataPadding;
};
