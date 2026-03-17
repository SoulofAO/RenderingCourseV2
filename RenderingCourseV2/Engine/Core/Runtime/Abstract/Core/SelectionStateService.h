#pragma once

#include "Engine/Core/Runtime/Abstract/Core/Object.h"

enum class SelectionSource : unsigned char
{
	None,
	Scene,
	ContentFolder
};

enum class SelectionStatus : unsigned char
{
	None,
	Hovered,
	Selected,
	EditingTransform
};

class SelectionStateService : public UObject
{
public:
	SelectionStateService();
	~SelectionStateService() override;

	void SetSelectedObject(UObject* NewSelectedObject, SelectionSource NewSelectionSource);
	void SetHoveredObject(UObject* NewHoveredObject);
	void ClearSelection();
	void BeginTransformEditing();
	void EndTransformEditing();

	UObject* GetSelectedObject() const;
	UObject* GetHoveredObject() const;
	SelectionSource GetSelectionSource() const;
	SelectionStatus GetSelectionStatus() const;

private:
	UObject* SelectedObject;
	UObject* HoveredObject;
	SelectionSource CurrentSelectionSource;
	SelectionStatus CurrentSelectionStatus;
};
