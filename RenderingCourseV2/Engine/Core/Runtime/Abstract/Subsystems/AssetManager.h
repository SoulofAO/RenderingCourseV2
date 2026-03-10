#pragma once

#include "Engine/Core/Runtime/Abstract/Subsystems/Subsystem.h"
#include <string>
#include <unordered_map>

class AssetManager : public Subsystem
{
public:
	AssetManager();
	~AssetManager() override;

	void SetAssetRootDirectory(const std::wstring& InAssetRootDirectory);
	const std::wstring& GetAssetRootDirectory() const;

	void RegisterAsset(const std::wstring& AssetIdentifier, const std::wstring& RelativePath);
	bool TryGetAssetPath(const std::wstring& AssetIdentifier, std::wstring& OutAssetPath) const;

private:
	std::wstring AssetRootDirectory;
	std::unordered_map<std::wstring, std::wstring> RegisteredAssetPaths;
};
