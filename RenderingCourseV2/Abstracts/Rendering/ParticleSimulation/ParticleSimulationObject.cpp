#include "Abstracts/Rendering/ParticleSimulation/ParticleSimulationObject.h"
#include <d3dcompiler.h>
#include <iostream>

#pragma comment(lib, "d3dcompiler.lib")

ParticleSimulationObject::~ParticleSimulationObject() = default;

void ParticleSimulationObject::Shutdown()
{
	ReleaseGpuResources();
	Object::Shutdown();
}

bool ParticleSimulationObject::CompileComputeShaderFromFile(
	const std::wstring& ShaderFilePath,
	const char* EntryPoint,
	ID3D11Device* Device,
	ID3D11ComputeShader** OutComputeShader,
	ID3DBlob** OutByteCode)
{
	if (Device == nullptr || EntryPoint == nullptr || OutComputeShader == nullptr || OutByteCode == nullptr)
	{
		return false;
	}

	ID3DBlob* ErrorCode = nullptr;
	HRESULT Result = D3DCompileFromFile(
		ShaderFilePath.c_str(),
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		EntryPoint,
		"cs_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		OutByteCode,
		&ErrorCode);

	if (FAILED(Result))
	{
		if (ErrorCode != nullptr)
		{
			char* CompileErrors = static_cast<char*>(ErrorCode->GetBufferPointer());
			std::cout << CompileErrors << std::endl;
			ErrorCode->Release();
		}
		return false;
	}

	if (ErrorCode != nullptr)
	{
		ErrorCode->Release();
	}

	Result = Device->CreateComputeShader(
		(*OutByteCode)->GetBufferPointer(),
		(*OutByteCode)->GetBufferSize(),
		nullptr,
		OutComputeShader);

	return SUCCEEDED(Result);
}
