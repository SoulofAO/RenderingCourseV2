#pragma once

#include "Engine/Core/Runtime/Abstract/Components/RenderingComponent.h"
#include <d3d11.h>
#include <directxmath.h>
#include <string>
#include <vector>

struct MeshUniversalVertex
{
	DirectX::XMFLOAT4 Position;
	DirectX::XMFLOAT4 Color;
	DirectX::XMFLOAT2 TextureCoordinates;
};

struct MeshUniversalTransformBufferData
{
	DirectX::XMFLOAT4X4 WorldViewProjectionMatrix;
};

class SceneViewportSubsystem;

class MeshUniversalComponent : public RenderingComponent
{
public:
	MeshUniversalComponent();
	~MeshUniversalComponent() override;

	void Initialize() override;
	void Update(float DeltaTime) override;
	void Render(SceneViewportSubsystem* SceneViewport) override;
	void Shutdown() override;
	void SetUseOrthographicProjection(bool NewUseOrthographicProjection);
	bool GetUseOrthographicProjection() const;
	void SetOrthographicProjectionSize(float NewOrthographicProjectionWidth, float NewOrthographicProjectionHeight);

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
	ID3D11Buffer* TransformConstantBuffer;
	ID3D11Buffer* VertexBuffer;
	ID3D11Buffer* IndexBuffer;
	ID3D11RasterizerState* RasterState;
	UINT IndexCount;
	bool UseOrthographicProjection;
	float OrthographicProjectionWidth;
	float OrthographicProjectionHeight;
};

