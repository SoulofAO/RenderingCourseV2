#pragma once

#include <directxmath.h>
#include <d3d11.h>
#include <array>
#include <string>
#include <vector>

constexpr int MaximumDeferredPointLightCount = 16;
constexpr int MaximumDeferredSpotLightCount = 16;

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
	void RenderDirectionalDeferredLightingPass(
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
	void RenderSinglePointLightDeferredLightingPass(
		ID3D11DeviceContext* DeviceContext,
		ID3D11RenderTargetView* FinalRenderTargetView,
		const DirectX::XMMATRIX& ViewMatrix,
		const DirectX::XMMATRIX& InverseViewProjectionMatrix,
		const DirectX::XMFLOAT3& CameraWorldPosition,
		const std::vector<DeferredPointLightData>& PointLights,
		int SinglePointLightIndex,
		float UseFullBrightnessWithoutLighting,
		float ShadowStrength,
		float DeferredDebugBufferViewMode,
		bool IsStencilShadowTestEnabled);
	void RenderSingleSpotLightDeferredLightingPass(
		ID3D11DeviceContext* DeviceContext,
		ID3D11RenderTargetView* FinalRenderTargetView,
		const DirectX::XMMATRIX& ViewMatrix,
		const DirectX::XMMATRIX& InverseViewProjectionMatrix,
		const DirectX::XMFLOAT3& CameraWorldPosition,
		const std::vector<DeferredSpotLightData>& SpotLights,
		int SingleSpotLightIndex,
		float UseFullBrightnessWithoutLighting,
		float ShadowStrength,
		float DeferredDebugBufferViewMode,
		bool IsStencilShadowTestEnabled);
	void ClearGBufferStencil(ID3D11DeviceContext* DeviceContext);
	void BeginShadowVolumeStencilPass(ID3D11DeviceContext* DeviceContext);
	void EndShadowVolumeStencilPass(ID3D11DeviceContext* DeviceContext);
	void BindShadowVolumePassPipelineForSubPass(ID3D11DeviceContext* DeviceContext, int SubPassIndex) const;
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
	ID3D11Buffer* GetShadowVolumeTransformConstantBuffer() const;

private:
	void ReleaseTargets();
	void ReleaseShadowResources();
	void ReleaseStencilShadowVolumeResources();
	bool CompileShader(ID3D11Device* Device, const std::string& Path, const char* EntryPoint, const char* Model, ID3DBlob** ByteCode, ID3D11DeviceChild** ShaderObject);
	void ExecuteDeferredLightingDrawCall(
		ID3D11DeviceContext* DeviceContext,
		ID3D11RenderTargetView* FinalRenderTargetView,
		bool UseAdditiveBlend,
		bool IsStencilShadowTestEnabled);

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
	ID3D11VertexShader* ShadowVolumeVertexShader;
	ID3DBlob* ShadowVolumeVertexShaderByteCode;
	ID3D11InputLayout* ShadowVolumeInputLayout;
	ID3D11Buffer* ShadowVolumeTransformConstantBuffer;
	ID3D11DepthStencilState* ShadowVolumeDepthStencilDepthFailPass1;
	ID3D11DepthStencilState* ShadowVolumeDepthStencilDepthFailPass2;
	ID3D11RasterizerState* ShadowVolumeRasterizerCullFront;
	ID3D11RasterizerState* ShadowVolumeRasterizerCullBack;
	ID3D11DepthStencilState* LightingStencilEqualZeroState;
	ID3D11BlendState* LightingAdditiveBlendState;
	ID3D11BlendState* ShadowVolumeColorWriteDisabledBlendState;
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
