#include "Abstracts/Rendering/ForwardRenderPipeline.h"
#include "Abstracts/Components/RenderingComponent.h"
#include "Abstracts/Rendering/RenderProxy/ForwardRendererProxyObject.h"
#include "Abstracts/Subsystems/SceneViewportSubsystem.h"

void ForwardRenderPipeline::RenderFrame(
	SceneViewportSubsystem* SceneViewport,
	const std::vector<RenderingComponent*>& RenderingComponents)
{
	if (SceneViewport == nullptr)
	{
		return;
	}

	ID3D11DeviceContext* DeviceContext = SceneViewport->GetDeviceContext();
	ID3D11RenderTargetView* RenderTargetView = SceneViewport->GetRenderTargetView();
	if (DeviceContext == nullptr || RenderTargetView == nullptr)
	{
		return;
	}

	ID3D11DepthStencilView* DepthStencilView = SceneViewport->GetDepthStencilView();
	DeviceContext->OMSetRenderTargets(1, &RenderTargetView, DepthStencilView);

	ForwardMainRenderPassState ForwardMainRenderPassStateValue = {};
	ForwardMainRenderPassStateValue.DeviceContext = DeviceContext;
	ForwardMainRenderPassStateValue.ViewMatrix = SceneViewport->GetViewMatrix();
	ForwardMainRenderPassStateValue.ProjectionMatrix = SceneViewport->GetProjectionMatrix();
	ForwardMainRenderPassStateValue.CameraWorldPosition = SceneViewport->GetCameraWorldPosition();
	ForwardMainRenderPassStateValue.DirectionalLightDirection = SceneViewport->GetDirectionalLightDirection();
	ForwardMainRenderPassStateValue.DirectionalLightColor = SceneViewport->GetDirectionalLightColor();
	ForwardMainRenderPassStateValue.DirectionalLightIntensity = SceneViewport->GetDirectionalLightIntensity();
	ForwardMainRenderPassStateValue.UseFullBrightnessWithoutLighting = SceneViewport->GetUseFullBrightnessWithoutLighting();
	ForwardMainRenderPassStateValue.IsDearImGuiInitialized = SceneViewport->GetIsDearImGuiInitialized();
	ForwardMainRenderPassStateValue.ParticleDistanceSortEnabled = SceneViewport->GetParticleDistanceSortEnabled();

	for (RenderingComponent* ExistingRenderingComponent : RenderingComponents)
	{
		if (ExistingRenderingComponent == nullptr)
		{
			continue;
		}

		ForwardRendererProxyObject* ForwardRendererProxyObjectInstance = ExistingRenderingComponent->GetForwardRendererProxyObject();
		if (ForwardRendererProxyObjectInstance != nullptr)
		{
			ForwardRendererProxyObjectInstance->RenderForwardMainPass(ForwardMainRenderPassStateValue);
		}
	}
}
