#pragma once

#include "Engine/Core/Runtime/Abstract/Core/Object.h"
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <vector>

enum class UPropertyType : uint8_t
{
	Float,
	Float3,
	Bool,
	Integer
};

enum UPropertyFlags : uint8_t
{
	PropertyFlagNone = 0,
	PropertyFlagEditableInDetails = 1 << 0
};

struct UPropertyDescriptor
{
	const char* PropertyName;
	UPropertyType PropertyType;
	uint32_t PropertyOffset;
	uint8_t PropertyFlags;
};

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
	void AddPropertyDescriptor(const UPropertyDescriptor& NewPropertyDescriptor);
	void SetPropertyDescriptors(const std::vector<UPropertyDescriptor>& NewPropertyDescriptors);
	const std::vector<UPropertyDescriptor>& GetPropertyDescriptors() const;

private:
	std::string ClassName;
	std::unique_ptr<UObject> ClassDefaultObject;
	DefaultObjectFactoryFunction FactoryFunction;
	DefaultObjectCopyFunction CopyFunction;
	std::vector<UPropertyDescriptor> PropertyDescriptors;
};
