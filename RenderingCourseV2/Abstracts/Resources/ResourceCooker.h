#pragma once

#include <string>

struct ID3D11Device;

class ResourceCooker
{
public:
	ResourceCooker();

	void SetProjectRootPath(const std::string& NewProjectRootPath);
	void SetCookedRootPath(const std::string& NewCookedRootPath);

	std::string ResolveCookedPath(const std::string& SourcePath, const std::string& DefaultExtension) const;
	bool EnsureCookedTexture(const std::string& SourcePath, const std::string& CookedPath, ID3D11Device* Device) const;
	bool EnsureCookedModel(const std::string& SourcePath, const std::string& CookedPath) const;

private:
	std::string ProjectRootPath;
	std::string CookedRootPath;
};
