#pragma once

#include "Engine/Core/Runtime/Abstract/Subsystems/Subsystem.h"
#include <d3d11.h>

class SceneViewportSubsystem;

class WorldRenderTargetSubsystem : public Subsystem
{
public:
	WorldRenderTargetSubsystem();
	~WorldRenderTargetSubsystem() override;

	void Initialize() override;
	void Shutdown() override;

	void BeginRenderPass(SceneViewportSubsystem* SceneViewport, float TotalTimeSeconds);
	void EndRenderPass(SceneViewportSubsystem* SceneViewport);
	ID3D11ShaderResourceView* GetShaderResourceView() const;

private:
	void CreateResources(SceneViewportSubsystem* SceneViewport);
	void DestroyResources();

	ID3D11Texture2D* RenderTexture;
	ID3D11RenderTargetView* RenderView;
	ID3D11ShaderResourceView* ShaderResourceView;
};
