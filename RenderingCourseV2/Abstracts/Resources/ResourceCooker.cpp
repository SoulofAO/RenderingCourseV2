#include "Abstracts/Resources/ResourceCooker.h"
#include <d3d11.h>
#include <filesystem>

#if __has_include(<DirectXTex.h>)
#include <DirectXTex.h>
#define HAS_DIRECTX_TEX 1
#else
#define HAS_DIRECTX_TEX 0
#endif

ResourceCooker::ResourceCooker()
	: ProjectRootPath()
	, CookedRootPath()
{
}

void ResourceCooker::SetProjectRootPath(const std::string& NewProjectRootPath)
{
	ProjectRootPath = NewProjectRootPath;
}

void ResourceCooker::SetCookedRootPath(const std::string& NewCookedRootPath)
{
	CookedRootPath = NewCookedRootPath;
}

std::string ResourceCooker::ResolveCookedPath(const std::string& SourcePath, const std::string& DefaultExtension) const
{
	namespace FileSystem = std::filesystem;

	FileSystem::path SourceFilePath = FileSystem::path(SourcePath);
	if (SourceFilePath.is_relative())
	{
		SourceFilePath = FileSystem::path(ProjectRootPath) / SourceFilePath;
	}

	std::error_code CanonicalErrorCode;
	SourceFilePath = FileSystem::weakly_canonical(SourceFilePath, CanonicalErrorCode);
	if (CanonicalErrorCode)
	{
		SourceFilePath = SourceFilePath.lexically_normal();
	}
	FileSystem::path RelativePath = SourceFilePath.filename();
	if (!ProjectRootPath.empty())
	{
		std::error_code ProjectCanonicalErrorCode;
		FileSystem::path ProjectRootFilePath = FileSystem::weakly_canonical(FileSystem::path(ProjectRootPath), ProjectCanonicalErrorCode);
		if (ProjectCanonicalErrorCode)
		{
			ProjectRootFilePath = FileSystem::path(ProjectRootPath).lexically_normal();
		}
		std::error_code ErrorCode;
		FileSystem::path CandidateRelativePath = FileSystem::relative(SourceFilePath, ProjectRootFilePath, ErrorCode);
		if (!ErrorCode)
		{
			RelativePath = CandidateRelativePath;
		}
	}

	FileSystem::path CookedFilePath = FileSystem::path(CookedRootPath) / RelativePath;
	if (!DefaultExtension.empty())
	{
		CookedFilePath.replace_extension(DefaultExtension);
	}

	return CookedFilePath.string();
}

bool ResourceCooker::EnsureCookedTexture(const std::string& SourcePath, const std::string& CookedPath, ID3D11Device* Device) const
{
	namespace FileSystem = std::filesystem;

	if (FileSystem::exists(CookedPath))
	{
		return true;
	}

	FileSystem::create_directories(FileSystem::path(CookedPath).parent_path());

#if HAS_DIRECTX_TEX
	(void)Device;
	DirectX::ScratchImage LoadedImage;
	std::wstring SourceWidePath(SourcePath.begin(), SourcePath.end());
	HRESULT LoadResult = DirectX::LoadFromWICFile(SourceWidePath.c_str(), DirectX::WIC_FLAGS_NONE, nullptr, LoadedImage);
	if (FAILED(LoadResult))
	{
		return false;
	}

	DirectX::ScratchImage CompressedImage;
	const DirectX::TexMetadata& Metadata = LoadedImage.GetMetadata();
	HRESULT CompressResult = DirectX::Compress(
		LoadedImage.GetImages(),
		LoadedImage.GetImageCount(),
		Metadata,
		DXGI_FORMAT_BC3_UNORM,
		DirectX::TEX_COMPRESS_DEFAULT,
		DirectX::TEX_THRESHOLD_DEFAULT,
		CompressedImage);
	if (FAILED(CompressResult))
	{
		return false;
	}

	std::wstring CookedWidePath(CookedPath.begin(), CookedPath.end());
	HRESULT SaveResult = DirectX::SaveToDDSFile(
		CompressedImage.GetImages(),
		CompressedImage.GetImageCount(),
		CompressedImage.GetMetadata(),
		DirectX::DDS_FLAGS_NONE,
		CookedWidePath.c_str());
	return SUCCEEDED(SaveResult);
#else
	(void)SourcePath;
	(void)CookedPath;
	(void)Device;
	return false;
#endif
}

bool ResourceCooker::EnsureCookedModel(const std::string& SourcePath, const std::string& CookedPath) const
{
	namespace FileSystem = std::filesystem;

	if (FileSystem::exists(CookedPath))
	{
		return true;
	}

	if (!FileSystem::exists(SourcePath))
	{
		return false;
	}

	FileSystem::create_directories(FileSystem::path(CookedPath).parent_path());
	std::error_code CopyErrorCode;
	FileSystem::copy_file(SourcePath, CookedPath, FileSystem::copy_options::overwrite_existing, CopyErrorCode);
	if (CopyErrorCode)
	{
		return false;
	}
	return FileSystem::exists(CookedPath);
}
