#include "Engine/Core/Runtime/Abstract/Core/UClass.h"
#include <utility>

UClass::UClass(
	const std::string& InClassName,
	DefaultObjectFactoryFunction InFactoryFunction,
	DefaultObjectCopyFunction InCopyFunction)
	: ClassName(InClassName)
	, ClassDefaultObject(nullptr)
	, FactoryFunction(std::move(InFactoryFunction))
	, CopyFunction(std::move(InCopyFunction))
{
}

UClass::~UClass() = default;

const std::string& UClass::GetClassName() const
{
	return ClassName;
}

UObject* UClass::GetClassDefaultObject() const
{
	return ClassDefaultObject.get();
}

void UClass::EnsureClassDefaultObject()
{
	if (ClassDefaultObject == nullptr && FactoryFunction)
	{
		ClassDefaultObject = FactoryFunction();
	}
}

void UClass::SetClassDefaultObject(std::unique_ptr<UObject> InClassDefaultObject)
{
	ClassDefaultObject = std::move(InClassDefaultObject);
}

std::unique_ptr<UObject> UClass::SpawnFromClassDefaultObject() const
{
	if (ClassDefaultObject == nullptr || CopyFunction == nullptr)
	{
		return nullptr;
	}

	return CopyFunction(*ClassDefaultObject);
}

void UClass::AddPropertyDescriptor(const UPropertyDescriptor& NewPropertyDescriptor)
{
	PropertyDescriptors.push_back(NewPropertyDescriptor);
}

void UClass::SetPropertyDescriptors(const std::vector<UPropertyDescriptor>& NewPropertyDescriptors)
{
	PropertyDescriptors = NewPropertyDescriptors;
}

const std::vector<UPropertyDescriptor>& UClass::GetPropertyDescriptors() const
{
	return PropertyDescriptors;
}
