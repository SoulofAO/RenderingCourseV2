#include "Abstracts/Rendering/ParticleSimulation/ParticleSpawnRateObject.h"
#include "Abstracts/Components/ParticleRenderingComponent.h"
#include <imgui.h>
#include <cmath>
#include <cstring>

namespace
{
	struct SpawnRateConstantsBufferData
	{
		UINT SpawnCount;
		UINT BaseIndex;
		UINT MaxParticleCount;
		UINT Padding0;
		DirectX::XMFLOAT3 EmitterWorldPosition;
		float Padding1;
		DirectX::XMFLOAT3 InitialVelocity;
		float Padding2;
		DirectX::XMFLOAT4 EmitterColor;
		float SpawnSizeWorldMinimum;
		float SpawnSizeWorldMaximum;
		float SpawnLifetimeSecondsMinimum;
		float SpawnLifetimeSecondsMaximum;
	};
}

ParticleSpawnRateObject::ParticleSpawnRateObject()
	: SpawnRate(180.0f)
	, SpawnAccumulator(0.0f)
	, NextSpawnRingIndex(0)
	, EmitterWorldPosition(0.0f, 1.2f, 2.0f)
	, InitialVelocity(0.0f, 4.5f, 0.0f)
	, EmitterColor(1.0f, 0.55f, 0.15f, 1.0f)
	, SpawnSizeWorldMinimum(0.35f)
	, SpawnSizeWorldMaximum(0.65f)
	, SpawnLifetimeSecondsMinimum(2.0f)
	, SpawnLifetimeSecondsMaximum(5.0f)
	, StageConstantBuffer(nullptr)
	, ComputeShader(nullptr)
	, ComputeShaderByteCode(nullptr)
{
}

bool ParticleSpawnRateObject::CreateGpuResources(ID3D11Device* Device)
{
	ReleaseGpuResources();
	if (Device == nullptr)
	{
		return false;
	}

	D3D11_BUFFER_DESC BufferDescription = {};
	BufferDescription.ByteWidth = sizeof(SpawnRateConstantsBufferData);
	BufferDescription.Usage = D3D11_USAGE_DYNAMIC;
	BufferDescription.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	BufferDescription.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	HRESULT Result = Device->CreateBuffer(&BufferDescription, nullptr, &StageConstantBuffer);
	if (FAILED(Result))
	{
		return false;
	}

	return CompileComputeShaderFromFile("Shaders/ParticleSystem/ParticleSpawnRate.hlsl", "Main", Device, &ComputeShader, &ComputeShaderByteCode);
}

void ParticleSpawnRateObject::ReleaseGpuResources()
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

void ParticleSpawnRateObject::Dispatch(ParticleRenderingComponent* OwnerComponent)
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

	const float DeltaTime = OwnerComponent->GetLastSimulationDeltaTime();
	if (DeltaTime <= 0.0f)
	{
		return;
	}

	if (SpawnRate <= 0.0f)
	{
		return;
	}

	const int MaxParticles = OwnerComponent->GetMaxParticleCount();
	if (MaxParticles <= 0)
	{
		return;
	}

	SpawnAccumulator += SpawnRate * DeltaTime;
	int SpawnCount = static_cast<int>(floorf(SpawnAccumulator));
	if (SpawnCount <= 0)
	{
		return;
	}

	SpawnAccumulator -= static_cast<float>(SpawnCount);
	if (SpawnCount > MaxParticles)
	{
		SpawnAccumulator += static_cast<float>(SpawnCount - MaxParticles);
		SpawnCount = MaxParticles;
	}

	SpawnRateConstantsBufferData BufferData = {};
	BufferData.SpawnCount = static_cast<UINT>(SpawnCount);
	BufferData.BaseIndex = NextSpawnRingIndex;
	BufferData.MaxParticleCount = static_cast<UINT>(MaxParticles);
	BufferData.Padding0 = 0;
	BufferData.EmitterWorldPosition = EmitterWorldPosition;
	BufferData.Padding1 = 0.0f;
	BufferData.InitialVelocity = InitialVelocity;
	BufferData.Padding2 = 0.0f;
	BufferData.EmitterColor = EmitterColor;
	BufferData.SpawnSizeWorldMinimum = SpawnSizeWorldMinimum;
	BufferData.SpawnSizeWorldMaximum = SpawnSizeWorldMaximum;
	BufferData.SpawnLifetimeSecondsMinimum = SpawnLifetimeSecondsMinimum;
	BufferData.SpawnLifetimeSecondsMaximum = SpawnLifetimeSecondsMaximum;

	D3D11_MAPPED_SUBRESOURCE MappedResource = {};
	HRESULT MapResult = DeviceContext->Map(StageConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource);
	if (FAILED(MapResult))
	{
		return;
	}

	memcpy(MappedResource.pData, &BufferData, sizeof(BufferData));
	DeviceContext->Unmap(StageConstantBuffer, 0);

	NextSpawnRingIndex = (NextSpawnRingIndex + static_cast<UINT>(SpawnCount)) % static_cast<UINT>(MaxParticles);

	OwnerComponent->BindParticleSimulationCommonComputeState(ComputeShader);
	ID3D11Buffer* ConstantBuffers[] = { StageConstantBuffer };
	DeviceContext->CSSetConstantBuffers(1, 1, ConstantBuffers);
	OwnerComponent->DispatchParticleSpawnThreadGroups(static_cast<UINT>(SpawnCount));
}

void ParticleSpawnRateObject::DrawDearImGui(ParticleRenderingComponent*)
{
	ImGui::DragFloat("SpawnRate", &SpawnRate, 1.0f, 0.0f, 100000.0f);
	float EmitterPositionArray[3] = {
		EmitterWorldPosition.x,
		EmitterWorldPosition.y,
		EmitterWorldPosition.z
	};
	if (ImGui::DragFloat3("EmitterWorldPosition", EmitterPositionArray, 0.05f))
	{
		EmitterWorldPosition.x = EmitterPositionArray[0];
		EmitterWorldPosition.y = EmitterPositionArray[1];
		EmitterWorldPosition.z = EmitterPositionArray[2];
	}
	float InitialVelocityArray[3] = {
		InitialVelocity.x,
		InitialVelocity.y,
		InitialVelocity.z
	};
	if (ImGui::DragFloat3("InitialVelocity", InitialVelocityArray, 0.05f))
	{
		InitialVelocity.x = InitialVelocityArray[0];
		InitialVelocity.y = InitialVelocityArray[1];
		InitialVelocity.z = InitialVelocityArray[2];
	}
	float EmitterColorArray[4] = {
		EmitterColor.x,
		EmitterColor.y,
		EmitterColor.z,
		EmitterColor.w
	};
	if (ImGui::DragFloat4("EmitterColor", EmitterColorArray, 0.01f, 0.0f, 1.0f))
	{
		EmitterColor.x = EmitterColorArray[0];
		EmitterColor.y = EmitterColorArray[1];
		EmitterColor.z = EmitterColorArray[2];
		EmitterColor.w = EmitterColorArray[3];
	}
	ImGui::DragFloat("SpawnSizeWorldMinimum", &SpawnSizeWorldMinimum, 0.01f, 0.0f, 100.0f);
	ImGui::DragFloat("SpawnSizeWorldMaximum", &SpawnSizeWorldMaximum, 0.01f, 0.0f, 100.0f);
	ImGui::DragFloat("SpawnLifetimeSecondsMinimum", &SpawnLifetimeSecondsMinimum, 0.05f, 0.0f, 1000.0f);
	ImGui::DragFloat("SpawnLifetimeSecondsMaximum", &SpawnLifetimeSecondsMaximum, 0.05f, 0.0f, 1000.0f);
}

const char* ParticleSpawnRateObject::GetStageDisplayName() const
{
	return "ParticleSpawnRate";
}
