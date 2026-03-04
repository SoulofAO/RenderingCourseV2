#include "PingPongPlane.h"
#include "Abstracts/Components/MeshUniversalComponent.h"
#include "Abstracts/Components/PhysicsComponent.h"

PingPongPlane::PingPongPlane()
	: Actor()
{
	std::unique_ptr<MeshUniversalComponent> MeshComponent = std::make_unique<MeshUniversalComponent>();
	MeshComponent->VertexShaderName = "./Shaders/PingPong/PingPongBaseShader.hlsl";
	MeshComponent->PixelShaderName = "./Shaders/PingPong/PingPongBaseShader.hlsl";

	MeshComponent->Vertices = {
		MeshUniversalVertex{
			DirectX::XMFLOAT4(0.5f, 0.2f, 0.5f, 1.0f),
			DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f),
			DirectX::XMFLOAT2(1.0f, 0.0f) },
		MeshUniversalVertex{
			DirectX::XMFLOAT4(-0.5f, -0.2f, 0.5f, 1.0f),
			DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f),
			DirectX::XMFLOAT2(0.0f, 1.0f) },
		MeshUniversalVertex{
			DirectX::XMFLOAT4(0.5f, -0.2f, 0.5f, 1.0f),
			DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f),
			DirectX::XMFLOAT2(1.0f, 1.0f) },
		MeshUniversalVertex{
			DirectX::XMFLOAT4(-0.5f, 0.2f, 0.5f, 1.0f),
			DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f),
			DirectX::XMFLOAT2(0.0f, 0.0f) }
	};

	MeshComponent->Indices = { 0, 1, 2, 1, 0, 3 };
	AddComponent(std::move(MeshComponent));

	std::unique_ptr<PhysicsComponent> Physics = std::make_unique<PhysicsComponent>();
	Physics->SetAabbCollider(DirectX::XMFLOAT3(3.0f, 0.2f, 0.2f));
	Physics->SetMass(0.0f);
	Physics->SetIsStatic(true);
	Physics->SetRestitution(0.9f);
	AddComponent(std::move(Physics));
}
