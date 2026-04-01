#include "Abstracts/Rendering/RenderProxy/MeshUniversalDeferredRendererProxyObject.h"
#include "Abstracts/Components/MeshUniversalComponent.h"
#include "Abstracts/Core/Game.h"
#include "Abstracts/Rendering/DeferredRenderer.h"
#include "Abstracts/Subsystems/SceneViewportSubsystem.h"

MeshUniversalDeferredRendererProxyObject::MeshUniversalDeferredRendererProxyObject(MeshUniversalComponent* NewOwnerComponent)
	: OwnerComponent(NewOwnerComponent)
{
}

void MeshUniversalDeferredRendererProxyObject::RenderDeferredGeometryPass(const DeferredGeometryRenderPassState& DeferredGeometryRenderPassStateValue)
{
	if (OwnerComponent == nullptr)
	{
		return;
	}

	ID3D11DeviceContext* DeviceContext = DeferredGeometryRenderPassStateValue.DeviceContext;
	if (DeviceContext == nullptr || OwnerComponent->VertexBuffer == nullptr || OwnerComponent->IndexBuffer == nullptr)
	{
		return;
	}

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
		DirectX::XMMATRIX ProjectionMatrix = DeferredGeometryRenderPassStateValue.ProjectionMatrix;
		if (OwnerComponent->UseOrthographicProjection)
		{
			ProjectionMatrix = DirectX::XMMatrixOrthographicLH(
				OwnerComponent->OrthographicProjectionWidth,
				OwnerComponent->OrthographicProjectionHeight,
				0.1f,
				1000.0f);
		}

		const DirectX::XMMATRIX WorldViewProjectionMatrix = WorldMatrix * DeferredGeometryRenderPassStateValue.ViewMatrix * ProjectionMatrix;
		MeshUniversalTransformBufferData TransformBufferData = {};
		DirectX::XMStoreFloat4x4(&TransformBufferData.WorldViewProjectionMatrix, DirectX::XMMatrixTranspose(WorldViewProjectionMatrix));
		DirectX::XMStoreFloat4x4(&TransformBufferData.WorldMatrix, DirectX::XMMatrixTranspose(WorldMatrix));
		TransformBufferData.CameraWorldPosition = DeferredGeometryRenderPassStateValue.CameraWorldPosition;
		DeviceContext->UpdateSubresource(OwnerComponent->TransformConstantBuffer, 0, nullptr, &TransformBufferData, 0, 0);
		DeviceContext->VSSetConstantBuffers(0, 1, &OwnerComponent->TransformConstantBuffer);
		DeviceContext->PSSetConstantBuffers(0, 1, &OwnerComponent->TransformConstantBuffer);
	}

	if (OwnerComponent->LightConstantBuffer != nullptr)
	{
		MeshUniversalLightBufferData LightBufferData = {};
		LightBufferData.DirectionalLightDirection = DeferredGeometryRenderPassStateValue.DirectionalLightDirection;
		LightBufferData.DirectionalLightColor = DeferredGeometryRenderPassStateValue.DirectionalLightColor;
		LightBufferData.DirectionalLightIntensity = DeferredGeometryRenderPassStateValue.DirectionalLightIntensity;
		LightBufferData.UseFullBrightnessWithoutLighting = DeferredGeometryRenderPassStateValue.UseFullBrightnessWithoutLighting;
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
	if (OwnerComponent->DeferredVertexShader != nullptr && OwnerComponent->DeferredPixelShader != nullptr)
	{
		DeviceContext->VSSetShader(OwnerComponent->DeferredVertexShader, nullptr, 0);
		DeviceContext->PSSetShader(OwnerComponent->DeferredPixelShader, nullptr, 0);
	}
	else
	{
		DeviceContext->VSSetShader(OwnerComponent->VertexShader, nullptr, 0);
		DeviceContext->PSSetShader(OwnerComponent->PixelShader, nullptr, 0);
	}

	DeviceContext->DrawIndexed(OwnerComponent->IndexCount, 0, 0);
}

void MeshUniversalDeferredRendererProxyObject::RenderDeferredShadowPass(const DeferredShadowRenderPassState& DeferredShadowRenderPassStateValue)
{
	if (OwnerComponent == nullptr)
	{
		return;
	}

	ID3D11DeviceContext* DeviceContext = DeferredShadowRenderPassStateValue.DeviceContext;
	if (DeviceContext == nullptr || OwnerComponent->VertexBuffer == nullptr || OwnerComponent->IndexBuffer == nullptr)
	{
		return;
	}

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
		const DirectX::XMMATRIX WorldViewProjectionMatrix =
			WorldMatrix *
			DeferredShadowRenderPassStateValue.ViewMatrix *
			DeferredShadowRenderPassStateValue.ProjectionMatrix;
		MeshUniversalTransformBufferData TransformBufferData = {};
		DirectX::XMStoreFloat4x4(&TransformBufferData.WorldViewProjectionMatrix, DirectX::XMMatrixTranspose(WorldViewProjectionMatrix));
		DirectX::XMStoreFloat4x4(&TransformBufferData.WorldMatrix, DirectX::XMMatrixTranspose(WorldMatrix));
		TransformBufferData.CameraWorldPosition = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
		DeviceContext->UpdateSubresource(OwnerComponent->TransformConstantBuffer, 0, nullptr, &TransformBufferData, 0, 0);
		DeviceContext->VSSetConstantBuffers(0, 1, &OwnerComponent->TransformConstantBuffer);
		DeviceContext->PSSetConstantBuffers(0, 1, &OwnerComponent->TransformConstantBuffer);
	}

	if (OwnerComponent->DeferredVertexShader != nullptr)
	{
		DeviceContext->VSSetShader(OwnerComponent->DeferredVertexShader, nullptr, 0);
	}
	else
	{
		DeviceContext->VSSetShader(OwnerComponent->VertexShader, nullptr, 0);
	}
	DeviceContext->PSSetShader(nullptr, nullptr, 0);
	DeviceContext->DrawIndexed(OwnerComponent->IndexCount, 0, 0);
}

void MeshUniversalDeferredRendererProxyObject::RenderDeferredStencilShadowVolumePass(
	const DeferredStencilShadowRenderPassState& DeferredStencilShadowRenderPassStateValue)
{
	if (OwnerComponent == nullptr)
	{
		return;
	}

	if (DeferredStencilShadowRenderPassStateValue.DeferredRendererInstance == nullptr)
	{
		return;
	}

	ID3D11DeviceContext* DeviceContext = DeferredStencilShadowRenderPassStateValue.DeviceContext;
	if (DeviceContext == nullptr)
	{
		return;
	}

	Game* OwningGame = OwnerComponent->GetOwningGame();
	if (OwningGame == nullptr)
	{
		return;
	}

	SceneViewportSubsystem* SceneViewport = OwningGame->GetSubsystem<SceneViewportSubsystem>();
	if (SceneViewport == nullptr)
	{
		return;
	}

	ID3D11Device* Device = SceneViewport->GetDevice();
	if (Device == nullptr)
	{
		return;
	}

	OwnerComponent->EnsureShadowVolumeGeometryForLight(Device, DeferredStencilShadowRenderPassStateValue.LightWorldPosition);
	if (
		OwnerComponent->ShadowVolumeIndexCount == 0 ||
		OwnerComponent->ShadowVolumeVertexBuffer == nullptr ||
		OwnerComponent->ShadowVolumeIndexBuffer == nullptr)
	{
		return;
	}

	const DirectX::XMMATRIX WorldMatrix = OwnerComponent->GetWorldTransform().ToMatrix();
	const DirectX::XMMATRIX WorldViewProjectionMatrix =
		WorldMatrix *
		DeferredStencilShadowRenderPassStateValue.ViewMatrix *
		DeferredStencilShadowRenderPassStateValue.ProjectionMatrix;

	DirectX::XMFLOAT4X4 WorldViewProjectionMatrixStorage;
	DirectX::XMStoreFloat4x4(&WorldViewProjectionMatrixStorage, DirectX::XMMatrixTranspose(WorldViewProjectionMatrix));

	ID3D11Buffer* ShadowVolumeTransformConstantBuffer =
		DeferredStencilShadowRenderPassStateValue.DeferredRendererInstance->GetShadowVolumeTransformConstantBuffer();
	if (ShadowVolumeTransformConstantBuffer == nullptr)
	{
		return;
	}

	DeviceContext->UpdateSubresource(ShadowVolumeTransformConstantBuffer, 0, nullptr, &WorldViewProjectionMatrixStorage, 0, 0);

	const UINT VertexBufferStride = sizeof(DirectX::XMFLOAT4);
	const UINT VertexBufferOffset = 0;
	DeviceContext->IASetVertexBuffers(0, 1, &OwnerComponent->ShadowVolumeVertexBuffer, &VertexBufferStride, &VertexBufferOffset);
	DeviceContext->IASetIndexBuffer(OwnerComponent->ShadowVolumeIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	DeferredStencilShadowRenderPassStateValue.DeferredRendererInstance->BindShadowVolumePassPipelineForSubPass(DeviceContext, 0);
	DeviceContext->DrawIndexed(OwnerComponent->ShadowVolumeIndexCount, 0, 0);
	DeferredStencilShadowRenderPassStateValue.DeferredRendererInstance->BindShadowVolumePassPipelineForSubPass(DeviceContext, 1);
	DeviceContext->DrawIndexed(OwnerComponent->ShadowVolumeIndexCount, 0, 0);
}
