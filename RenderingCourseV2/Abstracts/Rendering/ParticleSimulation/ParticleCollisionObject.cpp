#include "Abstracts/Rendering/ParticleSimulation/ParticleCollisionObject.h"
#include "Abstracts/Components/ParticleRenderingComponent.h"
#include "Abstracts/Rendering/DeferredRenderer.h"
#include "Abstracts/Subsystems/SceneViewportSubsystem.h"
#include <imgui.h>
#include <cstring>

namespace
{
	struct ParticleCollisionConstantsBufferData
	{
		DirectX::XMFLOAT4X4 ViewProjectionMatrix;
		DirectX::XMFLOAT4X4 InverseViewProjectionMatrix;
		DirectX::XMFLOAT3 CameraWorldPosition;
		float DepthBias;
		float SurfaceOffset;
		float BounceRestitution;
		float NormalSampleDistanceScale;
		float Padding0;
		DirectX::XMFLOAT2 ScreenSize;
		DirectX::XMFLOAT2 Padding1;
	};
}

ParticleCollisionObject::ParticleCollisionObject()
	: DepthBias(0.0001f)
	, SurfaceOffset(0.05f)
	, BounceRestitution(0.3f)
	, NormalSampleDistanceScale(1.0f)
	, StageConstantBuffer(nullptr)
	, ComputeShader(nullptr)
	, ComputeShaderByteCode(nullptr)
{
}

bool ParticleCollisionObject::CreateGpuResources(ID3D11Device* Device)
{
	ReleaseGpuResources();
	if (Device == nullptr)
	{
		return false;
	}

	D3D11_BUFFER_DESC BufferDescription = {};
	BufferDescription.ByteWidth = sizeof(ParticleCollisionConstantsBufferData);
	BufferDescription.Usage = D3D11_USAGE_DYNAMIC;
	BufferDescription.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	BufferDescription.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	HRESULT Result = Device->CreateBuffer(&BufferDescription, nullptr, &StageConstantBuffer);
	if (FAILED(Result))
	{
		return false;
	}

	return CompileComputeShaderFromFile("Shaders/ParticleSystem/ParticleCollision.hlsl", "Main", Device, &ComputeShader, &ComputeShaderByteCode);
}

void ParticleCollisionObject::ReleaseGpuResources()
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

void ParticleCollisionObject::Dispatch(ParticleRenderingComponent* OwnerComponent)
{
	if (OwnerComponent == nullptr || ComputeShader == nullptr || StageConstantBuffer == nullptr)
	{
		return;
	}

	SceneViewportSubsystem* SceneViewport = OwnerComponent->GetSceneViewportSubsystem();
	if (SceneViewport == nullptr)
	{
		return;
	}

	DeferredRenderer* DeferredRendererInstance = SceneViewport->GetDeferredRenderer();
	if (DeferredRendererInstance == nullptr)
	{
		return;
	}

	ID3D11ShaderResourceView* DepthShaderResourceView = DeferredRendererInstance->GetGBufferDepthShaderResourceView();
	if (DepthShaderResourceView == nullptr)
	{
		return;
	}

	if (SceneViewport->IsDeferredRenderingEnabled() == false)
	{
		return;
	}

	ID3D11DeviceContext* DeviceContext = OwnerComponent->GetDeviceContext();
	if (DeviceContext == nullptr)
	{
		return;
	}

	DirectX::XMMATRIX ViewMatrix = SceneViewport->GetViewMatrix();
	DirectX::XMMATRIX ProjectionMatrix = SceneViewport->GetProjectionMatrix();
	DirectX::XMMATRIX ViewProjectionMatrix = ViewMatrix * ProjectionMatrix;
	DirectX::XMMATRIX InverseViewProjectionMatrix = DirectX::XMMatrixInverse(nullptr, ViewProjectionMatrix);

	ParticleCollisionConstantsBufferData BufferData = {};
	DirectX::XMStoreFloat4x4(&BufferData.ViewProjectionMatrix, DirectX::XMMatrixTranspose(ViewProjectionMatrix));
	DirectX::XMStoreFloat4x4(&BufferData.InverseViewProjectionMatrix, DirectX::XMMatrixTranspose(InverseViewProjectionMatrix));
	BufferData.CameraWorldPosition = SceneViewport->GetCameraWorldPosition();
	BufferData.DepthBias = DepthBias;
	BufferData.SurfaceOffset = SurfaceOffset;
	BufferData.BounceRestitution = BounceRestitution;
	BufferData.NormalSampleDistanceScale = NormalSampleDistanceScale;
	BufferData.Padding0 = 0.0f;
	BufferData.ScreenSize = DirectX::XMFLOAT2(
		static_cast<float>(SceneViewport->GetScreenWidth()),
		static_cast<float>(SceneViewport->GetScreenHeight()));
	BufferData.Padding1 = DirectX::XMFLOAT2(0.0f, 0.0f);

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
	ID3D11ShaderResourceView* DepthResources[] = { DepthShaderResourceView };
	DeviceContext->CSSetShaderResources(1, 1, DepthResources);
	ID3D11SamplerState* Samplers[] = { OwnerComponent->GetSceneDepthSamplerState() };
	DeviceContext->CSSetSamplers(0, 1, Samplers);
	OwnerComponent->DispatchParticleSimulationThreadGroups();

	ID3D11ShaderResourceView* NullShaderResourceViews[] = { nullptr };
	DeviceContext->CSSetShaderResources(1, 1, NullShaderResourceViews);
}

void ParticleCollisionObject::DrawDearImGui(ParticleRenderingComponent*)
{
	ImGui::DragFloat("DepthBias", &DepthBias, 0.000001f, 0.0f, 0.01f, "%.6f");
	ImGui::DragFloat("SurfaceOffset", &SurfaceOffset, 0.001f, 0.0f, 10.0f);
	ImGui::DragFloat("BounceRestitution", &BounceRestitution, 0.01f, 0.0f, 2.0f);
	ImGui::DragFloat("NormalSampleDistanceScale", &NormalSampleDistanceScale, 0.1f, 0.1f, 16.0f);
}

const char* ParticleCollisionObject::GetStageDisplayName() const
{
	return "ParticleCollision";
}
