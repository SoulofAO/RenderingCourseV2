#include "Abstracts/Resources/ResourceManager.h"
#include <d3d11.h>
#include <filesystem>

#include <DirectXTex.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

ResourceManager::ResourceManager()
	: Cooker()
	, ProjectRootPath()
	, TextureResourceCache()
	, ModelResourceCache()
{
}

void ResourceManager::Initialize(const std::string& NewProjectRootPath, const std::string& NewCookedRootPath)
{
	ProjectRootPath = NewProjectRootPath;
	Cooker.SetProjectRootPath(NewProjectRootPath);
	Cooker.SetCookedRootPath(NewCookedRootPath);
}

std::shared_ptr<TextureResource> ResourceManager::LoadTextureResource(const std::string& SourcePath, ID3D11Device* Device)
{
	const std::string NormalizedSourcePath = NormalizePath(SourcePath);
	auto ExistingTextureIterator = TextureResourceCache.find(NormalizedSourcePath);
	if (ExistingTextureIterator != TextureResourceCache.end())
	{
		return ExistingTextureIterator->second;
	}

	const std::string CookedTexturePath = Cooker.ResolveCookedPath(NormalizedSourcePath, ".dds");
	Cooker.EnsureCookedTexture(NormalizedSourcePath, CookedTexturePath, Device);

	std::shared_ptr<TextureResource> NewTextureResource = std::make_shared<TextureResource>(NormalizedSourcePath, CookedTexturePath);

	if (Device != nullptr)
	{
		std::filesystem::path CookedTextureFilePath = std::filesystem::u8path(CookedTexturePath);
		std::wstring CookedTextureWidePath = CookedTextureFilePath.wstring();
		DirectX::ScratchImage LoadedImage;
		HRESULT LoadResult = DirectX::LoadFromDDSFile(CookedTextureWidePath.c_str(), DirectX::DDS_FLAGS_NONE, nullptr, LoadedImage);
		if (SUCCEEDED(LoadResult))
		{
			HRESULT CreateResult = DirectX::CreateShaderResourceView(
				Device,
				LoadedImage.GetImages(),
				LoadedImage.GetImageCount(),
				LoadedImage.GetMetadata(),
				NewTextureResource->ShaderResourceView.ReleaseAndGetAddressOf());
			if (FAILED(CreateResult))
			{
				NewTextureResource->ShaderResourceView.Reset();
			}
		}
	}

	TextureResourceCache.insert({ NormalizedSourcePath, NewTextureResource });
	return NewTextureResource;
}

std::shared_ptr<ModelResource> ResourceManager::LoadModelResource(const std::string& SourcePath)
{
	const std::string NormalizedSourcePath = NormalizePath(SourcePath);
	auto ExistingModelIterator = ModelResourceCache.find(NormalizedSourcePath);
	if (ExistingModelIterator != ModelResourceCache.end())
	{
		return ExistingModelIterator->second;
	}

	const std::string CookedModelPath = Cooker.ResolveCookedPath(NormalizedSourcePath, ".fbx");
	Cooker.EnsureCookedModel(NormalizedSourcePath, CookedModelPath);
	std::shared_ptr<ModelResource> NewModelResource = std::make_shared<ModelResource>(NormalizedSourcePath, CookedModelPath);

	Assimp::Importer Importer;
	const aiScene* ImportedScene = Importer.ReadFile(
		CookedModelPath,
		aiProcess_Triangulate |
		aiProcess_GenSmoothNormals |
		aiProcess_CalcTangentSpace |
		aiProcess_JoinIdenticalVertices);

	if (ImportedScene != nullptr && ImportedScene->HasMeshes())
	{
		const aiMesh* ImportedMesh = ImportedScene->mMeshes[0];
		NewModelResource->Vertices.reserve(ImportedMesh->mNumVertices);
		for (unsigned int VertexIndex = 0; VertexIndex < ImportedMesh->mNumVertices; ++VertexIndex)
		{
			ModelResourceVertex NewVertex = {};
			NewVertex.Position = DirectX::XMFLOAT4(
				ImportedMesh->mVertices[VertexIndex].x,
				ImportedMesh->mVertices[VertexIndex].y,
				ImportedMesh->mVertices[VertexIndex].z,
				1.0f);
			NewVertex.Color = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
			if (ImportedMesh->HasNormals())
			{
				NewVertex.Normal = DirectX::XMFLOAT3(
					ImportedMesh->mNormals[VertexIndex].x,
					ImportedMesh->mNormals[VertexIndex].y,
					ImportedMesh->mNormals[VertexIndex].z);
			}
			else
			{
				NewVertex.Normal = DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f);
			}

			if (ImportedMesh->HasTangentsAndBitangents())
			{
				NewVertex.Tangent = DirectX::XMFLOAT3(
					ImportedMesh->mTangents[VertexIndex].x,
					ImportedMesh->mTangents[VertexIndex].y,
					ImportedMesh->mTangents[VertexIndex].z);
			}
			else
			{
				NewVertex.Tangent = DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f);
			}

			if (ImportedMesh->HasTextureCoords(0))
			{
				NewVertex.TextureCoordinates = DirectX::XMFLOAT2(
					ImportedMesh->mTextureCoords[0][VertexIndex].x,
					ImportedMesh->mTextureCoords[0][VertexIndex].y);
			}
			else
			{
				NewVertex.TextureCoordinates = DirectX::XMFLOAT2(0.0f, 0.0f);
			}

			NewModelResource->Vertices.push_back(NewVertex);
		}

		for (unsigned int FaceIndex = 0; FaceIndex < ImportedMesh->mNumFaces; ++FaceIndex)
		{
			const aiFace& ImportedFace = ImportedMesh->mFaces[FaceIndex];
			for (unsigned int IndexPosition = 0; IndexPosition < ImportedFace.mNumIndices; ++IndexPosition)
			{
				NewModelResource->Indices.push_back(ImportedFace.mIndices[IndexPosition]);
			}
		}
	}

	ModelResourceCache.insert({ NormalizedSourcePath, NewModelResource });
	return NewModelResource;
}

std::string ResourceManager::NormalizePath(const std::string& Path) const
{
	namespace FileSystem = std::filesystem;
	std::error_code ErrorCode;
	FileSystem::path SourcePath = FileSystem::u8path(Path);
	if (SourcePath.is_relative() && ProjectRootPath.empty() == false)
	{
		SourcePath = FileSystem::u8path(ProjectRootPath) / SourcePath;
	}

	FileSystem::path NormalizedPath = FileSystem::weakly_canonical(SourcePath, ErrorCode);
	if (ErrorCode)
	{
		NormalizedPath = SourcePath.lexically_normal();
	}
	return NormalizedPath.string();
}
