#pragma once

#include "Abstracts/Core/RenderTypes.h"
#include <d3d11.h>
#include <vector>

struct PlayerRenderTargetIdentifier
{
	int SessionIdentifier;
	int PlayerIdentifier;
};

struct PlayerRenderTargetCompositeCommand
{
	PlayerRenderTargetIdentifier Identifier;
	int DestinationPixelX;
	int DestinationPixelY;
	int DestinationWidth;
	int DestinationHeight;
};

class PlayerRenderTargetService
{
public:
	PlayerRenderTargetService();
	~PlayerRenderTargetService();

	bool EnsurePlayerRenderTarget(
		ID3D11Device* Device,
		const PlayerRenderTargetIdentifier& Identifier,
		int Width,
		int Height);
	bool GetPlayerRenderTargetOverride(
		const PlayerRenderTargetIdentifier& Identifier,
		GameRenderTargetOverride& RenderTargetOverride) const;
	void CompositeToBackBuffer(
		ID3D11DeviceContext* DeviceContext,
		ID3D11Texture2D* BackBufferTexture,
		const std::vector<PlayerRenderTargetCompositeCommand>& CompositeCommands) const;
	void ReleaseAll();

private:
	struct PlayerRenderTargetResource
	{
		PlayerRenderTargetIdentifier Identifier;
		int Width;
		int Height;
		ID3D11Texture2D* ColorTexture;
		ID3D11RenderTargetView* ColorRenderTargetView;
		ID3D11ShaderResourceView* ColorShaderResourceView;
		ID3D11Texture2D* DepthTexture;
		ID3D11DepthStencilView* DepthStencilView;
	};

	std::vector<PlayerRenderTargetResource> PlayerRenderTargets;
};
