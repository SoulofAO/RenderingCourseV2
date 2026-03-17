#pragma once

#include <directxmath.h>
#include <d3d11.h>
#include <string>

class DeferredRenderer
{
public:
	DeferredRenderer();
	~DeferredRenderer();

	void Initialize(ID3D11Device* Device);
	void Shutdown();
	void EnsureTargets(ID3D11Device* Device, int ScreenWidth, int ScreenHeight);
	void BeginGeometryPass(ID3D11DeviceContext* DeviceContext);
	void EndGeometryPass(ID3D11DeviceContext* DeviceContext);
	void RenderLightingPass(
		ID3D11DeviceContext* DeviceContext,
		ID3D11RenderTargetView* FinalRenderTargetView,
		const DirectX::XMMATRIX& InverseViewProjectionMatrix,
		const DirectX::XMFLOAT3& CameraWorldPosition,
		const DirectX::XMFLOAT3& DirectionalLightDirection,
		const DirectX::XMFLOAT4& DirectionalLightColor,
		float DirectionalLightIntensity);

	ID3D11DepthStencilView* GetDepthStencilView() const;

private:
	void ReleaseTargets();
	bool CompileShader(ID3D11Device* Device, const std::string& Path, const char* EntryPoint, const char* Model, ID3DBlob** ByteCode, ID3D11DeviceChild** ShaderObject);

	ID3D11Texture2D* GBufferAlbedoTexture;
	ID3D11Texture2D* GBufferNormalTexture;
	ID3D11Texture2D* GBufferMaterialTexture;
	ID3D11Texture2D* GBufferDepthTexture;
	ID3D11RenderTargetView* GBufferAlbedoRTV;
	ID3D11RenderTargetView* GBufferNormalRTV;
	ID3D11RenderTargetView* GBufferMaterialRTV;
	ID3D11ShaderResourceView* GBufferAlbedoSRV;
	ID3D11ShaderResourceView* GBufferNormalSRV;
	ID3D11ShaderResourceView* GBufferMaterialSRV;
	ID3D11ShaderResourceView* GBufferDepthSRV;
	ID3D11DepthStencilView* GBufferDepthDSV;
	ID3D11VertexShader* LightingVertexShader;
	ID3D11PixelShader* LightingPixelShader;
	ID3DBlob* LightingVertexShaderByteCode;
	ID3DBlob* LightingPixelShaderByteCode;
	ID3D11Buffer* CameraConstantBuffer;
	ID3D11Buffer* LightConstantBuffer;
	ID3D11SamplerState* GBufferSampler;
	int CachedWidth;
	int CachedHeight;
};
