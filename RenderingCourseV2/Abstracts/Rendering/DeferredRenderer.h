#pragma once

#include <directxmath.h>
#include <d3d11.h>
#include <array>
#include <string>
#include <vector>

constexpr int MaximumDeferredPointLightCount = 16;
constexpr int MaximumDeferredSpotLightCount = 16;

struct DeferredGBufferInspectPixelResult
{
	bool HasSurfaceHit;
	DirectX::XMFLOAT4 AlbedoSample;
	DirectX::XMFLOAT4 NormalWorld;
	DirectX::XMFLOAT4 MaterialSample;
	float DepthHardwareNormalized;
	DirectX::XMFLOAT3 WorldPosition;
	uint32_t PickIdentifierUint32;
};

struct DeferredPointLightData
{
	DirectX::XMFLOAT3 Position;
	float Intensity;
	DirectX::XMFLOAT4 Color;
	float Range;
	DirectX::XMFLOAT3 Padding0;
};

struct DeferredSpotLightData
{
	DirectX::XMFLOAT3 Position;
	float Intensity;
	DirectX::XMFLOAT4 Color;
	DirectX::XMFLOAT3 Direction;
	float Range;
	float InnerConeAngleCosine;
	float OuterConeAngleCosine;
	DirectX::XMFLOAT2 Padding0;
};

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
		const std::vector<DeferredPointLightData>& PointLights,
		const std::vector<DeferredSpotLightData>& SpotLights,
		float UseFullBrightnessWithoutLighting,
		float ShadowStrength,
		float UseShadowedAlbedoTextureWithoutShadowDimming,
		float UseShadowedAlbedoTextureShadowEfficiencyAdjustment,
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
	ID3D11ShaderResourceView* GetGBufferDepthShaderResourceView() const;

	bool ReadGBufferInspectPixel(
		ID3D11Device* Device,
		ID3D11DeviceContext* DeviceContext,
		int PixelPositionX,
		int PixelPositionY,
		int ScreenWidth,
		int ScreenHeight,
		const DirectX::XMMATRIX& InverseViewProjectionMatrix,
		DeferredGBufferInspectPixelResult& OutInspectResult) const;

private:
	void ReleaseTargets();
	void ReleaseShadowResources();
	void ReleaseInspectReadbackStagingTextures();
	void EnsureInspectReadbackStagingTextures(ID3D11Device* Device) const;
	bool CompileShader(ID3D11Device* Device, const std::string& Path, const char* EntryPoint, const char* Model, ID3DBlob** ByteCode, ID3D11DeviceChild** ShaderObject);

	ID3D11Texture2D* GBufferAlbedoTexture;
	ID3D11Texture2D* GBufferNormalTexture;
	ID3D11Texture2D* GBufferMaterialTexture;
	ID3D11Texture2D* GBufferShadowAlbedoTexture;
	ID3D11Texture2D* GBufferPickTexture;
	ID3D11Texture2D* GBufferDepthTexture;
	ID3D11RenderTargetView* GBufferAlbedoRTV;
	ID3D11RenderTargetView* GBufferNormalRTV;
	ID3D11RenderTargetView* GBufferMaterialRTV;
	ID3D11RenderTargetView* GBufferShadowAlbedoRTV;
	ID3D11RenderTargetView* GBufferPickRTV;
	ID3D11ShaderResourceView* GBufferAlbedoSRV;
	ID3D11ShaderResourceView* GBufferNormalSRV;
	ID3D11ShaderResourceView* GBufferMaterialSRV;
	ID3D11ShaderResourceView* GBufferShadowAlbedoSRV;
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

	mutable ID3D11Texture2D* InspectReadbackAlbedoStagingTexture;
	mutable ID3D11Texture2D* InspectReadbackNormalStagingTexture;
	mutable ID3D11Texture2D* InspectReadbackMaterialStagingTexture;
	mutable ID3D11Texture2D* InspectReadbackPickStagingTexture;
	mutable ID3D11Texture2D* InspectReadbackDepthStagingTexture;
};
