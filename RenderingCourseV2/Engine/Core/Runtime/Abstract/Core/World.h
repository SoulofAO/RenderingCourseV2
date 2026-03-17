#pragma once

#include "Engine/Core/Runtime/Abstract/Core/Object.h"
#include <memory>
#include <string>
#include <vector>

class Actor;
class Game;
class SceneViewportSubsystem;
class Subsystem;

class World : public Object
{
public:
	explicit World(const std::string& InWorldName);
	~World() override;

	void SetOwningGame(Game* GameInstance);
	Game* GetOwningGame() const;
	const std::string& GetWorldName() const;

	void Initialize() override;
	void Update(float DeltaTime) override;
	void Shutdown() override;

	void Draw(SceneViewportSubsystem* SceneViewport, float TotalTimeSeconds);

	void AddSubsystem(std::unique_ptr<Subsystem> NewSubsystem);
	void AddActor(std::unique_ptr<Actor> NewActor);

	template<typename TSubsystem>
	TSubsystem* GetSubsystem() const
	{
		for (const std::unique_ptr<Subsystem>& ExistingSubsystem : Subsystems)
		{
			TSubsystem* TypedSubsystem = dynamic_cast<TSubsystem*>(ExistingSubsystem.get());
			if (TypedSubsystem != nullptr)
			{
				return TypedSubsystem;
			}
		}

		return nullptr;
	}

	const std::vector<std::unique_ptr<Subsystem>>& GetSubsystems() const;
	const std::vector<std::unique_ptr<Actor>>& GetActors() const;

private:
	std::string WorldName;
	Game* OwningGame;
	std::vector<std::unique_ptr<Subsystem>> Subsystems;
	std::vector<std::unique_ptr<Actor>> Actors;
};
