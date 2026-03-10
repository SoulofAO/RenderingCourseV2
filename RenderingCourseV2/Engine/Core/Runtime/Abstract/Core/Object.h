#pragma once

#include <memory>

class UObject
{
public:
	UObject();
	virtual ~UObject();

	virtual void Initialize();
	virtual void Update(float DeltaTime);
	virtual void Shutdown();
	virtual std::unique_ptr<UObject> Duplicate() const;

	bool GetIsInitialized() const;

protected:
	bool IsInitialized;
};

using Object = UObject;
