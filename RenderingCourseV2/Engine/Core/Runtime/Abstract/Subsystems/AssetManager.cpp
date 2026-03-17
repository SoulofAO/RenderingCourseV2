#include "Engine/Core/Runtime/Abstract/Subsystems/AssetManager.h"

AssetManager::AssetManager()
	: Subsystem(SubsystemCategory::Editor)
	, AssetRootDirectory(L"")
{
}

AssetManager::~AssetManager() = default;

void AssetManager::SetAssetRootDirectory(const std::wstring& InAssetRootDirectory)
{
	AssetRootDirectory = InAssetRootDirectory;
}

const std::wstring& AssetManager::GetAssetRootDirectory() const
{
	return AssetRootDirectory;
}

void AssetManager::RegisterAsset(const std::wstring& AssetIdentifier, const std::wstring& RelativePath)
{
	RegisteredAssetPaths[AssetIdentifier] = RelativePath;
}

bool AssetManager::TryGetAssetPath(const std::wstring& AssetIdentifier, std::wstring& OutAssetPath) const
{
	auto AssetIterator = RegisteredAssetPaths.find(AssetIdentifier);
	if (AssetIterator == RegisteredAssetPaths.end())
	{
		return false;
	}

	if (AssetRootDirectory.empty())
	{
		OutAssetPath = AssetIterator->second;
		return true;
	}

	OutAssetPath = AssetRootDirectory + L"/" + AssetIterator->second;
	return true;
}

std::vector<AssetRegistryEntry> AssetManager::GetRegisteredAssets() const
{
	std::vector<AssetRegistryEntry> AssetEntries;
	AssetEntries.reserve(RegisteredAssetPaths.size());
	for (const auto& AssetPair : RegisteredAssetPaths)
	{
		AssetEntries.push_back({ AssetPair.first, AssetPair.second });
	}

	return AssetEntries;
}
