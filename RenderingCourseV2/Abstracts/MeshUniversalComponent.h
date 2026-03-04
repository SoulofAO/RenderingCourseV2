#pragma once

#include "Abstracts/GameComponent.h"
#include <d3d11.h>
#include <directxmath.h>
#include <string>
#include <vector>

struct MeshUniversalVertex
{
	DirectX::XMFLOAT4 Position;
	DirectX::XMFLOAT4 Color;
};

class MeshUniversalComponent : public GameComponent
{
public:
	MeshUniversalComponent(Game* GameInstance);
	~MeshUniversalComponent() override;

	void Initialize() override;
	void Update(float DeltaTime) override;
	void Draw() override;
	void DestroyResources() override;

	std::string VertexShaderName = "";
	std::string PixelShaderName = "";
	std::vector<MeshUniversalVertex> Vertices;
	std::vector<unsigned int> Indices;
	
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
