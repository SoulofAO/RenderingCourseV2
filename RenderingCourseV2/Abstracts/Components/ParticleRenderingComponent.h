#pragma once
#include "RenderingComponent.h"
#include <d3d11.h>


class ParticleRenderingComponent : public RenderingComponent
{
public:
    ParticleRenderingComponent();
    ~ParticleRenderingComponent() override;
    
    void Initialize() override;
    void Update(float DeltaTime) override;
    void Shutdown() override;
public:
    ID3D11InputLayout* Layout;
    ID3D11VertexShader* VertexShader;
    ID3DBlob* VertexShaderByteCode;
    ID3D11PixelShader* PixelShader;
    ID3DBlob* PixelShaderByteCode;
    ID3D11Buffer* VertexBuffer;
    ID3D11Buffer* IndexBuffer;
    ID3D11RasterizerState* RasterState;
    UINT IndexCount;
};
