#pragma once

#include "Engine/Core/Runtime/Abstract/Core/Object.h"
#include <functional>
#include <memory>
#include <string>

class UClass : public UObject
{
public:
	using DefaultObjectFactoryFunction = std::function<std::unique_ptr<UObject>()>;
	using DefaultObjectCopyFunction = std::function<std::unique_ptr<UObject>(const UObject&)>;

	UClass(
		const std::string& InClassName,
		DefaultObjectFactoryFunction InFactoryFunction,
		DefaultObjectCopyFunction InCopyFunction);
	~UClass() override;

	const std::string& GetClassName() const;
	UObject* GetClassDefaultObject() const;

	void EnsureClassDefaultObject();
	void SetClassDefaultObject(std::unique_ptr<UObject> InClassDefaultObject);
	std::unique_ptr<UObject> SpawnFromClassDefaultObject() const;

private:
	std::string ClassName;
	std::unique_ptr<UObject> ClassDefaultObject;
	DefaultObjectFactoryFunction FactoryFunction;
	DefaultObjectCopyFunction CopyFunction;
};
