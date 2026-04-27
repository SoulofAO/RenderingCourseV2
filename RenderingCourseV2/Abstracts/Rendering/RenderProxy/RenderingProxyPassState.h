#pragma once

#include <d3d11.h>
#include <directxmath.h>

struct ForwardMainRenderPassState
{
	ID3D11DeviceContext* DeviceContext;
	DirectX::XMMATRIX ViewMatrix;
	DirectX::XMMATRIX ProjectionMatrix;
	DirectX::XMFLOAT3 CameraWorldPosition;
	DirectX::XMFLOAT3 DirectionalLightDirection;
	DirectX::XMFLOAT4 DirectionalLightColor;
	float DirectionalLightIntensity;
	float UseFullBrightnessWithoutLighting;
	bool IsDearImGuiInitialized;
	bool ParticleDistanceSortEnabled;
};

struct DeferredGeometryRenderPassState
{
	ID3D11DeviceContext* DeviceContext;
	DirectX::XMMATRIX ViewMatrix;
	DirectX::XMMATRIX ProjectionMatrix;
	DirectX::XMFLOAT3 CameraWorldPosition;
	DirectX::XMFLOAT3 DirectionalLightDirection;
	DirectX::XMFLOAT4 DirectionalLightColor;
	float DirectionalLightIntensity;
	float UseFullBrightnessWithoutLighting;
	bool IsDearImGuiInitialized;
};

struct DeferredShadowRenderPassState
{
	ID3D11DeviceContext* DeviceContext;
	DirectX::XMMATRIX ViewMatrix;
	DirectX::XMMATRIX ProjectionMatrix;
};
