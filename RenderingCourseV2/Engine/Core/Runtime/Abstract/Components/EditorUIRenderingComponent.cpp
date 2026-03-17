#include "Engine/Core/Runtime/Abstract/Components/EditorUIRenderingComponent.h"
#include "Engine/Core/Runtime/Abstract/Core/Actor.h"
#include "Engine/Core/Runtime/Abstract/Core/Editor.h"
#include "Engine/Core/Runtime/Abstract/Core/Game.h"
#include "Engine/Core/Runtime/Abstract/Core/RuntimeObjectSystem.h"
#include "Engine/Core/Runtime/Abstract/Core/SelectionStateService.h"
#include "Engine/Core/Runtime/Abstract/Core/World.h"
#include "Engine/Core/Runtime/Abstract/Subsystems/AssetManager.h"
#include "Engine/Core/Runtime/Abstract/Subsystems/SceneViewportSubsystem.h"
#include "Engine/Core/Runtime/Abstract/Subsystems/WorldRenderTargetSubsystem.h"
#include "imgui_internal.h"
#include <algorithm>
#include <string>

EditorUIRenderingComponent::EditorUIRenderingComponent()
	: UIRenderingComponent()
	, SceneViewOrigin(0.0f, 0.0f)
	, SceneViewSize(0.0f, 0.0f)
	, ActiveGizmoAxis(-1)
	, IsDockLayoutInitialized(false)
{
}

EditorUIRenderingComponent::~EditorUIRenderingComponent() = default;

void EditorUIRenderingComponent::RenderUI()
{
	RenderDockSpace();
	RenderSceneViewPanel();
	RenderContentFolderPanel();
	RenderSceneInspectorPanel();
	RenderDetailsPanel();
}

void EditorUIRenderingComponent::RenderDockSpace()
{
	ImGuiViewport* MainViewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(MainViewport->WorkPos);
	ImGui::SetNextWindowSize(MainViewport->WorkSize);
	ImGui::SetNextWindowViewport(MainViewport->ID);

	ImGuiWindowFlags RootWindowFlags = ImGuiWindowFlags_NoTitleBar
		| ImGuiWindowFlags_NoCollapse
		| ImGuiWindowFlags_NoResize
		| ImGuiWindowFlags_NoMove
		| ImGuiWindowFlags_NoBringToFrontOnFocus
		| ImGuiWindowFlags_NoNavFocus
		| ImGuiWindowFlags_NoBackground
		| ImGuiWindowFlags_NoDocking;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::Begin("EditorDockRootWindow", nullptr, RootWindowFlags);
	ImGui::PopStyleVar();

	ImGuiID DockSpaceIdentifier = ImGui::GetID("EditorDockSpaceIdentifier");
	ImGui::DockSpace(DockSpaceIdentifier, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);

	if (IsDockLayoutInitialized == false)
	{
		ImGui::DockBuilderRemoveNode(DockSpaceIdentifier);
		ImGui::DockBuilderAddNode(DockSpaceIdentifier, ImGuiDockNodeFlags_DockSpace);
		ImGui::DockBuilderSetNodeSize(DockSpaceIdentifier, MainViewport->WorkSize);

		ImGuiID RootDockIdentifier = DockSpaceIdentifier;
		ImGuiID RightDockIdentifier = ImGui::DockBuilderSplitNode(RootDockIdentifier, ImGuiDir_Right, 0.26f, nullptr, &RootDockIdentifier);
		ImGuiID BottomDockIdentifier = ImGui::DockBuilderSplitNode(RootDockIdentifier, ImGuiDir_Down, 0.30f, nullptr, &RootDockIdentifier);
		ImGuiID ContentDockIdentifier = ImGui::DockBuilderSplitNode(BottomDockIdentifier, ImGuiDir_Left, 0.42f, nullptr, &BottomDockIdentifier);

		ImGui::DockBuilderDockWindow("Scene View", RootDockIdentifier);
		ImGui::DockBuilderDockWindow("SceneInspector", RightDockIdentifier);
		ImGui::DockBuilderDockWindow("ContentFolder", ContentDockIdentifier);
		ImGui::DockBuilderDockWindow("Details", BottomDockIdentifier);
		ImGui::DockBuilderFinish(DockSpaceIdentifier);
		IsDockLayoutInitialized = true;
	}

	ImGui::End();
}

void EditorUIRenderingComponent::RenderSceneViewPanel()
{
	ImGui::Begin("Scene View");
	SceneViewOrigin = ImGui::GetCursorScreenPos();
	SceneViewSize = ImGui::GetContentRegionAvail();
	if (SceneViewSize.x < 1.0f)
	{
		SceneViewSize.x = 1.0f;
	}

	if (SceneViewSize.y < 1.0f)
	{
		SceneViewSize.y = 1.0f;
	}

	SceneViewportSubsystem* SceneViewportSubsystemInstance = nullptr;
	Game* OwningGame = GetOwningGame();
	if (OwningGame != nullptr)
	{
		SceneViewportSubsystemInstance = OwningGame->GetSubsystem<SceneViewportSubsystem>();
	}

	ID3D11ShaderResourceView* SceneViewShaderResourceView = nullptr;
	if (SceneViewportSubsystemInstance != nullptr && OwningGame != nullptr && OwningGame->GetEditorViewportWorld() != nullptr)
	{
		WorldRenderTargetSubsystem* WorldRenderTarget = OwningGame->GetEditorViewportWorld()->GetSubsystem<WorldRenderTargetSubsystem>();
		if (WorldRenderTarget != nullptr)
		{
			SceneViewShaderResourceView = WorldRenderTarget->GetShaderResourceView();
		}
	}

	if (SceneViewShaderResourceView != nullptr)
	{
		ImGui::Image(reinterpret_cast<ImTextureID>(SceneViewShaderResourceView), SceneViewSize);
	}
	else
	{
		ImGui::InvisibleButton("SceneViewFallbackButton", SceneViewSize);
	}

	HandleSceneSelection();

	SelectionStateService* SelectionStateServiceInstance = GetSelectionStateService();
	if (SelectionStateServiceInstance != nullptr)
	{
		Actor* SelectedActor = dynamic_cast<Actor*>(SelectionStateServiceInstance->GetSelectedObject());
		if (SelectedActor != nullptr)
		{
			RenderTranslateGizmo(SelectedActor);
		}
	}

	ImGui::End();
}

void EditorUIRenderingComponent::RenderContentFolderPanel()
{
	ImGui::Begin("ContentFolder");
	Editor& EditorInstance = Editor::Get();
	AssetManager* AssetManagerSubsystem = EditorInstance.GetSubsystem<AssetManager>();
	if (AssetManagerSubsystem == nullptr)
	{
		ImGui::Text("AssetManager is not available.");
		ImGui::End();
		return;
	}

	const std::wstring& AssetRootDirectory = AssetManagerSubsystem->GetAssetRootDirectory();
	ImGui::Text("Root: %ls", AssetRootDirectory.c_str());
	ImGui::Separator();

	const std::vector<AssetRegistryEntry> AssetEntries = AssetManagerSubsystem->GetRegisteredAssets();
	for (const AssetRegistryEntry& ExistingAssetEntry : AssetEntries)
	{
		ImGui::BulletText("%ls -> %ls", ExistingAssetEntry.AssetIdentifier.c_str(), ExistingAssetEntry.RelativePath.c_str());
	}

	ImGui::End();
}

void EditorUIRenderingComponent::RenderSceneInspectorPanel()
{
	ImGui::Begin("SceneInspector");
	Game* OwningGame = GetOwningGame();
	SelectionStateService* SelectionStateServiceInstance = GetSelectionStateService();
	if (OwningGame == nullptr || SelectionStateServiceInstance == nullptr || OwningGame->GetEditorViewportWorld() == nullptr)
	{
		ImGui::Text("Scene is not available.");
		ImGui::End();
		return;
	}

	const std::vector<std::unique_ptr<Actor>>& ExistingActors = OwningGame->GetEditorViewportWorld()->GetActors();
	for (size_t ActorIndex = 0; ActorIndex < ExistingActors.size(); ++ActorIndex)
	{
		Actor* ExistingActor = ExistingActors[ActorIndex].get();
		bool IsSelected = SelectionStateServiceInstance->GetSelectedObject() == ExistingActor;
		std::string ActorLabel = "Actor " + std::to_string(ActorIndex);
		if (ImGui::Selectable(ActorLabel.c_str(), IsSelected))
		{
			SelectionStateServiceInstance->SetSelectedObject(ExistingActor, SelectionSource::Scene);
		}
	}

	ImGui::End();
}

void EditorUIRenderingComponent::RenderDetailsPanel()
{
	ImGui::Begin("Details");
	SelectionStateService* SelectionStateServiceInstance = GetSelectionStateService();
	if (SelectionStateServiceInstance == nullptr)
	{
		ImGui::Text("Selection service is not available.");
		ImGui::End();
		return;
	}

	UObject* SelectedObject = SelectionStateServiceInstance->GetSelectedObject();
	if (SelectedObject == nullptr)
	{
		ImGui::Text("No object selected.");
		ImGui::End();
		return;
	}

	UClass* ObjectClass = RuntimeObjectSystem::Get().FindClass(SelectedObject->GetRuntimeClassName());
	if (ObjectClass == nullptr)
	{
		ImGui::Text("Class is not registered: %s", SelectedObject->GetRuntimeClassName());
		ImGui::End();
		return;
	}

	const std::vector<UPropertyDescriptor>& PropertyDescriptors = ObjectClass->GetPropertyDescriptors();
	unsigned char* ObjectMemoryAddress = reinterpret_cast<unsigned char*>(SelectedObject);
	for (const UPropertyDescriptor& ExistingPropertyDescriptor : PropertyDescriptors)
	{
		if ((ExistingPropertyDescriptor.PropertyFlags & PropertyFlagEditableInDetails) == 0)
		{
			continue;
		}

		unsigned char* PropertyMemoryAddress = ObjectMemoryAddress + ExistingPropertyDescriptor.PropertyOffset;
		if (ExistingPropertyDescriptor.PropertyType == UPropertyType::Float3)
		{
			DirectX::XMFLOAT3* Float3Value = reinterpret_cast<DirectX::XMFLOAT3*>(PropertyMemoryAddress);
			float PropertyBuffer[3] = { Float3Value->x, Float3Value->y, Float3Value->z };
			if (ImGui::DragFloat3(ExistingPropertyDescriptor.PropertyName, PropertyBuffer, 0.05f))
			{
				Float3Value->x = PropertyBuffer[0];
				Float3Value->y = PropertyBuffer[1];
				Float3Value->z = PropertyBuffer[2];
			}
		}
	}

	ImGui::End();
}

void EditorUIRenderingComponent::RenderTranslateGizmo(Actor* SelectedActor)
{
	if (SelectedActor == nullptr)
	{
		return;
	}

	ImDrawList* DrawList = ImGui::GetWindowDrawList();
	DirectX::XMFLOAT3 ActorPosition = SelectedActor->GetPosition();
	ImVec2 ActorScreenPosition = ProjectWorldToSceneView(ActorPosition);

	float GizmoAxisLength = 80.0f;
	ImVec2 AxisXEndPoint(ActorScreenPosition.x + GizmoAxisLength, ActorScreenPosition.y);
	ImVec2 AxisYEndPoint(ActorScreenPosition.x, ActorScreenPosition.y - GizmoAxisLength);
	ImVec2 AxisZEndPoint(ActorScreenPosition.x + GizmoAxisLength * 0.5f, ActorScreenPosition.y + GizmoAxisLength * 0.5f);

	DrawList->AddLine(ActorScreenPosition, AxisXEndPoint, IM_COL32(255, 70, 70, 255), 2.0f);
	DrawList->AddLine(ActorScreenPosition, AxisYEndPoint, IM_COL32(70, 255, 70, 255), 2.0f);
	DrawList->AddLine(ActorScreenPosition, AxisZEndPoint, IM_COL32(70, 70, 255, 255), 2.0f);

	ImGui::SetCursorScreenPos(ImVec2(AxisXEndPoint.x - 10.0f, AxisXEndPoint.y - 10.0f));
	ImGui::InvisibleButton("GizmoAxisXButton", ImVec2(20.0f, 20.0f));
	if (ImGui::IsItemActive())
	{
		ActiveGizmoAxis = 0;
	}

	ImGui::SetCursorScreenPos(ImVec2(AxisYEndPoint.x - 10.0f, AxisYEndPoint.y - 10.0f));
	ImGui::InvisibleButton("GizmoAxisYButton", ImVec2(20.0f, 20.0f));
	if (ImGui::IsItemActive())
	{
		ActiveGizmoAxis = 1;
	}

	ImGui::SetCursorScreenPos(ImVec2(AxisZEndPoint.x - 10.0f, AxisZEndPoint.y - 10.0f));
	ImGui::InvisibleButton("GizmoAxisZButton", ImVec2(20.0f, 20.0f));
	if (ImGui::IsItemActive())
	{
		ActiveGizmoAxis = 2;
	}

	SelectionStateService* SelectionStateServiceInstance = GetSelectionStateService();
	if (ImGui::IsMouseDown(ImGuiMouseButton_Left) && ActiveGizmoAxis != -1)
	{
		if (SelectionStateServiceInstance != nullptr)
		{
			SelectionStateServiceInstance->BeginTransformEditing();
		}

		ImVec2 MouseDelta = ImGui::GetIO().MouseDelta;
		float MoveScale = 0.02f;
		if (ActiveGizmoAxis == 0)
		{
			ActorPosition.x += MouseDelta.x * MoveScale;
		}
		else if (ActiveGizmoAxis == 1)
		{
			ActorPosition.y -= MouseDelta.y * MoveScale;
		}
		else if (ActiveGizmoAxis == 2)
		{
			ActorPosition.z += (MouseDelta.x - MouseDelta.y) * MoveScale;
		}

		SelectedActor->SetPosition(ActorPosition);
	}
	else
	{
		ActiveGizmoAxis = -1;
		if (SelectionStateServiceInstance != nullptr)
		{
			SelectionStateServiceInstance->EndTransformEditing();
		}
	}
}

void EditorUIRenderingComponent::HandleSceneSelection()
{
	SelectionStateService* SelectionStateServiceInstance = GetSelectionStateService();
	Game* OwningGame = GetOwningGame();
	if (SelectionStateServiceInstance == nullptr || OwningGame == nullptr || OwningGame->GetEditorViewportWorld() == nullptr)
	{
		return;
	}

	if (ImGui::IsWindowHovered() == false)
	{
		return;
	}

	if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) == false)
	{
		return;
	}

	const std::vector<std::unique_ptr<Actor>>& ExistingActors = OwningGame->GetEditorViewportWorld()->GetActors();
	Actor* NearestActor = nullptr;
	float NearestDistanceSquared = 0.0f;
	ImVec2 MousePosition = ImGui::GetIO().MousePos;
	for (const std::unique_ptr<Actor>& ExistingActor : ExistingActors)
	{
		ImVec2 ActorScreenPosition = ProjectWorldToSceneView(ExistingActor->GetPosition());
		float PositionDeltaX = MousePosition.x - ActorScreenPosition.x;
		float PositionDeltaY = MousePosition.y - ActorScreenPosition.y;
		float DistanceSquared = PositionDeltaX * PositionDeltaX + PositionDeltaY * PositionDeltaY;
		if (NearestActor == nullptr || DistanceSquared < NearestDistanceSquared)
		{
			NearestActor = ExistingActor.get();
			NearestDistanceSquared = DistanceSquared;
		}
	}

	if (NearestActor != nullptr && NearestDistanceSquared < 1600.0f)
	{
		SelectionStateServiceInstance->SetSelectedObject(NearestActor, SelectionSource::Scene);
	}
}

ImVec2 EditorUIRenderingComponent::ProjectWorldToSceneView(const DirectX::XMFLOAT3& WorldPosition) const
{
	float HorizontalScale = SceneViewSize.x * 0.05f;
	float VerticalScale = SceneViewSize.y * 0.05f;
	float CenterPositionX = SceneViewOrigin.x + SceneViewSize.x * 0.5f;
	float CenterPositionY = SceneViewOrigin.y + SceneViewSize.y * 0.5f;
	return ImVec2(
		CenterPositionX + WorldPosition.x * HorizontalScale,
		CenterPositionY - WorldPosition.y * VerticalScale);
}

SelectionStateService* EditorUIRenderingComponent::GetSelectionStateService() const
{
	return Editor::Get().GetSelectionStateService();
}
