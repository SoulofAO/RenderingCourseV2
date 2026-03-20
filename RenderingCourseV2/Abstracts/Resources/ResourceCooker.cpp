#include "Abstracts/Resources/ResourceCooker.h"
#include <d3d11.h>
#include <filesystem>

#include <comdef.h>
#include <iostream>
#include <iomanip>
#include <utility>

#include <DirectXTex.h>

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

bool ResourceCooker::EnsureCookedTexture(const std::string& SourcePath, const std::string& CookedPath, ID3D11Device*) const
{
	namespace FileSystem = std::filesystem;

	if (FileSystem::exists(CookedPath))
	{
		std::wstring ExistingCookedWidePath(CookedPath.begin(), CookedPath.end());
		DirectX::TexMetadata ExistingMetadata = {};
		HRESULT ExistingMetadataResult = DirectX::GetMetadataFromDDSFile(ExistingCookedWidePath.c_str(), DirectX::DDS_FLAGS_NONE, ExistingMetadata);
		if (SUCCEEDED(ExistingMetadataResult))
		{
			const bool ExistingTextureUsesBlockCompression = DirectX::IsCompressed(ExistingMetadata.format);
			const bool ExistingTextureHasValidBlockDimensions =
				(ExistingMetadata.width % 4 == 0) &&
				(ExistingMetadata.height % 4 == 0);
			if (!ExistingTextureUsesBlockCompression || ExistingTextureHasValidBlockDimensions)
			{
				return true;
			}
		}

		std::error_code RemoveErrorCode;
		FileSystem::remove(CookedPath, RemoveErrorCode);
	}

	FileSystem::create_directories(FileSystem::path(CookedPath).parent_path());

	FileSystem::path SourceFilePath = FileSystem::u8path(SourcePath);
	if (FileSystem::exists(SourceFilePath) == false)
	{
		return false;
	}

	DirectX::ScratchImage LoadedImage;
	std::wstring SourceWidePath = SourceFilePath.wstring();
	HRESULT LoadResult = DirectX::LoadFromWICFile(SourceWidePath.c_str(), DirectX::WIC_FLAGS_FORCE_RGB, nullptr, LoadedImage);
	if (FAILED(LoadResult))
	{
		return false;
	}

	DirectX::ScratchImage CompressedImage;
	const DirectX::TexMetadata& Metadata = LoadedImage.GetMetadata();
	const bool HasBlockCompressionCompatibleDimensions =
		(Metadata.width % 4 == 0) &&
		(Metadata.height % 4 == 0);
	if (HasBlockCompressionCompatibleDimensions)
	{
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
	}
	else
	{
		if (Metadata.format == DXGI_FORMAT_R8G8B8A8_UNORM)
		{
			CompressedImage = std::move(LoadedImage);
		}
		else
		{
			HRESULT ConvertResult = DirectX::Convert(
				LoadedImage.GetImages(),
				LoadedImage.GetImageCount(),
				Metadata,
				DXGI_FORMAT_R8G8B8A8_UNORM,
				DirectX::TEX_FILTER_DEFAULT,
				DirectX::TEX_THRESHOLD_DEFAULT,
				CompressedImage);
			if (FAILED(ConvertResult))
			{
				_com_error Error(ConvertResult);
				std::wcerr
					<< L"DirectX::Convert failed: 0x"
					<< std::hex << std::uppercase
					<< static_cast<unsigned long>(ConvertResult)
					<< L" (" << Error.ErrorMessage() << L")\n";
				return false;
			}
		}
	}

	std::wstring CookedWidePath = FileSystem::u8path(CookedPath).wstring();
	HRESULT SaveResult = DirectX::SaveToDDSFile(
		CompressedImage.GetImages(),
		CompressedImage.GetImageCount(),
		CompressedImage.GetMetadata(),
		DirectX::DDS_FLAGS_NONE,
		CookedWidePath.c_str());
	return SUCCEEDED(SaveResult);
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
