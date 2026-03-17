#include "Engine/Core/Runtime/Abstract/Subsystems/Subsystem.h"

Subsystem::Subsystem(SubsystemCategory InCategory)
	: Category(InCategory)
	, OwningGame(nullptr)
	, OwningEditor(nullptr)
	, OwningWorld(nullptr)
{
}

Subsystem::~Subsystem() = default;

void Subsystem::SetOwningGame(Game* GameInstance)
{
	OwningGame = GameInstance;
	OwningEditor = nullptr;
	OwningWorld = nullptr;
}

Game* Subsystem::GetOwningGame() const
{
	return OwningGame;
}

void Subsystem::SetOwningEditor(Editor* EditorInstance)
{
	OwningEditor = EditorInstance;
	OwningGame = nullptr;
	OwningWorld = nullptr;
}

Editor* Subsystem::GetOwningEditor() const
{
	return OwningEditor;
}

void Subsystem::SetOwningWorld(World* WorldInstance)
{
	OwningWorld = WorldInstance;
	OwningGame = nullptr;
	OwningEditor = nullptr;
}

World* Subsystem::GetOwningWorld() const
{
	return OwningWorld;
}

SubsystemCategory Subsystem::GetCategory() const
{
	return Category;
}

