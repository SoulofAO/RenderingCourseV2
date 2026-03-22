#pragma once

#include "Abstracts/Rendering/ParticleSimulation/ParticleSimulationObject.h"
#include <directxmath.h>

class ParticlePointForceObject final : public ParticleSimulationObject
{
public:
	ParticlePointForceObject();

	DirectX::XMFLOAT4 WorldPointPositionAndStrength;
	DirectX::XMFLOAT4 FalloffConfiguration;
	ID3D11Buffer* StageConstantBuffer;

	bool CreateGpuResources(ID3D11Device* Device) override;
	void ReleaseGpuResources() override;
	void Dispatch(ParticleRenderingComponent* OwnerComponent) override;
	void DrawDearImGui(ParticleRenderingComponent* OwnerComponent) override;
	const char* GetStageDisplayName() const override;

private:
	ID3D11ComputeShader* ComputeShader;
	ID3DBlob* ComputeShaderByteCode;
};
