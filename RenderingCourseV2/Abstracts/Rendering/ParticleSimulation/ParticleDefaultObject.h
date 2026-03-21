#pragma once

#include "Abstracts/Rendering/ParticleSimulation/ParticleSimulationObject.h"

class ParticleDefaultObject final : public ParticleSimulationObject
{
public:
	ParticleDefaultObject();

	bool CreateGpuResources(ID3D11Device* Device) override;
	void ReleaseGpuResources() override;
	void Dispatch(ParticleRenderingComponent* OwnerComponent) override;
	void DrawDearImGui(ParticleRenderingComponent* OwnerComponent) override;

private:
	ID3D11ComputeShader* ComputeShader;
	ID3DBlob* ComputeShaderByteCode;
};
