#include "PingPongSphere.h"
#include "Engine/Core/Runtime/Abstract/Components/MeshUniversalComponent.h"

PingPongSphere::PingPongSphere()
	: Actor()
{
	std::unique_ptr<MeshUniversalComponent> MeshComponent = std::make_unique<MeshUniversalComponent>();
	MeshComponent->VertexShaderName = "./Shaders/PingPong/PingPongPointSphere.hlsl";
	MeshComponent->PixelShaderName = "./Shaders/PingPong/PingPongPointSphere.hlsl";

	MeshComponent->Vertices = {
		MeshUniversalVertex{
			DirectX::XMFLOAT4(0.08f, 0.08f, 0.5f, 1.0f),
			DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f),
			DirectX::XMFLOAT2(1.0f, 0.0f) },
		MeshUniversalVertex{
			DirectX::XMFLOAT4(-0.08f, -0.08f, 0.5f, 1.0f),
			DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f),
			DirectX::XMFLOAT2(0.0f, 1.0f) },
		MeshUniversalVertex{
			DirectX::XMFLOAT4(0.08f, -0.08f, 0.5f, 1.0f),
			DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f),
			DirectX::XMFLOAT2(1.0f, 1.0f) },
		MeshUniversalVertex{
			DirectX::XMFLOAT4(-0.08f, 0.08f, 0.5f, 1.0f),
			DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f),
			DirectX::XMFLOAT2(0.0f, 0.0f) }
	};

	MeshComponent->Indices = { 0, 1, 2, 1, 0, 3 };
	AddComponent(std::move(MeshComponent));
}

