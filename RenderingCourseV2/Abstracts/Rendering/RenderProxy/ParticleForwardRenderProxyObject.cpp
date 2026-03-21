#include "Abstracts/Rendering/RenderProxy/ParticleForwardRenderProxyObject.h"
#include "Abstracts/Components/ParticleRenderingComponent.h"

namespace
{
	struct ParticleMaterialConstantsBufferData
	{
		DirectX::XMFLOAT4X4 ViewProjectionMatrix;
		DirectX::XMFLOAT3 CameraRightWorld;
		float ParticleSize;
		DirectX::XMFLOAT3 CameraUpWorld;
		float Padding0;
		UINT ParticleDrawCount;
		UINT Padding1[3];
	};
}

ParticleForwardRenderProxyObject::ParticleForwardRenderProxyObject(ParticleRenderingComponent* NewOwnerComponent)
	: OwnerComponent(NewOwnerComponent)
{
}

void ParticleForwardRenderProxyObject::RenderForwardMainPass(const ForwardMainRenderPassState& ForwardMainRenderPassStateValue)
{
	if (OwnerComponent == nullptr)
	{
		return;
	}

	ID3D11DeviceContext* DeviceContext = ForwardMainRenderPassStateValue.DeviceContext;
	if (DeviceContext == nullptr)
	{
		return;
	}

	ID3D11VertexShader* VertexShader = OwnerComponent->GetParticleVertexShader();
	ID3D11PixelShader* PixelShader = OwnerComponent->GetParticlePixelShader();
	ID3D11Buffer* MaterialConstantBuffer = OwnerComponent->GetParticleMaterialConstantBuffer();
	ID3D11ShaderResourceView* ParticleStateShaderResourceView = OwnerComponent->GetParticleStateShaderResourceView();
	if (VertexShader == nullptr || PixelShader == nullptr || MaterialConstantBuffer == nullptr || ParticleStateShaderResourceView == nullptr)
	{
		return;
	}

	DirectX::XMMATRIX ViewMatrix = ForwardMainRenderPassStateValue.ViewMatrix;
	DirectX::XMMATRIX ProjectionMatrix = ForwardMainRenderPassStateValue.ProjectionMatrix;
	DirectX::XMMATRIX ViewProjectionMatrix = ViewMatrix * ProjectionMatrix;
	DirectX::XMMATRIX InverseViewMatrix = DirectX::XMMatrixInverse(nullptr, ViewMatrix);

	DirectX::XMFLOAT4X4 InverseViewMatrixStored;
	DirectX::XMStoreFloat4x4(&InverseViewMatrixStored, InverseViewMatrix);

	DirectX::XMFLOAT3 CameraRightWorld(
		InverseViewMatrixStored._11,
		InverseViewMatrixStored._21,
		InverseViewMatrixStored._31);
	DirectX::XMFLOAT3 CameraUpWorld(
		InverseViewMatrixStored._12,
		InverseViewMatrixStored._22,
		InverseViewMatrixStored._32);

	ParticleMaterialConstantsBufferData MaterialBufferData = {};
	DirectX::XMStoreFloat4x4(&MaterialBufferData.ViewProjectionMatrix, DirectX::XMMatrixTranspose(ViewProjectionMatrix));
	MaterialBufferData.CameraRightWorld = CameraRightWorld;
	MaterialBufferData.ParticleSize = OwnerComponent->GetParticleSizeWorld();
	MaterialBufferData.CameraUpWorld = CameraUpWorld;
	MaterialBufferData.Padding0 = 0.0f;
	MaterialBufferData.ParticleDrawCount = OwnerComponent->GetParticleDrawInstanceCount();
	MaterialBufferData.Padding1[0] = 0;
	MaterialBufferData.Padding1[1] = 0;
	MaterialBufferData.Padding1[2] = 0;

	DeviceContext->UpdateSubresource(MaterialConstantBuffer, 0, nullptr, &MaterialBufferData, 0, 0);

	const float BlendFactor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	DeviceContext->OMSetBlendState(OwnerComponent->GetParticleAlphaBlendState(), BlendFactor, 0xFFFFFFFF);
	DeviceContext->OMSetDepthStencilState(OwnerComponent->GetParticleDepthStencilState(), 0);
	DeviceContext->RSSetState(OwnerComponent->GetParticleRasterizerState());

	DeviceContext->IASetInputLayout(nullptr);
	DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	ID3D11Buffer* ConstantBuffers[] = { MaterialConstantBuffer };
	DeviceContext->VSSetConstantBuffers(0, 1, ConstantBuffers);
	DeviceContext->PSSetConstantBuffers(0, 1, ConstantBuffers);

	ID3D11ShaderResourceView* ShaderResourceViews[] = { ParticleStateShaderResourceView };
	DeviceContext->VSSetShaderResources(0, 1, ShaderResourceViews);

	DeviceContext->VSSetShader(VertexShader, nullptr, 0);
	DeviceContext->PSSetShader(PixelShader, nullptr, 0);

	DeviceContext->DrawInstanced(4, OwnerComponent->GetParticleDrawInstanceCount(), 0, 0);

	ID3D11ShaderResourceView* NullShaderResourceViews[] = { nullptr };
	DeviceContext->VSSetShaderResources(0, 1, NullShaderResourceViews);

	ID3D11BlendState* DefaultBlendState = nullptr;
	DeviceContext->OMSetBlendState(DefaultBlendState, nullptr, 0xFFFFFFFF);
	ID3D11DepthStencilState* DefaultDepthStencilState = nullptr;
	DeviceContext->OMSetDepthStencilState(DefaultDepthStencilState, 0);
	ID3D11RasterizerState* DefaultRasterizerState = nullptr;
	DeviceContext->RSSetState(DefaultRasterizerState);
}
