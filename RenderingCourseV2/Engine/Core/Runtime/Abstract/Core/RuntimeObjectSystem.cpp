#include "Engine/Core/Runtime/Abstract/Core/RuntimeObjectSystem.h"
#include <utility>

RuntimeObjectSystem& RuntimeObjectSystem::Get()
{
	static RuntimeObjectSystem RuntimeObjectSystemInstance;
	return RuntimeObjectSystemInstance;
}

RuntimeObjectSystem::RuntimeObjectSystem() = default;

RuntimeObjectSystem::~RuntimeObjectSystem() = default;

UClass* RuntimeObjectSystem::RegisterClassInternal(
	const std::string& ClassName,
	UClass::DefaultObjectFactoryFunction FactoryFunction,
	UClass::DefaultObjectCopyFunction CopyFunction)
{
	auto ExistingClassIterator = RegisteredClasses.find(ClassName);
	if (ExistingClassIterator != RegisteredClasses.end())
	{
		return ExistingClassIterator->second.get();
	}

	std::unique_ptr<UClass> NewClass = std::make_unique<UClass>(ClassName, std::move(FactoryFunction), std::move(CopyFunction));
	NewClass->EnsureClassDefaultObject();
	UClass* NewClassRawPointer = NewClass.get();
	RegisteredClasses[ClassName] = std::move(NewClass);
	return NewClassRawPointer;
}

UClass* RuntimeObjectSystem::FindClass(const std::string& ClassName) const
{
	auto ExistingClassIterator = RegisteredClasses.find(ClassName);
	if (ExistingClassIterator == RegisteredClasses.end())
	{
		return nullptr;
	}

	return ExistingClassIterator->second.get();
}

UObject* RuntimeObjectSystem::GetClassDefaultObject(const std::string& ClassName) const
{
	UClass* ExistingClass = FindClass(ClassName);
	if (ExistingClass == nullptr)
	{
		return nullptr;
	}

	return ExistingClass->GetClassDefaultObject();
}

bool RuntimeObjectSystem::SetClassDefaultObject(const std::string& ClassName, std::unique_ptr<UObject> ClassDefaultObject)
{
	UClass* ExistingClass = FindClass(ClassName);
	if (ExistingClass == nullptr || ClassDefaultObject == nullptr)
	{
		return false;
	}

	ExistingClass->SetClassDefaultObject(std::move(ClassDefaultObject));
	return true;
}

bool RuntimeObjectSystem::RegisterPropertyDescriptor(const std::string& ClassName, const UPropertyDescriptor& PropertyDescriptor)
{
	UClass* ExistingClass = FindClass(ClassName);
	if (ExistingClass == nullptr)
	{
		return false;
	}

	ExistingClass->AddPropertyDescriptor(PropertyDescriptor);
	return true;
}

bool RuntimeObjectSystem::RegisterPropertyDescriptors(const std::string& ClassName, const std::vector<UPropertyDescriptor>& PropertyDescriptors)
{
	UClass* ExistingClass = FindClass(ClassName);
	if (ExistingClass == nullptr)
	{
		return false;
	}

	ExistingClass->SetPropertyDescriptors(PropertyDescriptors);
	return true;
}

std::unique_ptr<UObject> RuntimeObjectSystem::SpawnObject(const std::string& ClassName) const
{
	UClass* ExistingClass = FindClass(ClassName);
	if (ExistingClass == nullptr)
	{
		return nullptr;
	}

	return ExistingClass->SpawnFromClassDefaultObject();
}
