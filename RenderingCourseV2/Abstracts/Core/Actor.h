#pragma once

#include "Abstracts/Core/Object.h"
#include <directxmath.h>
#include <memory>
#include <vector>

class Game;
class ActorComponent;

class Actor : public Object
{
public:
	Actor();
	~Actor() override;

	void SetOwningGame(Game* GameInstance);
	Game* GetOwningGame() const;

	void Initialize() override;
	void Update(float DeltaTime) override;
	void Shutdown() override;

	void AddComponent(std::unique_ptr<ActorComponent> Component);
	const std::vector<std::unique_ptr<ActorComponent>>& GetComponents() const;

	void SetPosition(const DirectX::XMFLOAT3& NewPosition);
	const DirectX::XMFLOAT3& GetPosition() const;

private:
	Game* OwningGame;
	std::vector<std::unique_ptr<ActorComponent>> Components;
	DirectX::XMFLOAT3 Position;
};
