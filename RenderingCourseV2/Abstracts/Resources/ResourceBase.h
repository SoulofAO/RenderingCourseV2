#pragma once

#include <string>

enum class ResourceType
{
	Unknown,
	Texture,
	Model
};

class ResourceBase
{
public:
	ResourceBase(ResourceType NewResourceType, std::string NewSourcePath, std::string NewResolvedPath)
		: Type(NewResourceType)
		, SourcePath(std::move(NewSourcePath))
		, ResolvedPath(std::move(NewResolvedPath))
	{
	}

	virtual ~ResourceBase() = default;

	ResourceType GetType() const
	{
		return Type;
	}

	const std::string& GetSourcePath() const
	{
		return SourcePath;
	}

	const std::string& GetResolvedPath() const
	{
		return ResolvedPath;
	}

protected:
	ResourceType Type;
	std::string SourcePath;
	std::string ResolvedPath;
};
