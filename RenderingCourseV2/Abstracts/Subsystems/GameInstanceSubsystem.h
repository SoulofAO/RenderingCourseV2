#pragma once

#include "Abstracts/Core/Object.h"

class GameInstance;

class GameInstanceSubsystem : public Object
{
public:
	GameInstanceSubsystem();
	~GameInstanceSubsystem() override;

	void SetOwningGameInstance(GameInstance* NewOwningGameInstance);
	GameInstance* GetOwningGameInstance() const;

private:
	GameInstance* OwningGameInstance;
};
