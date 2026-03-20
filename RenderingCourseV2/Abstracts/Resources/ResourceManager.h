#pragma once

#include "Abstracts/Resources/ModelResource.h"
#include "Abstracts/Resources/ResourceCooker.h"
#include "Abstracts/Resources/TextureResource.h"
#include <memory>
#include <string>
#include <unordered_map>

struct ID3D11Device;

class ResourceManager
{
public:
	ResourceManager();

	void Initialize(const std::string& NewProjectRootPath, const std::string& NewCookedRootPath);

	std::shared_ptr<TextureResource> LoadTextureResource(const std::string& SourcePath, ID3D11Device* Device);
	std::shared_ptr<ModelResource> LoadModelResource(const std::string& SourcePath);
	bool ForceRebuildInputResources(ID3D11Device* Device);

private:
	std::string NormalizePath(const std::string& Path) const;

	ResourceCooker Cooker;
	std::string ProjectRootPath;
	std::unordered_map<std::string, std::shared_ptr<TextureResource>> TextureResourceCache;
	std::unordered_map<std::string, std::shared_ptr<ModelResource>> ModelResourceCache;
};
