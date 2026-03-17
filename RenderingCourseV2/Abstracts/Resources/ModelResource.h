#pragma once

#include "Abstracts/Resources/ResourceBase.h"
#include <directxmath.h>
#include <vector>

struct ModelResourceVertex
{
	DirectX::XMFLOAT4 Position;
	DirectX::XMFLOAT4 Color;
	DirectX::XMFLOAT3 Normal;
	DirectX::XMFLOAT3 Tangent;
	DirectX::XMFLOAT2 TextureCoordinates;
};

class ModelResource : public ResourceBase
{
public:
	ModelResource(const std::string& NewSourcePath, const std::string& NewResolvedPath)
		: ResourceBase(ResourceType::Model, NewSourcePath, NewResolvedPath)
	{
	}

	std::vector<ModelResourceVertex> Vertices;
	std::vector<unsigned int> Indices;
};
