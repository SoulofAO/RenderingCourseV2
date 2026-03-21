#pragma once

#include "Abstracts/Core/Object.h"
#include <d3d11.h>
#include <string>

class ParticleRenderingComponent;

class ParticleSimulationObject : public Object
{
public:
	~ParticleSimulationObject() override;

	virtual bool CreateGpuResources(ID3D11Device* Device) = 0;
	virtual void ReleaseGpuResources() = 0;
	virtual void Dispatch(ParticleRenderingComponent* OwnerComponent) = 0;
	virtual void DrawDearImGui(ParticleRenderingComponent* OwnerComponent) = 0;

	void Shutdown() override;

protected:
	static bool CompileComputeShaderFromFile(
		const std::wstring& ShaderFilePath,
		const char* EntryPoint,
		ID3D11Device* Device,
		ID3D11ComputeShader** OutComputeShader,
		ID3DBlob** OutByteCode);
};
