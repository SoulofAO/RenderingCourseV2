#pragma once

#include "Engine/Core/Runtime/Abstract/Subsystems/Subsystem.h"
#include <memory>
#include <string>
#include <vector>

class AssetManager;
class SelectionStateService;

class Editor
{
public:
	static Editor& Get();

	Editor(const Editor&) = delete;
	Editor& operator=(const Editor&) = delete;

	void Configure(const std::wstring& InEditorName, const std::wstring& InProjectDirectory);
	void Initialize();
	void Update(float DeltaTime);
	void Shutdown();

	const std::wstring& GetEditorName() const;
	const std::wstring& GetProjectDirectory() const;
	SelectionStateService* GetSelectionStateService() const;

	void AddSubsystem(std::unique_ptr<Subsystem> NewSubsystem);

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

private:
	Editor();
	~Editor();

	std::wstring EditorName;
	std::wstring ProjectDirectory;
	std::vector<std::unique_ptr<Subsystem>> Subsystems;
	std::unique_ptr<SelectionStateService> SelectionState;
	bool IsInitialized;
};
