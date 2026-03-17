#pragma once

#include "Abstracts/Resources/ResourceBase.h"
#include <d3d11.h>
#include <wrl/client.h>

class TextureResource : public ResourceBase
{
public:
	TextureResource(const std::string& NewSourcePath, const std::string& NewResolvedPath)
		: ResourceBase(ResourceType::Texture, NewSourcePath, NewResolvedPath)
	{
	}

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> ShaderResourceView;
};
