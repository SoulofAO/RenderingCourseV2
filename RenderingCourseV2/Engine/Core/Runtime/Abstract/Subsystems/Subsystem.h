#pragma once

#include "Engine/Core/Runtime/Abstract/Core/Object.h"

class Game;
class Editor;
class World;

enum class SubsystemCategory
{
	Game,
	Editor,
	World,
	Custom
};

class Subsystem : public Object
{
public:
	explicit Subsystem(SubsystemCategory InCategory = SubsystemCategory::Game);
	~Subsystem() override;

	void SetOwningGame(Game* GameInstance);
	Game* GetOwningGame() const;
	void SetOwningEditor(Editor* EditorInstance);
	Editor* GetOwningEditor() const;
	void SetOwningWorld(World* WorldInstance);
	World* GetOwningWorld() const;
	SubsystemCategory GetCategory() const;

protected:
	SubsystemCategory Category;
	Game* OwningGame;
	Editor* OwningEditor;
	World* OwningWorld;
};

