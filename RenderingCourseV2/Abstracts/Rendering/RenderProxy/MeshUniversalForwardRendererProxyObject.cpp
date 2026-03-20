#include "Abstracts/Rendering/RenderProxy/MeshUniversalForwardRendererProxyObject.h"
#include "Abstracts/Components/MeshUniversalComponent.h"

MeshUniversalForwardRendererProxyObject::MeshUniversalForwardRendererProxyObject(MeshUniversalComponent* NewOwnerComponent)
	: OwnerComponent(NewOwnerComponent)
{
}

void MeshUniversalForwardRendererProxyObject::RenderForwardMainPass(const ForwardMainRenderPassState& ForwardMainRenderPassStateValue)
{
	if (OwnerComponent == nullptr)
	{
		return;
	}

	ID3D11DeviceContext* DeviceContext = ForwardMainRenderPassStateValue.DeviceContext;
	if (DeviceContext == nullptr || OwnerComponent->VertexBuffer == nullptr || OwnerComponent->IndexBuffer == nullptr)
	{
		return;
	}

	DeviceContext->RSSetState(OwnerComponent->RasterState);
	DeviceContext->IASetInputLayout(OwnerComponent->Layout);
	DeviceContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	DeviceContext->IASetIndexBuffer(OwnerComponent->IndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	UINT Strides[] = { static_cast<UINT>(sizeof(MeshUniversalVertex)) };
	UINT Offsets[] = { 0 };
	DeviceContext->IASetVertexBuffers(0, 1, &OwnerComponent->VertexBuffer, Strides, Offsets);

	if (OwnerComponent->TransformConstantBuffer != nullptr)
	{
		Transform WorldTransform = OwnerComponent->GetWorldTransform();
		DirectX::XMMATRIX WorldMatrix = WorldTransform.ToMatrix();
		DirectX::XMMATRIX ViewMatrix = ForwardMainRenderPassStateValue.ViewMatrix;
		DirectX::XMMATRIX ProjectionMatrix = ForwardMainRenderPassStateValue.ProjectionMatrix;

		if (OwnerComponent->UseOrthographicProjection)
		{
			ProjectionMatrix = DirectX::XMMatrixOrthographicLH(
				OwnerComponent->OrthographicProjectionWidth,
				OwnerComponent->OrthographicProjectionHeight,
				0.1f,
				1000.0f);
		}

		DirectX::XMMATRIX WorldViewProjectionMatrix = WorldMatrix * ViewMatrix * ProjectionMatrix;
		MeshUniversalTransformBufferData TransformBufferData = {};
		DirectX::XMStoreFloat4x4(
			&TransformBufferData.WorldViewProjectionMatrix,
			DirectX::XMMatrixTranspose(WorldViewProjectionMatrix));
		DirectX::XMStoreFloat4x4(&TransformBufferData.WorldMatrix, DirectX::XMMatrixTranspose(WorldMatrix));
		TransformBufferData.CameraWorldPosition = ForwardMainRenderPassStateValue.CameraWorldPosition;

		DeviceContext->UpdateSubresource(OwnerComponent->TransformConstantBuffer, 0, nullptr, &TransformBufferData, 0, 0);
		DeviceContext->VSSetConstantBuffers(0, 1, &OwnerComponent->TransformConstantBuffer);
		DeviceContext->PSSetConstantBuffers(0, 1, &OwnerComponent->TransformConstantBuffer);
	}

	if (OwnerComponent->LightConstantBuffer != nullptr)
	{
		MeshUniversalLightBufferData LightBufferData = {};
		LightBufferData.DirectionalLightDirection = ForwardMainRenderPassStateValue.DirectionalLightDirection;
		LightBufferData.DirectionalLightColor = ForwardMainRenderPassStateValue.DirectionalLightColor;
		LightBufferData.DirectionalLightIntensity = ForwardMainRenderPassStateValue.DirectionalLightIntensity;
		LightBufferData.UseFullBrightnessWithoutLighting = ForwardMainRenderPassStateValue.UseFullBrightnessWithoutLighting;
		DeviceContext->UpdateSubresource(OwnerComponent->LightConstantBuffer, 0, nullptr, &LightBufferData, 0, 0);
		DeviceContext->PSSetConstantBuffers(1, 1, &OwnerComponent->LightConstantBuffer);
	}

	if (OwnerComponent->MaterialConstantBuffer != nullptr)
	{
		MeshUniversalMaterialBufferData MaterialBufferData = {};
		MaterialBufferData.BaseColor = OwnerComponent->BaseColor;
		MaterialBufferData.SpecularPower = OwnerComponent->SpecularPower;
		MaterialBufferData.SpecularIntensity = OwnerComponent->SpecularIntensity;
		MaterialBufferData.UseAlbedoTexture = OwnerComponent->AlbedoTexture ? 1.0f : 0.0f;
		MaterialBufferData.UseNormalTexture = OwnerComponent->NormalTexture ? 1.0f : 0.0f;
		DeviceContext->UpdateSubresource(OwnerComponent->MaterialConstantBuffer, 0, nullptr, &MaterialBufferData, 0, 0);
		DeviceContext->PSSetConstantBuffers(2, 1, &OwnerComponent->MaterialConstantBuffer);
	}

	OwnerComponent->BindMaterialResources(DeviceContext);
	DeviceContext->VSSetShader(OwnerComponent->VertexShader, nullptr, 0);
	DeviceContext->PSSetShader(OwnerComponent->PixelShader, nullptr, 0);
	DeviceContext->DrawIndexed(OwnerComponent->IndexCount, 0, 0);
}
