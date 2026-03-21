#include "Abstracts/Rendering/ParticleSimulation/ParticleFrictionObject.h"
#include "Abstracts/Components/ParticleRenderingComponent.h"
#include <imgui.h>
#include <cstring>

namespace
{
	struct FrictionConstantsBufferData
	{
		DirectX::XMFLOAT4 LinearFrictionData;
	};
}

ParticleFrictionObject::ParticleFrictionObject()
	: LinearFriction(0.5f)
	, StageConstantBuffer(nullptr)
	, ComputeShader(nullptr)
	, ComputeShaderByteCode(nullptr)
{
}

bool ParticleFrictionObject::CreateGpuResources(ID3D11Device* Device)
{
	ReleaseGpuResources();
	if (Device == nullptr)
	{
		return false;
	}

	D3D11_BUFFER_DESC BufferDescription = {};
	BufferDescription.ByteWidth = sizeof(FrictionConstantsBufferData);
	BufferDescription.Usage = D3D11_USAGE_DYNAMIC;
	BufferDescription.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	BufferDescription.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	HRESULT Result = Device->CreateBuffer(&BufferDescription, nullptr, &StageConstantBuffer);
	if (FAILED(Result))
	{
		return false;
	}

	const std::wstring ShaderPath = L"./Shaders/ParticleSystem/ParticleFriction.hlsl";
	return CompileComputeShaderFromFile(ShaderPath, "Main", Device, &ComputeShader, &ComputeShaderByteCode);
}

void ParticleFrictionObject::ReleaseGpuResources()
{
	if (StageConstantBuffer != nullptr)
	{
		StageConstantBuffer->Release();
		StageConstantBuffer = nullptr;
	}
	if (ComputeShader != nullptr)
	{
		ComputeShader->Release();
		ComputeShader = nullptr;
	}
	if (ComputeShaderByteCode != nullptr)
	{
		ComputeShaderByteCode->Release();
		ComputeShaderByteCode = nullptr;
	}
}

void ParticleFrictionObject::Dispatch(ParticleRenderingComponent* OwnerComponent)
{
	if (OwnerComponent == nullptr || ComputeShader == nullptr || StageConstantBuffer == nullptr)
	{
		return;
	}

	ID3D11DeviceContext* DeviceContext = OwnerComponent->GetDeviceContext();
	if (DeviceContext == nullptr)
	{
		return;
	}

	FrictionConstantsBufferData BufferData = {};
	BufferData.LinearFrictionData = DirectX::XMFLOAT4(LinearFriction, 0.0f, 0.0f, 0.0f);
	D3D11_MAPPED_SUBRESOURCE MappedResource = {};
	HRESULT MapResult = DeviceContext->Map(StageConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource);
	if (SUCCEEDED(MapResult))
	{
		memcpy(MappedResource.pData, &BufferData, sizeof(BufferData));
		DeviceContext->Unmap(StageConstantBuffer, 0);
	}

	OwnerComponent->BindParticleSimulationCommonComputeState(ComputeShader);
	ID3D11Buffer* ConstantBuffers[] = { StageConstantBuffer };
	DeviceContext->CSSetConstantBuffers(1, 1, ConstantBuffers);
	OwnerComponent->DispatchParticleSimulationThreadGroups();
}

void ParticleFrictionObject::DrawDearImGui(ParticleRenderingComponent*)
{
	ImGui::DragFloat("LinearFriction", &LinearFriction, 0.01f, 0.0f, 1000.0f);
}
