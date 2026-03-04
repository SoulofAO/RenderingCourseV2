#include "PingPongSphere.h"
#include "Abstracts/Components/MeshUniversalComponent.h"
#include "Abstracts/Components/PhysicsComponent.h"

PingPongSphere::PingPongSphere()
	: Actor()
{
	std::unique_ptr<MeshUniversalComponent> MeshComponent = std::make_unique<MeshUniversalComponent>();
	MeshComponent->VertexShaderName = "./Shaders/PingPong/PingPongPointSphere.hlsl";
	MeshComponent->PixelShaderName = "./Shaders/PingPong/PingPongPointSphere.hlsl";

	MeshComponent->Vertices = {
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

	MeshComponent->Indices = { 0, 1, 2, 1, 0, 3 };
	AddComponent(std::move(MeshComponent));

	std::unique_ptr<PhysicsComponent> Physics = std::make_unique<PhysicsComponent>();
	Physics->SetSphereCollider(0.5f);
	Physics->SetMass(1.0f);
	Physics->SetRestitution(0.85f);
	Physics->SetVelocity(DirectX::XMFLOAT3(0.8f, -0.25f, 0.0f));
	AddComponent(std::move(Physics));
}
