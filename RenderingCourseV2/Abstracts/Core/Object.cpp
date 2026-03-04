#include "Abstracts/Core/Object.h"

Object::Object()
	: IsInitialized(false)
{
}

Object::~Object() = default;

void Object::Initialize()
{
	IsInitialized = true;
}

void Object::Update(float DeltaTime)
{
	(void)DeltaTime;
}

void Object::Shutdown()
{
	IsInitialized = false;
}

bool Object::GetIsInitialized() const
{
	return IsInitialized;
}
