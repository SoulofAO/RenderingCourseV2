#include "Abstracts/Rendering/DeferredRenderPipeline.h"
#include "Abstracts/Components/RenderingComponent.h"
#include "Abstracts/Rendering/DeferredRenderer.h"
#include "Abstracts/Rendering/RenderProxy/DeferredRendererProxyObject.h"
#include "Abstracts/Subsystems/SceneViewportSubsystem.h"

void DeferredRenderPipeline::RenderFrame(
	SceneViewportSubsystem* SceneViewport,
	const std::vector<RenderingComponent*>& RenderingComponents)
{
	if (SceneViewport == nullptr)
	{
		return;
	}

	DeferredRenderer* DeferredRendererInstance = SceneViewport->GetDeferredRenderer();
	ID3D11DeviceContext* DeviceContext = SceneViewport->GetDeviceContext();
	ID3D11RenderTargetView* RenderTargetView = SceneViewport->GetRenderTargetView();
	if (DeferredRendererInstance == nullptr || DeviceContext == nullptr || RenderTargetView == nullptr)
	{
		return;
	}

	DeferredRendererInstance->EnsureTargets(
		SceneViewport->GetDevice(),
		SceneViewport->GetScreenWidth(),
		SceneViewport->GetScreenHeight());

	const bool IsShadowRenderingEnabled = SceneViewport->GetIsShadowRenderingEnabled();
	if (IsShadowRenderingEnabled && SceneViewport->GetDirectionalLightIntensity() > 0.0f)
	{
		const DirectX::XMMATRIX CameraViewMatrix = SceneViewport->GetViewMatrix();
		const DirectX::XMMATRIX CameraProjectionMatrix = SceneViewport->GetProjectionMatrix();
		DeferredRendererInstance->PrepareCascadedShadowMaps(
			CameraViewMatrix,
			CameraProjectionMatrix,
			SceneViewport->GetCameraWorldPosition(),
			SceneViewport->GetDirectionalLightDirection());

		const int ShadowCascadeCount = DeferredRendererInstance->GetShadowCascadeCount();
		for (int CascadeIndex = 0; CascadeIndex < ShadowCascadeCount; ++CascadeIndex)
		{
			if (DeferredRendererInstance->BeginShadowCascadePass(DeviceContext, CascadeIndex) == false)
			{
				continue;
			}

			DeferredShadowRenderPassState DeferredShadowRenderPassStateValue = {};
			DeferredShadowRenderPassStateValue.DeviceContext = DeviceContext;
			DeferredShadowRenderPassStateValue.ViewMatrix = DeferredRendererInstance->GetShadowCascadeViewMatrix(CascadeIndex);
			DeferredShadowRenderPassStateValue.ProjectionMatrix = DeferredRendererInstance->GetShadowCascadeProjectionMatrix(CascadeIndex);
			for (RenderingComponent* ExistingRenderingComponent : RenderingComponents)
			{
				if (ExistingRenderingComponent == nullptr)
				{
					continue;
				}

				DeferredRendererProxyObject* DeferredRendererProxyObjectInstance = ExistingRenderingComponent->GetDeferredRendererProxyObject();
				if (DeferredRendererProxyObjectInstance != nullptr)
				{
					DeferredRendererProxyObjectInstance->RenderDeferredShadowPass(DeferredShadowRenderPassStateValue);
				}
			}
		}

		DeferredRendererInstance->EndShadowPass(DeviceContext);
	}

	D3D11_VIEWPORT ScreenViewport = {};
	ScreenViewport.Width = static_cast<float>(SceneViewport->GetScreenWidth());
	ScreenViewport.Height = static_cast<float>(SceneViewport->GetScreenHeight());
	ScreenViewport.TopLeftX = 0.0f;
	ScreenViewport.TopLeftY = 0.0f;
	ScreenViewport.MinDepth = 0.0f;
	ScreenViewport.MaxDepth = 1.0f;
	DeviceContext->RSSetViewports(1, &ScreenViewport);

	DeferredRendererInstance->BeginGeometryPass(DeviceContext);
	DeferredGeometryRenderPassState DeferredGeometryRenderPassStateValue = {};
	DeferredGeometryRenderPassStateValue.DeviceContext = DeviceContext;
	DeferredGeometryRenderPassStateValue.ViewMatrix = SceneViewport->GetViewMatrix();
	DeferredGeometryRenderPassStateValue.ProjectionMatrix = SceneViewport->GetProjectionMatrix();
	DeferredGeometryRenderPassStateValue.CameraWorldPosition = SceneViewport->GetCameraWorldPosition();
	DeferredGeometryRenderPassStateValue.DirectionalLightDirection = SceneViewport->GetDirectionalLightDirection();
	DeferredGeometryRenderPassStateValue.DirectionalLightColor = SceneViewport->GetDirectionalLightColor();
	DeferredGeometryRenderPassStateValue.DirectionalLightIntensity = SceneViewport->GetDirectionalLightIntensity();
	DeferredGeometryRenderPassStateValue.UseFullBrightnessWithoutLighting = SceneViewport->GetUseFullBrightnessWithoutLighting();
	DeferredGeometryRenderPassStateValue.IsDearImGuiInitialized = SceneViewport->GetIsDearImGuiInitialized();

	for (RenderingComponent* ExistingRenderingComponent : RenderingComponents)
	{
		if (ExistingRenderingComponent == nullptr)
		{
			continue;
		}

		DeferredRendererProxyObject* DeferredRendererProxyObjectInstance = ExistingRenderingComponent->GetDeferredRendererProxyObject();
		if (DeferredRendererProxyObjectInstance != nullptr)
		{
			DeferredRendererProxyObjectInstance->RenderDeferredGeometryPass(DeferredGeometryRenderPassStateValue);
		}
	}
	DeferredRendererInstance->EndGeometryPass(DeviceContext);

	const DirectX::XMMATRIX ViewMatrix = SceneViewport->GetViewMatrix();
	const DirectX::XMMATRIX ProjectionMatrix = SceneViewport->GetProjectionMatrix();
	const DirectX::XMMATRIX ViewProjectionMatrix = ViewMatrix * ProjectionMatrix;
	const DirectX::XMMATRIX InverseViewProjectionMatrix = DirectX::XMMatrixInverse(nullptr, ViewProjectionMatrix);

	DeferredRendererInstance->RenderLightingPass(
		DeviceContext,
		RenderTargetView,
		ViewMatrix,
		InverseViewProjectionMatrix,
		SceneViewport->GetCameraWorldPosition(),
		SceneViewport->GetDirectionalLightDirection(),
		SceneViewport->GetDirectionalLightColor(),
		SceneViewport->GetDirectionalLightIntensity(),
		SceneViewport->GetPointLights(),
		SceneViewport->GetSpotLights(),
		SceneViewport->GetUseFullBrightnessWithoutLighting(),
		IsShadowRenderingEnabled ? 1.0f : 0.0f,
		static_cast<float>(SceneViewport->GetDeferredDebugBufferViewMode()));
}
