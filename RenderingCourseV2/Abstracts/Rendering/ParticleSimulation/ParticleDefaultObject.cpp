#include "Abstracts/Rendering/ParticleSimulation/ParticleDefaultObject.h"
#include "Abstracts/Components/ParticleRenderingComponent.h"
#include <imgui.h>

ParticleDefaultObject::ParticleDefaultObject()
	: ComputeShader(nullptr)
	, ComputeShaderByteCode(nullptr)
{
}

bool ParticleDefaultObject::CreateGpuResources(ID3D11Device* Device)
{
	ReleaseGpuResources();
	return CompileComputeShaderFromFile("Shaders/ParticleSystem/ParticleDefault.hlsl", "Main", Device, &ComputeShader, &ComputeShaderByteCode);
}

void ParticleDefaultObject::ReleaseGpuResources()
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

void ParticleDefaultObject::Dispatch(ParticleRenderingComponent* OwnerComponent)
{
	if (OwnerComponent == nullptr || ComputeShader == nullptr)
	{
		return;
	}
	OwnerComponent->BindParticleSimulationCommonComputeState(ComputeShader);
	OwnerComponent->DispatchParticleSimulationThreadGroups();
}

void ParticleDefaultObject::DrawDearImGui(ParticleRenderingComponent*)
{
	ImGui::TextUnformatted("Integrates position and age from velocity.");
}

const char* ParticleDefaultObject::GetStageDisplayName() const
{
	return "ParticleDefault";
}
