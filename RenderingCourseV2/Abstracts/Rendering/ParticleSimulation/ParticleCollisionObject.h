#pragma once

#include "Abstracts/Rendering/ParticleSimulation/ParticleSimulationObject.h"

class ParticleCollisionObject final : public ParticleSimulationObject
{
public:
	ParticleCollisionObject();

	float DepthBias;
	float SurfaceOffset;
	float BounceRestitution;
	float NormalSampleDistanceScale;
	ID3D11Buffer* StageConstantBuffer;

	bool CreateGpuResources(ID3D11Device* Device) override;
	void ReleaseGpuResources() override;
	void Dispatch(ParticleRenderingComponent* OwnerComponent) override;
	void DrawDearImGui(ParticleRenderingComponent* OwnerComponent) override;

private:
	ID3D11ComputeShader* ComputeShader;
	ID3DBlob* ComputeShaderByteCode;
};
