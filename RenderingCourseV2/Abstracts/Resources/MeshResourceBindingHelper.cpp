#include "Abstracts/Resources/MeshResourceBindingHelper.h"
#include "Abstracts/Resources/ResourceManager.h"
#include "Abstracts/Resources/TextureResource.h"

Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> MeshResourceBindingHelper::LoadTexture(
	ResourceManager* Resources,
	ID3D11Device* Device,
	const std::string& TexturePath)
{
	if (Resources == nullptr || TexturePath.empty())
	{
		return nullptr;
	}

	std::shared_ptr<TextureResource> ExistingTextureResource = Resources->LoadTextureResource(TexturePath, Device);
	if (!ExistingTextureResource)
	{
		return nullptr;
	}

	return ExistingTextureResource->ShaderResourceView;
}
