#include "TriangleComponent.h"
#include "Engine/Core/Runtime/Abstract/Core/Game.h"
#include "Engine/Core/Runtime/Abstract/Subsystems/SceneViewportSubsystem.h"
#include "Engine/Core/Runtime/Abstract/Subsystems/DisplayWin32.h"
#include <d3dcompiler.h>
#include <directxmath.h>
#include <iostream>

#pragma comment(lib, "d3dcompiler.lib")

TriangleComponent::TriangleComponent()
	: RenderingComponent()
	, Layout(nullptr)
	, VertexShader(nullptr)
	, VertexShaderByteCode(nullptr)
	, PixelShader(nullptr)
	, PixelShaderByteCode(nullptr)
	, VertexBuffer(nullptr)
	, IndexBuffer(nullptr)
	, RasterState(nullptr)
	, IndexCount(0)
{
}

TriangleComponent::~TriangleComponent()
{
	Shutdown();
}

void TriangleComponent::Initialize()
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

	ID3DBlob* ErrorCode = nullptr;
	auto Result = D3DCompileFromFile(
		L"./Shaders/FirstTask/MyVeryFirstShader.hlsl",
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
		L"./Shaders/FirstTask/MyVeryFirstShader.hlsl",
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
			0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	Device->CreateInputLayout(
		InputElements,
		2,
		VertexShaderByteCode->GetBufferPointer(),
		VertexShaderByteCode->GetBufferSize(),
		&Layout);

	DirectX::XMFLOAT4 Points[8] = {
		DirectX::XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f),   DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f),
		DirectX::XMFLOAT4(-0.5f, -0.5f, 0.5f, 1.0f),  DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f),
		DirectX::XMFLOAT4(0.5f, -0.5f, 0.5f, 1.0f),   DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f),
		DirectX::XMFLOAT4(-0.5f, 0.5f, 0.5f, 1.0f),   DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
	};

	D3D11_BUFFER_DESC VertexBufferDescription = {};
	VertexBufferDescription.Usage = D3D11_USAGE_DEFAULT;
	VertexBufferDescription.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	VertexBufferDescription.CPUAccessFlags = 0;
	VertexBufferDescription.MiscFlags = 0;
	VertexBufferDescription.StructureByteStride = 0;
	VertexBufferDescription.ByteWidth = sizeof(DirectX::XMFLOAT4) * std::size(Points);

	D3D11_SUBRESOURCE_DATA VertexData = {};
	VertexData.pSysMem = Points;
	VertexData.SysMemPitch = 0;
	VertexData.SysMemSlicePitch = 0;

	Device->CreateBuffer(&VertexBufferDescription, &VertexData, &VertexBuffer);

	int Indices[] = { 0, 1, 2, 1, 0, 3 };
	IndexCount = static_cast<UINT>(std::size(Indices));

	D3D11_BUFFER_DESC IndexBufferDescription = {};
	IndexBufferDescription.Usage = D3D11_USAGE_DEFAULT;
	IndexBufferDescription.BindFlags = D3D11_BIND_INDEX_BUFFER;
	IndexBufferDescription.CPUAccessFlags = 0;
	IndexBufferDescription.MiscFlags = 0;
	IndexBufferDescription.StructureByteStride = 0;
	IndexBufferDescription.ByteWidth = sizeof(int) * IndexCount;

	D3D11_SUBRESOURCE_DATA IndexData = {};
	IndexData.pSysMem = Indices;
	IndexData.SysMemPitch = 0;
	IndexData.SysMemSlicePitch = 0;
	
	Device->CreateBuffer(&IndexBufferDescription, &IndexData, &IndexBuffer);

	CD3D11_RASTERIZER_DESC RasterizerDescription = {};
	RasterizerDescription.CullMode = D3D11_CULL_NONE;
	RasterizerDescription.FillMode = D3D11_FILL_SOLID;

	Device->CreateRasterizerState(&RasterizerDescription, &RasterState);
}

void TriangleComponent::Update(float DeltaTime)
{
	RenderingComponent::Update(DeltaTime);
}

void TriangleComponent::Render(SceneViewportSubsystem* SceneViewport)
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

	UINT Strides[] = { 32 };
	UINT Offsets[] = { 0 };
	DeviceContext->IASetVertexBuffers(0, 1, &VertexBuffer, Strides, Offsets);

	DeviceContext->VSSetShader(VertexShader, nullptr, 0);
	DeviceContext->PSSetShader(PixelShader, nullptr, 0);

	DeviceContext->DrawIndexed(IndexCount, 0, 0);
}

void TriangleComponent::Shutdown()
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

