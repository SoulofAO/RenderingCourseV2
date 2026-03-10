#include "Engine/Core/Runtime/Abstract/Components/GameComponent.h"

GameComponent::GameComponent(Game* GameInstance)
	: OwningGame(GameInstance)
{
}

GameComponent::~GameComponent() = default;

