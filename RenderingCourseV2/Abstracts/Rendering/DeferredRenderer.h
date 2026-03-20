#pragma once

#include <directxmath.h>
#include <d3d11.h>
#include <array>
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
		const DirectX::XMMATRIX& ViewMatrix,
		const DirectX::XMMATRIX& InverseViewProjectionMatrix,
		const DirectX::XMFLOAT3& CameraWorldPosition,
		const DirectX::XMFLOAT3& DirectionalLightDirection,
		const DirectX::XMFLOAT4& DirectionalLightColor,
		float DirectionalLightIntensity,
		float UseFullBrightnessWithoutLighting,
		float ShadowStrength,
		float DeferredDebugBufferViewMode);
	void PrepareCascadedShadowMaps(
		const DirectX::XMMATRIX& CameraViewMatrix,
		const DirectX::XMMATRIX& CameraProjectionMatrix,
		const DirectX::XMFLOAT3& CameraWorldPosition,
		const DirectX::XMFLOAT3& DirectionalLightDirection);
	bool BeginShadowCascadePass(ID3D11DeviceContext* DeviceContext, int CascadeIndex);
	void EndShadowPass(ID3D11DeviceContext* DeviceContext);
	int GetShadowCascadeCount() const;
	void SetShadowCascadeSettings(int NewShadowCascadeCount, float NewShadowMaximumDistance);
	int GetShadowCascadeCountSetting() const;
	float GetShadowMaximumDistanceSetting() const;
	DirectX::XMMATRIX GetShadowCascadeViewMatrix(int CascadeIndex) const;
	DirectX::XMMATRIX GetShadowCascadeProjectionMatrix(int CascadeIndex) const;

	ID3D11DepthStencilView* GetDepthStencilView() const;

private:
	void ReleaseTargets();
	void ReleaseShadowResources();
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
	ID3D11Texture2D* ShadowDepthTextureArray;
	ID3D11ShaderResourceView* ShadowDepthSRV;
	ID3D11DepthStencilView* ShadowDepthDSVs[4];
	ID3D11SamplerState* ShadowComparisonSampler;
	ID3D11RasterizerState* ShadowRasterizerState;
	std::array<DirectX::XMFLOAT4X4, 4> ShadowCascadeViewMatricesStorage;
	std::array<DirectX::XMFLOAT4X4, 4> ShadowCascadeProjectionMatricesStorage;
	std::array<DirectX::XMFLOAT4X4, 4> ShadowCascadeViewProjectionMatricesStorage;
	DirectX::XMFLOAT4 ShadowCascadeSplitDepths;
	int ShadowMapResolution;
	int ShadowCascadeCountSetting;
	float ShadowMaximumDistanceSetting;
	int CachedWidth;
	int CachedHeight;
};
