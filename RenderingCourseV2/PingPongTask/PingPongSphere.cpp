#include "PingPongSphere.h"

PingPongSphere::PingPongSphere(Game* GameInstance)
	: MeshUniversalComponent(GameInstance)
{
	VertexShaderName = "./Shaders/PingPongPointSphere.hlsl";
	PixelShaderName = "./Shaders/PingPongPointSphere.hlsl";

	Vertices = {
		MeshUniversalVertex{
			DirectX::XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f),
			DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f),
			DirectX::XMFLOAT2(1.0f, 0.0f) },
		MeshUniversalVertex{
			DirectX::XMFLOAT4(-0.5f, -0.5f, 0.5f, 1.0f),
			DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f),
			DirectX::XMFLOAT2(0.0f, 1.0f) },
		MeshUniversalVertex{
			DirectX::XMFLOAT4(0.5f, -0.5f, 0.5f, 1.0f),
			DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f),
			DirectX::XMFLOAT2(1.0f, 1.0f) },
		MeshUniversalVertex{
			DirectX::XMFLOAT4(-0.5f, 0.5f, 0.5f, 1.0f),
			DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
			DirectX::XMFLOAT2(0.0f, 0.0f) }
	};

	Indices = { 0, 1, 2, 1, 0, 3 };
}

void PingPongSphere::Initialize()
{
	MeshUniversalComponent::Initialize();
}

void PingPongSphere::Update(float DeltaTime)
{
	MeshUniversalComponent::Update(DeltaTime);
}
