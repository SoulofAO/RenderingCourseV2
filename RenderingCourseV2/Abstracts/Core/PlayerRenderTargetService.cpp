#include "Abstracts/Core/PlayerRenderTargetService.h"

PlayerRenderTargetService::PlayerRenderTargetService() = default;

PlayerRenderTargetService::~PlayerRenderTargetService()
{
	ReleaseAll();
}

bool PlayerRenderTargetService::EnsurePlayerRenderTarget(
	ID3D11Device* Device,
	const PlayerRenderTargetIdentifier& Identifier,
	int Width,
	int Height)
{
	if (Device == nullptr || Width <= 0 || Height <= 0)
	{
		return false;
	}

	PlayerRenderTargetResource* ExistingResource = nullptr;
	for (PlayerRenderTargetResource& ExistingPlayerRenderTarget : PlayerRenderTargets)
	{
		if (
			ExistingPlayerRenderTarget.Identifier.SessionIdentifier == Identifier.SessionIdentifier &&
			ExistingPlayerRenderTarget.Identifier.PlayerIdentifier == Identifier.PlayerIdentifier)
		{
			ExistingResource = &ExistingPlayerRenderTarget;
			break;
		}
	}

	if (ExistingResource != nullptr)
	{
		if (ExistingResource->Width == Width && ExistingResource->Height == Height && ExistingResource->ColorRenderTargetView != nullptr && ExistingResource->DepthStencilView != nullptr)
		{
			return true;
		}
	}
	else
	{
		PlayerRenderTargetResource NewPlayerRenderTarget = {};
		NewPlayerRenderTarget.Identifier = Identifier;
		PlayerRenderTargets.push_back(NewPlayerRenderTarget);
		ExistingResource = &PlayerRenderTargets.back();
	}

	if (ExistingResource->ColorShaderResourceView != nullptr)
	{
		ExistingResource->ColorShaderResourceView->Release();
		ExistingResource->ColorShaderResourceView = nullptr;
	}
	if (ExistingResource->ColorRenderTargetView != nullptr)
	{
		ExistingResource->ColorRenderTargetView->Release();
		ExistingResource->ColorRenderTargetView = nullptr;
	}
	if (ExistingResource->ColorTexture != nullptr)
	{
		ExistingResource->ColorTexture->Release();
		ExistingResource->ColorTexture = nullptr;
	}
	if (ExistingResource->DepthStencilView != nullptr)
	{
		ExistingResource->DepthStencilView->Release();
		ExistingResource->DepthStencilView = nullptr;
	}
	if (ExistingResource->DepthTexture != nullptr)
	{
		ExistingResource->DepthTexture->Release();
		ExistingResource->DepthTexture = nullptr;
	}

	D3D11_TEXTURE2D_DESC ColorTextureDescription = {};
	ColorTextureDescription.Width = static_cast<UINT>(Width);
	ColorTextureDescription.Height = static_cast<UINT>(Height);
	ColorTextureDescription.MipLevels = 1;
	ColorTextureDescription.ArraySize = 1;
	ColorTextureDescription.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	ColorTextureDescription.SampleDesc.Count = 1;
	ColorTextureDescription.Usage = D3D11_USAGE_DEFAULT;
	ColorTextureDescription.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	if (FAILED(Device->CreateTexture2D(&ColorTextureDescription, nullptr, &ExistingResource->ColorTexture)))
	{
		return false;
	}
	if (FAILED(Device->CreateRenderTargetView(ExistingResource->ColorTexture, nullptr, &ExistingResource->ColorRenderTargetView)))
	{
		return false;
	}
	if (FAILED(Device->CreateShaderResourceView(ExistingResource->ColorTexture, nullptr, &ExistingResource->ColorShaderResourceView)))
	{
		return false;
	}

	D3D11_TEXTURE2D_DESC DepthTextureDescription = {};
	DepthTextureDescription.Width = static_cast<UINT>(Width);
	DepthTextureDescription.Height = static_cast<UINT>(Height);
	DepthTextureDescription.MipLevels = 1;
	DepthTextureDescription.ArraySize = 1;
	DepthTextureDescription.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	DepthTextureDescription.SampleDesc.Count = 1;
	DepthTextureDescription.Usage = D3D11_USAGE_DEFAULT;
	DepthTextureDescription.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	if (FAILED(Device->CreateTexture2D(&DepthTextureDescription, nullptr, &ExistingResource->DepthTexture)))
	{
		return false;
	}
	if (FAILED(Device->CreateDepthStencilView(ExistingResource->DepthTexture, nullptr, &ExistingResource->DepthStencilView)))
	{
		return false;
	}

	ExistingResource->Width = Width;
	ExistingResource->Height = Height;
	return true;
}

bool PlayerRenderTargetService::GetPlayerRenderTargetOverride(
	const PlayerRenderTargetIdentifier& Identifier,
	GameRenderTargetOverride& RenderTargetOverride) const
{
	for (const PlayerRenderTargetResource& ExistingPlayerRenderTarget : PlayerRenderTargets)
	{
		if (
			ExistingPlayerRenderTarget.Identifier.SessionIdentifier == Identifier.SessionIdentifier &&
			ExistingPlayerRenderTarget.Identifier.PlayerIdentifier == Identifier.PlayerIdentifier)
		{
			if (ExistingPlayerRenderTarget.ColorRenderTargetView == nullptr || ExistingPlayerRenderTarget.DepthStencilView == nullptr)
			{
				return false;
			}

			RenderTargetOverride.RenderTargetView = ExistingPlayerRenderTarget.ColorRenderTargetView;
			RenderTargetOverride.DepthStencilView = ExistingPlayerRenderTarget.DepthStencilView;
			RenderTargetOverride.Width = ExistingPlayerRenderTarget.Width;
			RenderTargetOverride.Height = ExistingPlayerRenderTarget.Height;
			return true;
		}
	}

	return false;
}

void PlayerRenderTargetService::CompositeToBackBuffer(
	ID3D11DeviceContext* DeviceContext,
	ID3D11Texture2D* BackBufferTexture,
	const std::vector<PlayerRenderTargetCompositeCommand>& CompositeCommands) const
{
	if (DeviceContext == nullptr || BackBufferTexture == nullptr)
	{
		return;
	}

	for (const PlayerRenderTargetCompositeCommand& ExistingCompositeCommand : CompositeCommands)
	{
		for (const PlayerRenderTargetResource& ExistingPlayerRenderTarget : PlayerRenderTargets)
		{
			if (
				ExistingPlayerRenderTarget.Identifier.SessionIdentifier != ExistingCompositeCommand.Identifier.SessionIdentifier ||
				ExistingPlayerRenderTarget.Identifier.PlayerIdentifier != ExistingCompositeCommand.Identifier.PlayerIdentifier ||
				ExistingPlayerRenderTarget.ColorTexture == nullptr)
			{
				continue;
			}

			D3D11_BOX SourceBox = {};
			SourceBox.left = 0;
			SourceBox.top = 0;
			SourceBox.front = 0;
			SourceBox.right = static_cast<UINT>(ExistingPlayerRenderTarget.Width);
			SourceBox.bottom = static_cast<UINT>(ExistingPlayerRenderTarget.Height);
			SourceBox.back = 1;
			DeviceContext->CopySubresourceRegion(
				BackBufferTexture,
				0,
				static_cast<UINT>(ExistingCompositeCommand.DestinationPixelX),
				static_cast<UINT>(ExistingCompositeCommand.DestinationPixelY),
				0,
				ExistingPlayerRenderTarget.ColorTexture,
				0,
				&SourceBox);
			break;
		}
	}
}

void PlayerRenderTargetService::ReleaseAll()
{
	for (PlayerRenderTargetResource& ExistingPlayerRenderTarget : PlayerRenderTargets)
	{
		if (ExistingPlayerRenderTarget.ColorShaderResourceView != nullptr)
		{
			ExistingPlayerRenderTarget.ColorShaderResourceView->Release();
			ExistingPlayerRenderTarget.ColorShaderResourceView = nullptr;
		}
		if (ExistingPlayerRenderTarget.ColorRenderTargetView != nullptr)
		{
			ExistingPlayerRenderTarget.ColorRenderTargetView->Release();
			ExistingPlayerRenderTarget.ColorRenderTargetView = nullptr;
		}
		if (ExistingPlayerRenderTarget.ColorTexture != nullptr)
		{
			ExistingPlayerRenderTarget.ColorTexture->Release();
			ExistingPlayerRenderTarget.ColorTexture = nullptr;
		}
		if (ExistingPlayerRenderTarget.DepthStencilView != nullptr)
		{
			ExistingPlayerRenderTarget.DepthStencilView->Release();
			ExistingPlayerRenderTarget.DepthStencilView = nullptr;
		}
		if (ExistingPlayerRenderTarget.DepthTexture != nullptr)
		{
			ExistingPlayerRenderTarget.DepthTexture->Release();
			ExistingPlayerRenderTarget.DepthTexture = nullptr;
		}
	}
	PlayerRenderTargets.clear();
}
