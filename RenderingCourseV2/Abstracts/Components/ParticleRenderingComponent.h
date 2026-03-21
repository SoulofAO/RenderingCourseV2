#pragma once

#include "RenderingComponent.h"
#include "Abstracts/Rendering/ParticleSimulation/ParticleSimulationObject.h"
#include <d3d11.h>
#include <directxmath.h>
#include <memory>
#include <vector>

struct ParticleStructData
{
	DirectX::XMFLOAT3 Position;
	float PositionPadding;
	DirectX::XMFLOAT3 Velocity;
	float VelocityPadding;
	DirectX::XMFLOAT4 Color;
	float AgeTime;
	UINT Active;
};

class SceneViewportSubsystem;

class ParticleRenderingComponent : public RenderingComponent
{
public:
	explicit ParticleRenderingComponent(int NewMaxParticleCount = 4096);
	~ParticleRenderingComponent() override;

	void Initialize() override;
	void Update(float DeltaTime) override;
	void Shutdown() override;

	void DrawDearImGuiParticlePanels();

	ID3D11DeviceContext* GetDeviceContext() const;
	SceneViewportSubsystem* GetSceneViewportSubsystem() const;
	ID3D11SamplerState* GetSceneDepthSamplerState() const;

	void BindParticleSimulationCommonComputeState(ID3D11ComputeShader* ComputeShader);
	void DispatchParticleSimulationThreadGroups();
	void UnbindParticleSimulationCompute();

	DirectX::XMFLOAT3 GetGravityDirectionSimulation() const;
	void SetGravityDirectionSimulation(const DirectX::XMFLOAT3& NewGravityDirectionSimulation);

	int GetMaxParticleCount() const;
	float GetParticleSizeWorld() const;
	void SetParticleSizeWorld(float NewParticleSizeWorld);

	ID3D11Buffer* GetParticleMaterialConstantBuffer() const;
	ID3D11VertexShader* GetParticleVertexShader() const;
	ID3D11PixelShader* GetParticlePixelShader() const;
	ID3D11ShaderResourceView* GetParticleStateShaderResourceView() const;
	ID3D11BlendState* GetParticleAlphaBlendState() const;
	ID3D11DepthStencilState* GetParticleDepthStencilState() const;
	ID3D11RasterizerState* GetParticleRasterizerState() const;
	UINT GetParticleDrawInstanceCount() const;

private:
	bool CompileShaderFromFile(
		const std::string& ShaderPath,
		const char* EntryPoint,
		const char* ShaderModel,
		ID3DBlob** OutputByteCode) const;
	bool CreateDrawShaderProgram(ID3D11Device* Device);
	void ReleaseDrawShaderProgram();
	bool CreateParticleSimulationResources(ID3D11Device* Device);
	void ReleaseParticleSimulationResources();
	void UpdateParticleSimulationConstantsBuffer();
	void BuildDefaultSimulationPipeline(ID3D11Device* Device);

	int MaxParticleCount;
	float LastDeltaTime;
	DirectX::XMFLOAT3 GravityDirectionSimulation;
	float ParticleSizeWorld;
	SceneViewportSubsystem* CachedSceneViewport;

	ID3D11Buffer* ParticleStateBuffer;
	ID3D11UnorderedAccessView* ParticleStateUnorderedAccessView;
	ID3D11ShaderResourceView* ParticleStateShaderResourceView;
	ID3D11Buffer* ParticleSimulationConstantsBuffer;
	ID3D11Buffer* ParticleMaterialConstantBuffer;
	ID3D11SamplerState* SceneDepthSamplerState;

	ID3D11VertexShader* ParticleVertexShader;
	ID3D11PixelShader* ParticlePixelShader;
	ID3DBlob* ParticleVertexShaderByteCode;
	ID3DBlob* ParticlePixelShaderByteCode;

	ID3D11BlendState* ParticleAlphaBlendState;
	ID3D11DepthStencilState* ParticleDepthStencilState;
	ID3D11RasterizerState* ParticleRasterizerState;

	UINT ParticleSimulationThreadGroupCount;

	std::vector<std::unique_ptr<ParticleSimulationObject>> SimulationStages;
};
