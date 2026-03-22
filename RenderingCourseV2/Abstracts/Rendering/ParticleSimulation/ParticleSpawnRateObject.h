#pragma once

#include "Abstracts/Rendering/ParticleSimulation/ParticleSimulationObject.h"
#include <directxmath.h>

class ParticleSpawnRateObject final : public ParticleSimulationObject
{
public:
	ParticleSpawnRateObject();

	float SpawnRate;
	float SpawnAccumulator;
	UINT NextSpawnRingIndex;
	DirectX::XMFLOAT3 EmitterWorldPosition;
	DirectX::XMFLOAT3 InitialVelocity;
	DirectX::XMFLOAT4 EmitterColor;
	float SpawnSizeWorldMinimum;
	float SpawnSizeWorldMaximum;
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
