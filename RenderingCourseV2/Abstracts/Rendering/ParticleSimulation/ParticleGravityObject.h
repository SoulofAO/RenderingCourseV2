#pragma once

#include "Abstracts/Rendering/ParticleSimulation/ParticleSimulationObject.h"

class ParticleGravityObject final : public ParticleSimulationObject
{
public:
	ParticleGravityObject();

	bool CreateGpuResources(ID3D11Device* Device) override;
	void ReleaseGpuResources() override;
	void Dispatch(ParticleRenderingComponent* OwnerComponent) override;
	void DrawDearImGui(ParticleRenderingComponent* OwnerComponent) override;
	const char* GetStageDisplayName() const override;

private:
	ID3D11ComputeShader* ComputeShader;
	ID3DBlob* ComputeShaderByteCode;
};
