#include "PingPongPlane.h"

void PingPongPlane::Initialize()
{
    MeshUniversalComponent::Initialize();
    
    Vertices = {
        MeshUniversalVertex{ DirectX::XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f), DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },
        MeshUniversalVertex{ DirectX::XMFLOAT4(-0.5f, -0.5f, 0.5f, 1.0f), DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) },
        MeshUniversalVertex{ DirectX::XMFLOAT4(0.5f, -0.5f, 0.5f, 1.0f), DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },
        MeshUniversalVertex{ DirectX::XMFLOAT4(-0.5f, 0.5f, 0.5f, 1.0f), DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f) }
    };

    Indices = { 0, 1, 2, 1, 0, 3 };
    
    std::string VertexShaderName = "";
    std::string PixelShaderName = "";
}

void PingPongPlane::Update(float DeltaTime)
{
    MeshUniversalComponent::Update(DeltaTime);
}
