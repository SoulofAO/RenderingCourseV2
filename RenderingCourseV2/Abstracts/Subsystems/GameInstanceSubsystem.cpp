#include "Abstracts/Subsystems/GameInstanceSubsystem.h"

GameInstanceSubsystem::GameInstanceSubsystem()
	: OwningGameInstance(nullptr)
{
}

GameInstanceSubsystem::~GameInstanceSubsystem() = default;

void GameInstanceSubsystem::SetOwningGameInstance(GameInstance* NewOwningGameInstance)
{
	OwningGameInstance = NewOwningGameInstance;
}

GameInstance* GameInstanceSubsystem::GetOwningGameInstance() const
{
	return OwningGameInstance;
}
