#include "Engine/Core/Runtime/Abstract/Core/SelectionStateService.h"

SelectionStateService::SelectionStateService()
	: SelectedObject(nullptr)
	, HoveredObject(nullptr)
	, CurrentSelectionSource(SelectionSource::None)
	, CurrentSelectionStatus(SelectionStatus::None)
{
}

SelectionStateService::~SelectionStateService() = default;

void SelectionStateService::SetSelectedObject(UObject* NewSelectedObject, SelectionSource NewSelectionSource)
{
	SelectedObject = NewSelectedObject;
	CurrentSelectionSource = NewSelectionSource;
	if (SelectedObject == nullptr)
	{
		CurrentSelectionStatus = SelectionStatus::None;
	}
	else
	{
		CurrentSelectionStatus = SelectionStatus::Selected;
	}
}

void SelectionStateService::SetHoveredObject(UObject* NewHoveredObject)
{
	HoveredObject = NewHoveredObject;
	if (CurrentSelectionStatus != SelectionStatus::EditingTransform && HoveredObject != nullptr)
	{
		CurrentSelectionStatus = SelectionStatus::Hovered;
	}
}

void SelectionStateService::ClearSelection()
{
	SelectedObject = nullptr;
	HoveredObject = nullptr;
	CurrentSelectionSource = SelectionSource::None;
	CurrentSelectionStatus = SelectionStatus::None;
}

void SelectionStateService::BeginTransformEditing()
{
	if (SelectedObject != nullptr)
	{
		CurrentSelectionStatus = SelectionStatus::EditingTransform;
	}
}

void SelectionStateService::EndTransformEditing()
{
	if (SelectedObject != nullptr)
	{
		CurrentSelectionStatus = SelectionStatus::Selected;
	}
	else
	{
		CurrentSelectionStatus = SelectionStatus::None;
	}
}

UObject* SelectionStateService::GetSelectedObject() const
{
	return SelectedObject;
}

UObject* SelectionStateService::GetHoveredObject() const
{
	return HoveredObject;
}

SelectionSource SelectionStateService::GetSelectionSource() const
{
	return CurrentSelectionSource;
}

SelectionStatus SelectionStateService::GetSelectionStatus() const
{
	return CurrentSelectionStatus;
}
