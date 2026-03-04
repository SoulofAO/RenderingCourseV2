#include "PingPongPlane.h"

PingPongPlane::PingPongPlane(Game* GameInstance)
	: MeshUniversalComponent(GameInstance)
{
	VertexShaderName = "./Shaders/PingPong/PingPongPointSphere.hlsl";
	PixelShaderName = "./Shaders/PingPong/PingPongPointSphere.hlsl";

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

void PingPongPlane::Initialize()
{
	MeshUniversalComponent::Initialize();
}

void PingPongPlane::Update(float DeltaTime)
{
	MeshUniversalComponent::Update(DeltaTime);
}
