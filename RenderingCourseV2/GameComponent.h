#pragma once

class Game;

class GameComponent
{
public:
	GameComponent(Game* GameInstance);
	virtual ~GameComponent();

	virtual void Initialize() = 0;
	virtual void Update(float DeltaTime) = 0;
	virtual void Draw() = 0;
	virtual void DestroyResources() = 0;

protected:
	Game* OwningGame;
};
