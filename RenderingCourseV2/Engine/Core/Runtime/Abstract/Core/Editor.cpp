#include "Engine/Core/Runtime/Abstract/Core/Editor.h"
#include "Engine/Core/Runtime/Abstract/Subsystems/AssetManager.h"
#include <iostream>

Editor& Editor::Get()
{
	static Editor EditorInstance;
	return EditorInstance;
}

Editor::Editor()
	: EditorName(L"Editor")
	, ProjectDirectory(L"")
	, IsInitialized(false)
{
}

Editor::~Editor()
{
	Shutdown();
}

void Editor::Configure(const std::wstring& InEditorName, const std::wstring& InProjectDirectory)
{
	EditorName = InEditorName;
	ProjectDirectory = InProjectDirectory;
}

void Editor::Initialize()
{
	if (IsInitialized)
	{
		return;
	}

	if (GetSubsystem<AssetManager>() == nullptr)
	{
		std::unique_ptr<AssetManager> NewAssetManager = std::make_unique<AssetManager>();
		NewAssetManager->SetAssetRootDirectory(ProjectDirectory);
		AddSubsystem(std::move(NewAssetManager));
	}

	for (std::unique_ptr<Subsystem>& ExistingSubsystem : Subsystems)
	{
		if (ExistingSubsystem->GetIsInitialized() == false)
		{
			ExistingSubsystem->Initialize();
		}
	}

	IsInitialized = true;
}

void Editor::Update(float DeltaTime)
{
	for (std::unique_ptr<Subsystem>& ExistingSubsystem : Subsystems)
	{
		ExistingSubsystem->Update(DeltaTime);
	}
}

void Editor::Shutdown()
{
	if (IsInitialized == false && Subsystems.empty())
	{
		return;
	}

	for (std::unique_ptr<Subsystem>& ExistingSubsystem : Subsystems)
	{
		ExistingSubsystem->Shutdown();
	}

	Subsystems.clear();
	IsInitialized = false;
}

const std::wstring& Editor::GetEditorName() const
{
	return EditorName;
}

const std::wstring& Editor::GetProjectDirectory() const
{
	return ProjectDirectory;
}

void Editor::AddSubsystem(std::unique_ptr<Subsystem> NewSubsystem)
{
	if (NewSubsystem == nullptr)
	{
		return;
	}

	if (NewSubsystem->GetCategory() != SubsystemCategory::Editor)
	{
		std::cerr << "Only editor subsystems can be added to Editor." << std::endl;
		return;
	}

	NewSubsystem->SetOwningEditor(this);
	Subsystems.push_back(std::move(NewSubsystem));

	if (IsInitialized)
	{
		Subsystems.back()->Initialize();
	}
}
