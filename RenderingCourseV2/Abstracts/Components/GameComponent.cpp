#include "Abstracts/Components/GameComponent.h"

GameComponent::GameComponent(Game* GameInstance)
	: OwningGame(GameInstance)
{
}

GameComponent::~GameComponent() = default;
