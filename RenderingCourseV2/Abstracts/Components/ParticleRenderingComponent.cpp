#include "ParticleRenderingComponent.h"
#include <d3dcompiler.h>
#include "Abstracts/Subsystems/SceneViewportSubsystem.h"
#include "Abstracts/Core/Game.h"

ParticleRenderingComponent::ParticleRenderingComponent()
{
}

ParticleRenderingComponent::~ParticleRenderingComponent()
{
}

void ParticleRenderingComponent::Initialize()
{
    RenderingComponent::Initialize();
    
    SceneViewportSubsystem* SceneViewport = GetOwningGame()->GetSubsystem<SceneViewportSubsystem>();
    
    ID3D11Device* Device = SceneViewport->GetDevice();
    if (Device == nullptr)
    {
        return;
    }

    ID3DBlob* ErrorCode = nullptr;
    auto Result = D3DCompileFromFile(
        L"./Shaders/FirstTask/MyVeryFirstShader.hlsl",
        nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "VSMain",
        "vs_5_0",
        D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
        0,
        &VertexShaderByteCode,
        &ErrorCode);
    

    
}

void ParticleRenderingComponent::Update(float DeltaTime)
{
    RenderingComponent::Update(DeltaTime);
}

void ParticleRenderingComponent::Shutdown()
{
    RenderingComponent::Shutdown();
}
