#include "Engine/Core/Runtime/Abstract/Core/World.h"
#include "Engine/Core/Runtime/Abstract/Components/ActorComponent.h"
#include "Engine/Core/Runtime/Abstract/Components/RenderingComponent.h"
#include "Engine/Core/Runtime/Abstract/Core/Actor.h"
#include "Engine/Core/Runtime/Abstract/Subsystems/SceneViewportSubsystem.h"
#include "Engine/Core/Runtime/Abstract/Subsystems/Subsystem.h"
#include "Engine/Core/Runtime/Abstract/Subsystems/WorldRenderTargetSubsystem.h"
#include <algorithm>
#include <iostream>

World::World(const std::string& InWorldName)
	: WorldName(InWorldName)
	, OwningGame(nullptr)
{
}

World::~World()
{
	Shutdown();
}

void World::SetOwningGame(Game* GameInstance)
{
	OwningGame = GameInstance;
}

Game* World::GetOwningGame() const
{
	return OwningGame;
}

const std::string& World::GetWorldName() const
{
	return WorldName;
}

void World::Initialize()
{
	if (GetIsInitialized())
	{
		return;
	}

	Object::Initialize();

	for (std::unique_ptr<Subsystem>& ExistingSubsystem : Subsystems)
	{
		if (ExistingSubsystem->GetIsInitialized() == false)
		{
			ExistingSubsystem->Initialize();
		}
	}

	for (std::unique_ptr<Actor>& ExistingActor : Actors)
	{
		if (ExistingActor->GetIsInitialized() == false)
		{
			ExistingActor->Initialize();
		}
	}
}

void World::Update(float DeltaTime)
{
	for (std::unique_ptr<Subsystem>& ExistingSubsystem : Subsystems)
	{
		ExistingSubsystem->Update(DeltaTime);
	}

	for (std::unique_ptr<Actor>& ExistingActor : Actors)
	{
		ExistingActor->Update(DeltaTime);
	}
}

void World::Shutdown()
{
	if (GetIsInitialized() == false)
	{
		return;
	}

	for (std::unique_ptr<Actor>& ExistingActor : Actors)
	{
		ExistingActor->Shutdown();
	}

	for (std::unique_ptr<Subsystem>& ExistingSubsystem : Subsystems)
	{
		ExistingSubsystem->Shutdown();
	}

	Object::Shutdown();
}

void World::Draw(SceneViewportSubsystem* SceneViewport, float TotalTimeSeconds)
{
	if (SceneViewport == nullptr)
	{
		return;
	}

	WorldRenderTargetSubsystem* WorldRenderTarget = GetSubsystem<WorldRenderTargetSubsystem>();
	if (WorldRenderTarget != nullptr)
	{
		WorldRenderTarget->BeginRenderPass(SceneViewport, TotalTimeSeconds);
	}
	else
	{
		SceneViewport->ActivateWindowRenderTarget();
	}

	std::vector<RenderingComponent*> RenderingComponents;
	for (std::unique_ptr<Actor>& ExistingActor : Actors)
	{
		const std::vector<std::unique_ptr<ActorComponent>>& ActorComponents = ExistingActor->GetComponents();
		for (const std::unique_ptr<ActorComponent>& ExistingComponent : ActorComponents)
		{
			RenderingComponent* Rendering = dynamic_cast<RenderingComponent*>(ExistingComponent.get());
			if (Rendering != nullptr && Rendering->GetIsActive())
			{
				RenderingComponents.push_back(Rendering);
			}
		}
	}

	std::stable_sort(
		RenderingComponents.begin(),
		RenderingComponents.end(),
		[](const RenderingComponent* LeftRenderingComponent, const RenderingComponent* RightRenderingComponent)
		{
			return LeftRenderingComponent->GetRenderOrder() < RightRenderingComponent->GetRenderOrder();
		});

	for (RenderingComponent* ExistingRenderingComponent : RenderingComponents)
	{
		ExistingRenderingComponent->Render(SceneViewport);
	}

	if (WorldRenderTarget != nullptr)
	{
		WorldRenderTarget->EndRenderPass(SceneViewport);
	}
}

void World::AddSubsystem(std::unique_ptr<Subsystem> NewSubsystem)
{
	if (NewSubsystem == nullptr)
	{
		return;
	}

	if (NewSubsystem->GetCategory() != SubsystemCategory::World)
	{
		std::cerr << "Only world subsystems can be added to World." << std::endl;
		return;
	}

	NewSubsystem->SetOwningWorld(this);
	Subsystems.push_back(std::move(NewSubsystem));
	if (GetIsInitialized())
	{
		Subsystems.back()->Initialize();
	}
}

void World::AddActor(std::unique_ptr<Actor> NewActor)
{
	if (NewActor == nullptr)
	{
		return;
	}

	NewActor->SetOwningGame(OwningGame);
	Actors.push_back(std::move(NewActor));
	if (GetIsInitialized())
	{
		Actors.back()->Initialize();
	}
}

const std::vector<std::unique_ptr<Subsystem>>& World::GetSubsystems() const
{
	return Subsystems;
}

const std::vector<std::unique_ptr<Actor>>& World::GetActors() const
{
	return Actors;
}
