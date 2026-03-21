#pragma once

#include "Abstracts/Rendering/ParticleSimulation/ParticleSimulationObject.h"

class ParticleFrictionObject final : public ParticleSimulationObject
{
public:
	ParticleFrictionObject();

	float LinearFriction;
	ID3D11Buffer* StageConstantBuffer;

	bool CreateGpuResources(ID3D11Device* Device) override;
	void ReleaseGpuResources() override;
	void Dispatch(ParticleRenderingComponent* OwnerComponent) override;
	void DrawDearImGui(ParticleRenderingComponent* OwnerComponent) override;

private:
	ID3D11ComputeShader* ComputeShader;
	ID3DBlob* ComputeShaderByteCode;
};
