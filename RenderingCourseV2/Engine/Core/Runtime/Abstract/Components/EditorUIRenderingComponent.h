#pragma once

#include "Engine/Core/Runtime/Abstract/Components/UIRenderingComponent.h"
#include <directxmath.h>
#include "imgui.h"

class Actor;
class SelectionStateService;

class EditorUIRenderingComponent : public UIRenderingComponent
{
public:
	EditorUIRenderingComponent();
	~EditorUIRenderingComponent() override;

protected:
	void RenderUI() override;

private:
	void RenderDockSpace();
	void RenderSceneViewPanel();
	void RenderContentFolderPanel();
	void RenderSceneInspectorPanel();
	void RenderDetailsPanel();
	void RenderTranslateGizmo(Actor* SelectedActor);
	void HandleSceneSelection();
	ImVec2 ProjectWorldToSceneView(const DirectX::XMFLOAT3& WorldPosition) const;
	SelectionStateService* GetSelectionStateService() const;

	ImVec2 SceneViewOrigin;
	ImVec2 SceneViewSize;
	int ActiveGizmoAxis;
	bool IsDockLayoutInitialized;
};
