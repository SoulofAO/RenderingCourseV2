#include "Abstracts/Subsystems/Subsystem.h"

Subsystem::Subsystem()
	: OwningGame(nullptr)
{
}

Subsystem::~Subsystem() = default;

void Subsystem::SetOwningGame(Game* GameInstance)
{
	OwningGame = GameInstance;
}

Game* Subsystem::GetOwningGame() const
{
	return OwningGame;
}
