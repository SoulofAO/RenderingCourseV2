#pragma once

#include "Engine/Core/Runtime/Abstract/Core/Object.h"
#include "Engine/Core/Runtime/Abstract/Core/UClass.h"
#include <memory>
#include <string>
#include <unordered_map>

class RuntimeObjectSystem : public UObject
{
public:
	static RuntimeObjectSystem& Get();

	RuntimeObjectSystem(const RuntimeObjectSystem&) = delete;
	RuntimeObjectSystem& operator=(const RuntimeObjectSystem&) = delete;

	template<typename TObjectType>
	UClass* RegisterClass(const std::string& ClassName)
	{
		auto FactoryFunction = []() -> std::unique_ptr<UObject>
		{
			return std::make_unique<TObjectType>();
		};

		auto CopyFunction = [](const UObject& ClassDefaultObject) -> std::unique_ptr<UObject>
		{
			const TObjectType* TypedObject = dynamic_cast<const TObjectType*>(&ClassDefaultObject);
			if (TypedObject == nullptr)
			{
				return nullptr;
			}

			return std::make_unique<TObjectType>(*TypedObject);
		};

		return RegisterClassInternal(ClassName, std::move(FactoryFunction), std::move(CopyFunction));
	}

	UClass* RegisterClassInternal(
		const std::string& ClassName,
		UClass::DefaultObjectFactoryFunction FactoryFunction,
		UClass::DefaultObjectCopyFunction CopyFunction);

	UClass* FindClass(const std::string& ClassName) const;
	UObject* GetClassDefaultObject(const std::string& ClassName) const;
	bool SetClassDefaultObject(const std::string& ClassName, std::unique_ptr<UObject> ClassDefaultObject);
	bool RegisterPropertyDescriptor(const std::string& ClassName, const UPropertyDescriptor& PropertyDescriptor);
	bool RegisterPropertyDescriptors(const std::string& ClassName, const std::vector<UPropertyDescriptor>& PropertyDescriptors);
	std::unique_ptr<UObject> SpawnObject(const std::string& ClassName) const;

private:
	RuntimeObjectSystem();
	~RuntimeObjectSystem() override;

	std::unordered_map<std::string, std::unique_ptr<UClass>> RegisteredClasses;
};
