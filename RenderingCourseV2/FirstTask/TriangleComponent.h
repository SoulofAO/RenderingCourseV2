#pragma once

#include "..//Abstracts//GameComponent.h"
#include <d3d11.h>

class TriangleComponent : public GameComponent
{
public:
	TriangleComponent(Game* GameInstance);
	~TriangleComponent() override;

	void Initialize() override;
	void Update(float DeltaTime) override;
	void Draw() override;
	void DestroyResources() override;

private:
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
