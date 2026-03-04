#include "Abstracts/Components/MeshUniversalComponent.h"
#include "Abstracts/Core/Game.h"
#include "Abstracts/Core/Actor.h"
#include "Abstracts/Subsystems/SceneViewportSubsystem.h"
#include "Abstracts/Subsystems/DisplayWin32.h"
#include <d3dcompiler.h>
#include <directxmath.h>
#include <iostream>

#pragma comment(lib, "d3dcompiler.lib")

MeshUniversalComponent::MeshUniversalComponent()
	: RenderingComponent()
	, Layout(nullptr)
	, VertexShader(nullptr)
	, VertexShaderByteCode(nullptr)
	, PixelShader(nullptr)
	, PixelShaderByteCode(nullptr)
	, TransformConstantBuffer(nullptr)
	, VertexBuffer(nullptr)
	, IndexBuffer(nullptr)
	, RasterState(nullptr)
	, IndexCount(0)
{
	Transform NewLocalTransform;
	NewLocalTransform.Position = DirectX::XMFLOAT3(0.0f, 0.0f, 2.0f);
	SetLocalTransform(NewLocalTransform);

	Vertices = {
		MeshUniversalVertex{
			DirectX::XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f),
			DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f),
			DirectX::XMFLOAT2(1.0f, 0.0f) },
		MeshUniversalVertex{
			DirectX::XMFLOAT4(-0.5f, -0.5f, 0.5f, 1.0f),
			DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f),
			DirectX::XMFLOAT2(0.0f, 1.0f) },
		MeshUniversalVertex{
			DirectX::XMFLOAT4(0.5f, -0.5f, 0.5f, 1.0f),
			DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f),
			DirectX::XMFLOAT2(1.0f, 1.0f) },
		MeshUniversalVertex{
			DirectX::XMFLOAT4(-0.5f, 0.5f, 0.5f, 1.0f),
			DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
			DirectX::XMFLOAT2(0.0f, 0.0f) }
	};

	Indices = { 0, 1, 2, 1, 0, 3 };
}

MeshUniversalComponent::~MeshUniversalComponent()
{
	Shutdown();
}

void MeshUniversalComponent::Initialize()
{
	RenderingComponent::Initialize();

	Game* OwningGame = GetOwningGame();
	if (OwningGame == nullptr)
	{
		return;
	}

	SceneViewportSubsystem* SceneViewport = OwningGame->GetSubsystem<SceneViewportSubsystem>();
	if (SceneViewport == nullptr)
	{
		return;
	}

	ID3D11Device* Device = SceneViewport->GetDevice();
	if (Device == nullptr)
	{
		return;
	}

	if (VertexShaderName.empty())
	{
		std::cerr << "VertexShaderName is not initialized." << std::endl;
		return;
	}

	if (PixelShaderName.empty())
	{
		std::cerr << "PixelShaderName is not initialized." << std::endl;
		return;
	}

	std::wstring VertexShaderFilePath(VertexShaderName.begin(), VertexShaderName.end());
	std::wstring PixelShaderFilePath(PixelShaderName.begin(), PixelShaderName.end());

	ID3DBlob* ErrorCode = nullptr;
	auto Result = D3DCompileFromFile(
		VertexShaderFilePath.c_str(),
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"VSMain",
		"vs_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&VertexShaderByteCode,
		&ErrorCode);

	if (FAILED(Result))
	{
		if (ErrorCode)
		{
			char* CompileErrors = static_cast<char*>(ErrorCode->GetBufferPointer());
			std::cout << CompileErrors << std::endl;
		}
		else
		{
			MessageBox(
				SceneViewport->GetDisplay()->GetWindowHandle(),
				L"MyVeryFirstShader.hlsl",
				L"Missing Shader File",
				MB_OK);
		}
		return;
	}

	D3D_SHADER_MACRO ShaderMacros[] = {
		"TEST", "1",
		"TCOLOR", "float4(0.0f, 1.0f, 0.0f, 1.0f)",
		nullptr, nullptr
	};

	ID3DBlob* PixelErrorCode = nullptr;
	Result = D3DCompileFromFile(
		PixelShaderFilePath.c_str(),
		ShaderMacros,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"PSMain",
		"ps_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&PixelShaderByteCode,
		&PixelErrorCode);

	if (FAILED(Result))
	{
		if (PixelErrorCode)
		{
			char* CompileErrors = static_cast<char*>(PixelErrorCode->GetBufferPointer());
			std::cout << CompileErrors << std::endl;
		}
		return;
	}

	Device->CreateVertexShader(
		VertexShaderByteCode->GetBufferPointer(),
		VertexShaderByteCode->GetBufferSize(),
		nullptr,
		&VertexShader);

	Device->CreatePixelShader(
		PixelShaderByteCode->GetBufferPointer(),
		PixelShaderByteCode->GetBufferSize(),
		nullptr,
		&PixelShader);

	D3D11_INPUT_ELEMENT_DESC InputElements[] = {
		D3D11_INPUT_ELEMENT_DESC{
			"POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT,
			0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		D3D11_INPUT_ELEMENT_DESC{
			"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT,
			0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		D3D11_INPUT_ELEMENT_DESC{
			"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,
			0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	Result = Device->CreateInputLayout(
		InputElements,
		3,
		VertexShaderByteCode->GetBufferPointer(),
		VertexShaderByteCode->GetBufferSize(),
		&Layout);

	if (FAILED(Result))
	{
		std::cerr << "Failed to create input layout. Shader input signature does not match vertex format." << std::endl;
		return;
	}

	if (Vertices.empty())
	{
		std::cerr << "Vertices are empty." << std::endl;
		return;
	}

	if (Indices.empty())
	{
		std::cerr << "Indices are empty." << std::endl;
		return;
	}

	D3D11_BUFFER_DESC VertexBufferDescription = {};
	VertexBufferDescription.Usage = D3D11_USAGE_DEFAULT;
	VertexBufferDescription.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	VertexBufferDescription.CPUAccessFlags = 0;
	VertexBufferDescription.MiscFlags = 0;
	VertexBufferDescription.StructureByteStride = 0;
	VertexBufferDescription.ByteWidth = static_cast<UINT>(sizeof(MeshUniversalVertex) * Vertices.size());

	D3D11_SUBRESOURCE_DATA VertexData = {};
	VertexData.pSysMem = Vertices.data();
	VertexData.SysMemPitch = 0;
	VertexData.SysMemSlicePitch = 0;

	Device->CreateBuffer(&VertexBufferDescription, &VertexData, &VertexBuffer);

	IndexCount = static_cast<UINT>(Indices.size());

	D3D11_BUFFER_DESC IndexBufferDescription = {};
	IndexBufferDescription.Usage = D3D11_USAGE_DEFAULT;
	IndexBufferDescription.BindFlags = D3D11_BIND_INDEX_BUFFER;
	IndexBufferDescription.CPUAccessFlags = 0;
	IndexBufferDescription.MiscFlags = 0;
	IndexBufferDescription.StructureByteStride = 0;
	IndexBufferDescription.ByteWidth = static_cast<UINT>(sizeof(unsigned int) * Indices.size());

	D3D11_SUBRESOURCE_DATA IndexData = {};
	IndexData.pSysMem = Indices.data();
	IndexData.SysMemPitch = 0;
	IndexData.SysMemSlicePitch = 0;

	Device->CreateBuffer(&IndexBufferDescription, &IndexData, &IndexBuffer);

	D3D11_BUFFER_DESC TransformBufferDescription = {};
	TransformBufferDescription.Usage = D3D11_USAGE_DEFAULT;
	TransformBufferDescription.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	TransformBufferDescription.CPUAccessFlags = 0;
	TransformBufferDescription.MiscFlags = 0;
	TransformBufferDescription.StructureByteStride = 0;
	TransformBufferDescription.ByteWidth = static_cast<UINT>(sizeof(MeshUniversalTransformBufferData));

	Result = Device->CreateBuffer(&TransformBufferDescription, nullptr, &TransformConstantBuffer);
	if (FAILED(Result))
	{
		std::cerr << "Failed to create transform constant buffer." << std::endl;
		return;
	}

	CD3D11_RASTERIZER_DESC RasterizerDescription = {};
	RasterizerDescription.CullMode = D3D11_CULL_NONE;
	RasterizerDescription.FillMode = D3D11_FILL_SOLID;

	Device->CreateRasterizerState(&RasterizerDescription, &RasterState);
}

void MeshUniversalComponent::Update(float DeltaTime)
{
	RenderingComponent::Update(DeltaTime);
}

void MeshUniversalComponent::Render(SceneViewportSubsystem* SceneViewport)
{
	ID3D11DeviceContext* DeviceContext = SceneViewport->GetDeviceContext();
	if (DeviceContext == nullptr)
	{
		return;
	}

	DeviceContext->RSSetState(RasterState);
	DeviceContext->IASetInputLayout(Layout);
	DeviceContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	DeviceContext->IASetIndexBuffer(IndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	UINT Strides[] = { static_cast<UINT>(sizeof(MeshUniversalVertex)) };
	UINT Offsets[] = { 0 };
	DeviceContext->IASetVertexBuffers(0, 1, &VertexBuffer, Strides, Offsets);

	if (TransformConstantBuffer)
	{
		Game* OwningGame = GetOwningGame();
		if (OwningGame == nullptr)
		{
			return;
		}

		float ScreenWidth = static_cast<float>(OwningGame->GetScreenWidth());
		float ScreenHeight = static_cast<float>(OwningGame->GetScreenHeight());

		if (ScreenHeight > 0.0f && GetOwningActor() != nullptr)
		{
			float AspectRatio = ScreenWidth / ScreenHeight;
			Transform WorldTransform = GetWorldTransform();
			DirectX::XMMATRIX WorldMatrix = WorldTransform.ToMatrix();
			DirectX::XMMATRIX ViewMatrix = DirectX::XMMatrixIdentity();
			DirectX::XMMATRIX ProjectionMatrix = DirectX::XMMatrixPerspectiveFovLH(
				DirectX::XMConvertToRadians(60.0f),
				AspectRatio,
				0.1f,
				100.0f);
			DirectX::XMMATRIX WorldViewProjectionMatrix = WorldMatrix * ViewMatrix * ProjectionMatrix;

			MeshUniversalTransformBufferData TransformBufferData = {};
			DirectX::XMStoreFloat4x4(
				&TransformBufferData.WorldViewProjectionMatrix,
				DirectX::XMMatrixTranspose(WorldViewProjectionMatrix));

			DeviceContext->UpdateSubresource(
				TransformConstantBuffer,
				0,
				nullptr,
				&TransformBufferData,
				0,
				0);

			DeviceContext->VSSetConstantBuffers(0, 1, &TransformConstantBuffer);
		}
	}

	DeviceContext->VSSetShader(VertexShader, nullptr, 0);
	DeviceContext->PSSetShader(PixelShader, nullptr, 0);

	DeviceContext->DrawIndexed(IndexCount, 0, 0);
}

void MeshUniversalComponent::Shutdown()
{
	if (GetIsInitialized() == false)
	{
		return;
	}

	if (Layout)
	{
		Layout->Release();
		Layout = nullptr;
	}

	if (VertexShader)
	{
		VertexShader->Release();
		VertexShader = nullptr;
	}

	if (VertexShaderByteCode)
	{
		VertexShaderByteCode->Release();
		VertexShaderByteCode = nullptr;
	}

	if (PixelShader)
	{
		PixelShader->Release();
		PixelShader = nullptr;
	}

	if (PixelShaderByteCode)
	{
		PixelShaderByteCode->Release();
		PixelShaderByteCode = nullptr;
	}

	if (TransformConstantBuffer)
	{
		TransformConstantBuffer->Release();
		TransformConstantBuffer = nullptr;
	}

	if (VertexBuffer)
	{
		VertexBuffer->Release();
		VertexBuffer = nullptr;
	}

	if (IndexBuffer)
	{
		IndexBuffer->Release();
		IndexBuffer = nullptr;
	}

	if (RasterState)
	{
		RasterState->Release();
		RasterState = nullptr;
	}

	RenderingComponent::Shutdown();
}
