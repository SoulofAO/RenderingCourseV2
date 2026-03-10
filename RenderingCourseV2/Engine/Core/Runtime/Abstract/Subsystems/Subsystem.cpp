#include "Engine/Core/Runtime/Abstract/Subsystems/Subsystem.h"

Subsystem::Subsystem(SubsystemCategory InCategory)
	: Category(InCategory)
	, OwningGame(nullptr)
	, OwningEditor(nullptr)
{
}

Subsystem::~Subsystem() = default;

void Subsystem::SetOwningGame(Game* GameInstance)
{
	OwningGame = GameInstance;
	OwningEditor = nullptr;
}

Game* Subsystem::GetOwningGame() const
{
	return OwningGame;
}

void Subsystem::SetOwningEditor(Editor* EditorInstance)
{
	OwningEditor = EditorInstance;
	OwningGame = nullptr;
}

Editor* Subsystem::GetOwningEditor() const
{
	return OwningEditor;
}

SubsystemCategory Subsystem::GetCategory() const
{
	return Category;
}

