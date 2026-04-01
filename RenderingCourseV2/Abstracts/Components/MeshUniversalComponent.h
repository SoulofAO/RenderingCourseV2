#pragma once

#include "Abstracts/Components/RenderingComponent.h"
#include <d3d11.h>
#include <directxmath.h>
#include <string>
#include <vector>
#include <wrl/client.h>

struct MeshUniversalVertex
{
	DirectX::XMFLOAT4 Position;
	DirectX::XMFLOAT4 Color;
	DirectX::XMFLOAT3 Normal;
	DirectX::XMFLOAT3 Tangent;
	DirectX::XMFLOAT2 TextureCoordinates;
};

struct MeshUniversalTransformBufferData
{
	DirectX::XMFLOAT4X4 WorldViewProjectionMatrix;
	DirectX::XMFLOAT4X4 WorldMatrix;
	DirectX::XMFLOAT3 CameraWorldPosition;
	float Padding0;
};

struct MeshUniversalLightBufferData
{
	DirectX::XMFLOAT3 DirectionalLightDirection;
	float DirectionalLightIntensity;
	DirectX::XMFLOAT4 DirectionalLightColor;
	float UseFullBrightnessWithoutLighting;
	DirectX::XMFLOAT3 Padding0;
};

struct MeshUniversalMaterialBufferData
{
	DirectX::XMFLOAT4 BaseColor;
	float SpecularPower;
	float SpecularIntensity;
	float UseAlbedoTexture;
	float UseNormalTexture;
};

class SceneViewportSubsystem;

class MeshUniversalComponent : public RenderingComponent
{
public:
	MeshUniversalComponent();
	~MeshUniversalComponent() override;

	void Initialize() override;
	void Update(float DeltaTime) override;
	void Shutdown() override;
	void SetUseOrthographicProjection(bool NewUseOrthographicProjection);
	bool GetUseOrthographicProjection() const;
	void SetOrthographicProjectionSize(float NewOrthographicProjectionWidth, float NewOrthographicProjectionHeight);

	virtual bool InitializeShaderProgram(ID3D11Device* Device, SceneViewportSubsystem* SceneViewport);
	virtual bool InitializeRenderResources(ID3D11Device* Device, SceneViewportSubsystem* SceneViewport);
	virtual void BindMaterialResources(ID3D11DeviceContext* DeviceContext);

	std::string VertexShaderName = "";
	std::string PixelShaderName = "";
	std::string DeferredVertexShaderName = "./Shaders/Deferred/DeferredGeometryPass.hlsl";
	std::string DeferredPixelShaderName = "./Shaders/Deferred/DeferredGeometryPass.hlsl";
	std::vector<MeshUniversalVertex> Vertices;
	std::vector<unsigned int> Indices;
	std::string ModelMeshPath = "";
	std::string AlbedoTexturePath = "";
	std::string NormalTexturePath = "";
	std::string SpecularTexturePath = "";
	std::string EmissiveTexturePath = "";
	DirectX::XMFLOAT4 BaseColor = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	float SpecularPower = 64.0f;
	float SpecularIntensity = 1.0f;

private:
	bool CompileShaderFromFile(
		const std::string& ShaderPath,
		const char* EntryPoint,
		const char* ShaderModel,
		ID3DBlob** OutputByteCode) const;
	void ReleaseShaderProgramResources();
	void ReleaseRenderResources();
	void EnsureShadowVolumeGeometryForLight(ID3D11Device* Device, const DirectX::XMFLOAT3& LightWorldPosition);
	void ReleaseShadowVolumeGeometry();

	ID3D11InputLayout* Layout;
	ID3D11VertexShader* VertexShader;
	ID3DBlob* VertexShaderByteCode;
	ID3D11PixelShader* PixelShader;
	ID3DBlob* PixelShaderByteCode;
	ID3D11VertexShader* DeferredVertexShader;
	ID3DBlob* DeferredVertexShaderByteCode;
	ID3D11PixelShader* DeferredPixelShader;
	ID3DBlob* DeferredPixelShaderByteCode;
	ID3D11Buffer* TransformConstantBuffer;
	ID3D11Buffer* LightConstantBuffer;
	ID3D11Buffer* MaterialConstantBuffer;
	ID3D11Buffer* VertexBuffer;
	ID3D11Buffer* IndexBuffer;
	ID3D11RasterizerState* RasterState;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> AlbedoTexture;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> NormalTexture;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> SpecularTexture;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> EmissiveTexture;
	ID3D11SamplerState* DefaultSamplerState;
	UINT IndexCount;
	bool UseOrthographicProjection;
	float OrthographicProjectionWidth;
	float OrthographicProjectionHeight;
	ID3D11Buffer* ShadowVolumeVertexBuffer;
	ID3D11Buffer* ShadowVolumeIndexBuffer;
	UINT ShadowVolumeIndexCount;
	DirectX::XMFLOAT3 CachedShadowVolumeLightWorldPosition;
	bool ShadowVolumeGeometryValid;

	friend class MeshUniversalForwardRendererProxyObject;
	friend class MeshUniversalDeferredRendererProxyObject;
};
