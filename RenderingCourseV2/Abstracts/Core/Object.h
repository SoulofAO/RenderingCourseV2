#pragma once

class Object
{
public:
	Object();
	virtual ~Object();

	virtual void Initialize();
	virtual void Update(float DeltaTime);
	virtual void Shutdown();

	bool GetIsInitialized() const;

protected:
	bool IsInitialized;
};
