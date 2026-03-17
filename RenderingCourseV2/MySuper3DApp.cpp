#include "Engine/Core/Runtime/Abstract/Components/EditorUIRenderingComponent.h"
#include "Engine/Core/Runtime/Abstract/Core/Actor.h"
#include "Engine/Core/Runtime/Abstract/Core/Editor.h"
#include "Engine/Core/Runtime/Abstract/Core/Game.h"
#include "Engine/Core/Runtime/Abstract/Core/RuntimeObjectSystem.h"
#include "Engine/Core/Runtime/Abstract/Core/World.h"
#include "Engine/Core/Runtime/Abstract/Subsystems/AssetManager.h"
#include "Engine/Core/Runtime/Abstract/Subsystems/WorldRenderTargetSubsystem.h"

class EditorRuntimeGame : public Game
{
public:
	EditorRuntimeGame(LPCWSTR ApplicationName, int ScreenWidth, int ScreenHeight)
		: Game(ApplicationName, ScreenWidth, ScreenHeight)
	{
	}

	~EditorRuntimeGame() override = default;

protected:
	void BeginPlay() override
	{
		World* EditorViewportWorld = GetEditorViewportWorld();
		World* EditorUserInterfaceWorld = GetEditorWorld();
		if (EditorViewportWorld == nullptr || EditorUserInterfaceWorld == nullptr)
		{
			return;
		}

		if (EditorViewportWorld->GetSubsystem<WorldRenderTargetSubsystem>() == nullptr)
		{
			std::unique_ptr<WorldRenderTargetSubsystem> NewWorldRenderTargetSubsystem = std::make_unique<WorldRenderTargetSubsystem>();
			EditorViewportWorld->AddSubsystem(std::move(NewWorldRenderTargetSubsystem));
		}

		std::unique_ptr<Actor> SceneActor = std::make_unique<Actor>();
		SceneActor->SetPosition(DirectX::XMFLOAT3(-2.0f, 1.0f, 0.0f));
		EditorViewportWorld->AddActor(std::move(SceneActor));

		std::unique_ptr<Actor> SceneActorSecond = std::make_unique<Actor>();
		SceneActorSecond->SetPosition(DirectX::XMFLOAT3(1.5f, -0.5f, 0.0f));
		EditorViewportWorld->AddActor(std::move(SceneActorSecond));

		std::unique_ptr<Actor> UserInterfaceActor = std::make_unique<Actor>();
		std::unique_ptr<EditorUIRenderingComponent> EditorUserInterfaceRenderingComponent = std::make_unique<EditorUIRenderingComponent>();
		UserInterfaceActor->AddComponent(std::move(EditorUserInterfaceRenderingComponent));
		EditorUserInterfaceWorld->AddActor(std::move(UserInterfaceActor));
	}

	void Update(float DeltaTime) override
	{
		Game::Update(DeltaTime);
		Editor::Get().Update(DeltaTime);
	}
};

int main()
{
	Editor& EditorInstance = Editor::Get();
	EditorInstance.Configure(L"RenderingCourseEditor", L"G:/RenderingCourseV2/RenderingCourseV2");
	EditorInstance.Initialize();

	AssetManager* AssetManagerSubsystem = EditorInstance.GetSubsystem<AssetManager>();
	if (AssetManagerSubsystem != nullptr)
	{
		AssetManagerSubsystem->RegisterAsset(L"Mesh/Cube", L"Assets/Meshes/Cube.mesh");
		AssetManagerSubsystem->RegisterAsset(L"Material/Default", L"Assets/Materials/Default.mat");
		AssetManagerSubsystem->RegisterAsset(L"Texture/Checker", L"Assets/Textures/Checker.png");
	}

	RuntimeObjectSystem& RuntimeSystem = RuntimeObjectSystem::Get();
	Actor::RegisterActorClass(RuntimeSystem);

	EditorRuntimeGame EditorGame(L"RenderingCourseV2 Editor", 1400, 900);
	EditorGame.Initialize();
	EditorGame.Run();

	EditorInstance.Shutdown();
	return 0;
}
