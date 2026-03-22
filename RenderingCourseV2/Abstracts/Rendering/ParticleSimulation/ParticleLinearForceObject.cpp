#include "Abstracts/Rendering/ParticleSimulation/ParticleLinearForceObject.h"
#include "Abstracts/Components/ParticleRenderingComponent.h"
#include <imgui.h>
#include <cstring>

namespace
{
	struct LinearForceConstantsBufferData
	{
		DirectX::XMFLOAT3 LinearAcceleration;
		float Padding0;
	};
}

ParticleLinearForceObject::ParticleLinearForceObject()
	: LinearAcceleration(0.0f, 0.0f, 0.0f)
	, StageConstantBuffer(nullptr)
	, ComputeShader(nullptr)
	, ComputeShaderByteCode(nullptr)
{
}

bool ParticleLinearForceObject::CreateGpuResources(ID3D11Device* Device)
{
	ReleaseGpuResources();
	if (Device == nullptr)
	{
		return false;
	}

	D3D11_BUFFER_DESC BufferDescription = {};
	BufferDescription.ByteWidth = sizeof(LinearForceConstantsBufferData);
	BufferDescription.Usage = D3D11_USAGE_DYNAMIC;
	BufferDescription.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	BufferDescription.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	HRESULT Result = Device->CreateBuffer(&BufferDescription, nullptr, &StageConstantBuffer);
	if (FAILED(Result))
	{
		return false;
	}

	return CompileComputeShaderFromFile("Shaders/ParticleSystem/ParticleLinearForce.hlsl", "Main", Device, &ComputeShader, &ComputeShaderByteCode);
}

void ParticleLinearForceObject::ReleaseGpuResources()
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

void ParticleLinearForceObject::Dispatch(ParticleRenderingComponent* OwnerComponent)
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

	LinearForceConstantsBufferData BufferData = {};
	BufferData.LinearAcceleration = LinearAcceleration;
	BufferData.Padding0 = 0.0f;
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

void ParticleLinearForceObject::DrawDearImGui(ParticleRenderingComponent*)
{
	float AccelerationArray[3] = {
		LinearAcceleration.x,
		LinearAcceleration.y,
		LinearAcceleration.z
	};
	if (ImGui::DragFloat3("LinearAcceleration", AccelerationArray, 0.01f))
	{
		LinearAcceleration = DirectX::XMFLOAT3(
			AccelerationArray[0],
			AccelerationArray[1],
			AccelerationArray[2]);
	}
}

const char* ParticleLinearForceObject::GetStageDisplayName() const
{
	return "ParticleLinearForce";
}
