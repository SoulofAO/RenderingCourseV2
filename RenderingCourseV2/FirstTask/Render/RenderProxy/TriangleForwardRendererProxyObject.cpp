#include "FirstTask/Render/RenderProxy/TriangleForwardRendererProxyObject.h"
#include "FirstTask/TriangleComponent.h"

TriangleForwardRendererProxyObject::TriangleForwardRendererProxyObject(TriangleComponent* NewOwnerComponent)
	: OwnerComponent(NewOwnerComponent)
{
}

void TriangleForwardRendererProxyObject::RenderForwardMainPass(const ForwardMainRenderPassState& ForwardMainRenderPassStateValue)
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

	DeviceContext->RSSetState(OwnerComponent->RasterState);
	DeviceContext->IASetInputLayout(OwnerComponent->Layout);
	DeviceContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	DeviceContext->IASetIndexBuffer(OwnerComponent->IndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	UINT Strides[] = { 32 };
	UINT Offsets[] = { 0 };
	DeviceContext->IASetVertexBuffers(0, 1, &OwnerComponent->VertexBuffer, Strides, Offsets);

	DeviceContext->VSSetShader(OwnerComponent->VertexShader, nullptr, 0);
	DeviceContext->PSSetShader(OwnerComponent->PixelShader, nullptr, 0);

	DeviceContext->DrawIndexed(OwnerComponent->IndexCount, 0, 0);
}
