#include "Engine/Core/Runtime/Abstract/Core/Object.h"

UObject::UObject()
	: IsInitialized(false)
{
}

UObject::~UObject() = default;

void UObject::Initialize()
{
	IsInitialized = true;
}

void UObject::Update(float DeltaTime)
{
	(void)DeltaTime;
}

void UObject::Shutdown()
{
	IsInitialized = false;
}

std::unique_ptr<UObject> UObject::Duplicate() const
{
	return std::make_unique<UObject>(*this);
}

const char* UObject::GetRuntimeClassName() const
{
	return "UObject";
}

bool UObject::GetIsInitialized() const
{
	return IsInitialized;
}

