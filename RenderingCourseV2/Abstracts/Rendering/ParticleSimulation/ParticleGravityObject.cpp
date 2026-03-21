#include "Abstracts/Rendering/ParticleSimulation/ParticleGravityObject.h"
#include "Abstracts/Components/ParticleRenderingComponent.h"
#include <imgui.h>

ParticleGravityObject::ParticleGravityObject()
	: ComputeShader(nullptr)
	, ComputeShaderByteCode(nullptr)
{
}

bool ParticleGravityObject::CreateGpuResources(ID3D11Device* Device)
{
	ReleaseGpuResources();
	const std::wstring ShaderPath = L"./Shaders/ParticleSystem/ParticleGravity.hlsl";
	return CompileComputeShaderFromFile(ShaderPath, "Main", Device, &ComputeShader, &ComputeShaderByteCode);
}

void ParticleGravityObject::ReleaseGpuResources()
{
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

void ParticleGravityObject::Dispatch(ParticleRenderingComponent* OwnerComponent)
{
	if (OwnerComponent == nullptr || ComputeShader == nullptr)
	{
		return;
	}
	OwnerComponent->BindParticleSimulationCommonComputeState(ComputeShader);
	OwnerComponent->DispatchParticleSimulationThreadGroups();
}

void ParticleGravityObject::DrawDearImGui(ParticleRenderingComponent* OwnerComponent)
{
	if (OwnerComponent == nullptr)
	{
		return;
	}
	DirectX::XMFLOAT3 GravityDirection = OwnerComponent->GetGravityDirectionSimulation();
	float GravityDirectionArray[3] = { GravityDirection.x, GravityDirection.y, GravityDirection.z };
	if (ImGui::DragFloat3("GravityDirection", GravityDirectionArray, 0.01f))
	{
		OwnerComponent->SetGravityDirectionSimulation(
			DirectX::XMFLOAT3(GravityDirectionArray[0], GravityDirectionArray[1], GravityDirectionArray[2]));
	}
}
