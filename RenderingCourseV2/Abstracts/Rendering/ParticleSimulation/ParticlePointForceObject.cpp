#include "Abstracts/Rendering/ParticleSimulation/ParticlePointForceObject.h"
#include "Abstracts/Components/ParticleRenderingComponent.h"
#include <imgui.h>
#include <cstring>

namespace
{
	struct PointForceConstantsBufferData
	{
		DirectX::XMFLOAT4 WorldPointPositionAndStrength;
		DirectX::XMFLOAT4 FalloffConfiguration;
	};
}

ParticlePointForceObject::ParticlePointForceObject()
	: WorldPointPositionAndStrength(0.0f, 0.0f, 0.0f, 1.0f)
	, FalloffConfiguration(0.1f, 1.0f, 0.0f, 0.0f)
	, StageConstantBuffer(nullptr)
	, ComputeShader(nullptr)
	, ComputeShaderByteCode(nullptr)
{
}

bool ParticlePointForceObject::CreateGpuResources(ID3D11Device* Device)
{
	ReleaseGpuResources();
	if (Device == nullptr)
	{
		return false;
	}

	D3D11_BUFFER_DESC BufferDescription = {};
	BufferDescription.ByteWidth = sizeof(PointForceConstantsBufferData);
	BufferDescription.Usage = D3D11_USAGE_DYNAMIC;
	BufferDescription.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	BufferDescription.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	HRESULT Result = Device->CreateBuffer(&BufferDescription, nullptr, &StageConstantBuffer);
	if (FAILED(Result))
	{
		return false;
	}

	return CompileComputeShaderFromFile("Shaders/ParticleSystem/ParticlePointForce.hlsl", "Main", Device, &ComputeShader, &ComputeShaderByteCode);
}

void ParticlePointForceObject::ReleaseGpuResources()
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

void ParticlePointForceObject::Dispatch(ParticleRenderingComponent* OwnerComponent)
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

	PointForceConstantsBufferData BufferData = {};
	BufferData.WorldPointPositionAndStrength = WorldPointPositionAndStrength;
	BufferData.FalloffConfiguration = FalloffConfiguration;
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

void ParticlePointForceObject::DrawDearImGui(ParticleRenderingComponent*)
{
	float PositionArray[3] = {
		WorldPointPositionAndStrength.x,
		WorldPointPositionAndStrength.y,
		WorldPointPositionAndStrength.z
	};
	if (ImGui::DragFloat3("WorldPointPosition", PositionArray, 0.1f))
	{
		WorldPointPositionAndStrength.x = PositionArray[0];
		WorldPointPositionAndStrength.y = PositionArray[1];
		WorldPointPositionAndStrength.z = PositionArray[2];
	}
	ImGui::DragFloat("Strength", &WorldPointPositionAndStrength.w, 0.1f);
	ImGui::DragFloat("MinimumDistance", &FalloffConfiguration.x, 0.01f, 0.0001f, 1000.0f);
	ImGui::DragFloat("FalloffPower", &FalloffConfiguration.y, 0.01f, 0.01f, 10.0f);
}

const char* ParticlePointForceObject::GetStageDisplayName() const
{
	return "ParticlePointForce";
}
