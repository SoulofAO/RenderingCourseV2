#include "Abstracts/Core/Game.h"
#include "Abstracts/Subsystems/InputDevice.h"
#include "Abstracts/Subsystems/Subsystem.h"
#include "Abstracts/Core/Actor.h"
#include "Abstracts/Input/EngineHotkeyInputHandler.h"
#include "Abstracts/Input/FreeCameraInputHandler.h"
#include "Abstracts/Input/GameInputHandler.h"
#include "Abstracts/Components/RenderingComponent.h"
#include "Abstracts/Components/LightComponent.h"
#include "Abstracts/Components/CameraComponent.h"
#include "Abstracts/Subsystems/SceneViewportSubsystem.h"
#include "Abstracts/Subsystems/PhysicsSubsystem.h"
#include "Abstracts/Subsystems/CameraSubsystem.h"
#include "Abstracts/Subsystems/DisplayWin32.h"
#include "Abstracts/Rendering/DeferredRenderer.h"
#include "Abstracts/Resources/ResourceManager.h"
#include <filesystem>
#include <directxmath.h>
#include <algorithm>
#include <iostream>

Game::Game(LPCWSTR ApplicationName, int ScreenWidth, int ScreenHeight)
	: Name(ApplicationName)
	, ScreenWidth(ScreenWidth)
	, ScreenHeight(ScreenHeight)
	, TotalRunTimeSeconds(0.0f)
	, FrameCount(0)
	, IsExitRequested(false)
	, FallbackCameraActor(nullptr)
	, FallbackCameraComponent(nullptr)
{
}

/**
 * 
 */
Game::~Game()
{
	for (std::unique_ptr<Actor>& ExistingActor : Actors)
	{
		ExistingActor->Shutdown();
	}

	for (std::unique_ptr<Subsystem>& ExistingSubsystem : Subsystems)
	{
		ExistingSubsystem->Shutdown();
	}

	InputHandlers.clear();
}

void Game::Initialize()
{
	Input = std::make_unique<InputDevice>(this);
	Resources = std::make_unique<ResourceManager>();

	namespace FileSystem = std::filesystem;
	FileSystem::path CurrentPath = FileSystem::current_path();
	FileSystem::path SearchPath = CurrentPath;
	while (!SearchPath.empty() && !FileSystem::exists(SearchPath / "RenderingCourseV2.vcxproj"))
	{
		SearchPath = SearchPath.parent_path();
	}
	if (SearchPath.empty())
	{
		SearchPath = CurrentPath;
	}

	const FileSystem::path CookedPath = SearchPath / "Cooked";
	Resources->Initialize(SearchPath.string(), CookedPath.string());

	if (GetSubsystem<SceneViewportSubsystem>() == nullptr)
	{
		std::unique_ptr<SceneViewportSubsystem> NewSceneViewport = std::make_unique<SceneViewportSubsystem>();
		AddSubsystem(std::move(NewSceneViewport));
	}

	if (GetSubsystem<PhysicsSubsystem>() == nullptr)
	{
		std::unique_ptr<PhysicsSubsystem> NewPhysicsSubsystem = std::make_unique<PhysicsSubsystem>();
		AddSubsystem(std::move(NewPhysicsSubsystem));
	}

	if (GetSubsystem<CameraSubsystem>() == nullptr)
	{
		std::unique_ptr<CameraSubsystem> NewCameraSubsystem = std::make_unique<CameraSubsystem>();
		AddSubsystem(std::move(NewCameraSubsystem));
	}

	RegisterInputHandler(std::make_unique<EngineHotkeyInputHandler>());
	RegisterInputHandler(std::make_unique<FreeCameraInputHandler>());

	for (std::unique_ptr<Subsystem>& ExistingSubsystem : Subsystems)
	{
		ExistingSubsystem->Initialize();
	}

	for (std::unique_ptr<Actor>& ExistingActor : Actors)
	{
		ExistingActor->Initialize();
	}
}

void Game::Run()
{
	StartTime = std::chrono::steady_clock::now();
	PreviousTime = StartTime;
	BeginPlay();

	MSG Message = {};
	while (!IsExitRequested)
	{
		while (PeekMessage(&Message, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&Message);
			DispatchMessage(&Message);
		}

		if (Message.message == WM_QUIT)
		{
			IsExitRequested = true;
		}

		auto CurrentTime = std::chrono::steady_clock::now();
		float DeltaTime = std::chrono::duration_cast<std::chrono::microseconds>(CurrentTime - PreviousTime).count() / 1000000.0f;
		PreviousTime = CurrentTime;

		TotalRunTimeSeconds += DeltaTime;
		FrameCount++;

		SceneViewportSubsystem* SceneViewport = GetSubsystem<SceneViewportSubsystem>();
		if (SceneViewport != nullptr && SceneViewport->GetDisplay() != nullptr && TotalRunTimeSeconds > 0.0f)
		{
			static float FramesSecondAccumulator = 0.0f;
			FramesSecondAccumulator += DeltaTime;
			if (FramesSecondAccumulator > 1.0f)
			{
				float FramesPerSecond = static_cast<float>(FrameCount) / FramesSecondAccumulator;
				FramesSecondAccumulator = 0.0f;
				FrameCount = 0;

				WCHAR TitleText[256];
				swprintf_s(TitleText, TEXT("FPS: %f"), FramesPerSecond);
				SetWindowText(SceneViewport->GetDisplay()->GetWindowHandle(), TitleText);
			}
		}

		Update(DeltaTime);
		Draw();
	}
}

void Game::Update(float DeltaTime)
{
	if (Input != nullptr)
	{
		for (std::unique_ptr<GameInputHandler>& ExistingInputHandler : InputHandlers)
		{
			if (ExistingInputHandler)
			{
				ExistingInputHandler->HandleInput(this, Input.get(), DeltaTime);
			}
		}
	}

	for (std::unique_ptr<Actor>& ExistingActor : Actors)
	{
		ExistingActor->Update(DeltaTime);
	}

	for (std::unique_ptr<Subsystem>& ExistingSubsystem : Subsystems)
	{
		ExistingSubsystem->Update(DeltaTime);
	}

	if (Input)
	{
		Input->EndFrame();
	}
}

void Game::Draw()
{
	SceneViewportSubsystem* SceneViewport = GetSubsystem<SceneViewportSubsystem>();
	if (SceneViewport == nullptr)
	{
		return;
	}

	CameraSubsystem* CameraSystem = GetSubsystem<CameraSubsystem>();
	const float ScreenWidth = static_cast<float>(GetScreenWidth());
	const float ScreenHeight = static_cast<float>(GetScreenHeight());
	float AspectRatio = 1.0f;
	if (ScreenHeight > 0.0f)
	{
		AspectRatio = ScreenWidth / ScreenHeight;
	}

	DirectX::XMMATRIX ViewMatrix = DirectX::XMMatrixIdentity();
	DirectX::XMMATRIX ProjectionMatrix = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(60.0f), AspectRatio, 0.1f, 1000.0f);
	DirectX::XMFLOAT3 CameraWorldPosition(0.0f, 0.0f, 0.0f);
	if (CameraSystem != nullptr)
	{
		ViewMatrix = CameraSystem->GetActiveViewMatrix();
		ProjectionMatrix = CameraSystem->GetActiveProjectionMatrix(AspectRatio);
		CameraWorldPosition = CameraSystem->GetActiveCameraPosition();
	}
	SceneViewport->SetFrameCameraData(ViewMatrix, ProjectionMatrix, CameraWorldPosition);

	SceneViewport->BeginFrame(TotalRunTimeSeconds);

	std::vector<RenderingComponent*> RenderingComponents;
	std::vector<LightComponent*> LightComponents;
	for (std::unique_ptr<Actor>& ExistingActor : Actors)
	{
		const std::vector<std::unique_ptr<ActorComponent>>& ActorComponents = ExistingActor->GetComponents();
		for (const std::unique_ptr<ActorComponent>& ExistingComponent : ActorComponents)
		{
			RenderingComponent* Rendering = dynamic_cast<RenderingComponent*>(ExistingComponent.get());
			if (Rendering != nullptr && Rendering->GetIsActive())
			{
				RenderingComponents.push_back(Rendering);
			}

			LightComponent* Light = dynamic_cast<LightComponent*>(ExistingComponent.get());
			if (Light != nullptr && Light->GetIsActive())
			{
				LightComponents.push_back(Light);
			}
		}
	}

	for (LightComponent* ExistingLightComponent : LightComponents)
	{
		if (ExistingLightComponent->GetLightType() == LightType::Directional)
		{
			SceneViewport->SetDirectionalLightData(
				ExistingLightComponent->GetDirection(),
				ExistingLightComponent->GetColor(),
				ExistingLightComponent->GetIntensity());
			break;
		}
	}

	std::stable_sort(
		RenderingComponents.begin(),
		RenderingComponents.end(),
		[](const RenderingComponent* LeftRenderingComponent, const RenderingComponent* RightRenderingComponent)
		{
			return LeftRenderingComponent->GetRenderOrder() < RightRenderingComponent->GetRenderOrder();
		});

	SceneViewport->BeginGeometryPass();

	for (RenderingComponent* ExistingRenderingComponent : RenderingComponents)
	{
		ExistingRenderingComponent->Render(SceneViewport);
	}

	SceneViewport->EndGeometryPass();
	if (SceneViewport->IsDeferredRenderingEnabled())
	{
		SceneViewport->ExecuteDeferredLightingPass();
	}

	SceneViewport->EndFrame();
}

LPCWSTR Game::GetApplicationName() const
{
	return Name;
}

InputDevice* Game::GetInputDevice() const
{
	return Input.get();
}

int Game::GetScreenWidth() const
{
	return ScreenWidth;
}

int Game::GetScreenHeight() const
{
	return ScreenHeight;
}

float Game::GetTotalRunTimeSeconds() const
{
	return TotalRunTimeSeconds;
}

ResourceManager* Game::GetResourceManager() const
{
	return Resources.get();
}

void Game::AddSubsystem(std::unique_ptr<Subsystem> NewSubsystem)
{
	bool ShouldInitializeSubsystem = true;
	if (Subsystems.empty())
	{
		ShouldInitializeSubsystem = false;
	}
	else
	{
		for (const std::unique_ptr<Subsystem>& ExistingSubsystem : Subsystems)
		{
			if (ExistingSubsystem->GetIsInitialized() == false)
			{
				ShouldInitializeSubsystem = false;
				break;
			}
		}
	}

	NewSubsystem->SetOwningGame(this);
	Subsystems.push_back(std::move(NewSubsystem));

	if (ShouldInitializeSubsystem)
	{
		Subsystems.back()->Initialize();
	}
}

void Game::AddActor(std::unique_ptr<Actor> NewActor)
{
	bool ShouldInitializeActor = true;
	if (Subsystems.empty())
	{
		ShouldInitializeActor = false;
	}
	else
	{
		for (const std::unique_ptr<Subsystem>& ExistingSubsystem : Subsystems)
		{
			if (ExistingSubsystem->GetIsInitialized() == false)
			{
				ShouldInitializeActor = false;
				break;
			}
		}
	}

	NewActor->SetOwningGame(this);
	Actors.push_back(std::move(NewActor));

	if (ShouldInitializeActor)
	{
		Actors.back()->Initialize();
	}
}

LRESULT Game::MessageHandler(HWND WindowHandle, UINT Message, WPARAM WParam, LPARAM LParam)
{
	switch (Message)
	{
	case WM_KEYDOWN:
	{
		unsigned int KeyCode = static_cast<unsigned int>(WParam);
		std::cout << "Key: " << KeyCode << std::endl;

		if (Input)
		{
			Input->OnKeyDown(KeyCode);
		}

		if (KeyCode == 27)
		{
			PostQuitMessage(0);
		}

		return 0;
	}
	case WM_KEYUP:
	{
		if (Input)
		{
			Input->OnKeyUp(static_cast<unsigned int>(WParam));
		}
		return 0;
	}
	case WM_MOUSEMOVE:
	{
		if (Input)
		{
			int PositionX = LOWORD(LParam);
			int PositionY = HIWORD(LParam);
			Input->OnMouseMove(PositionX, PositionY);
		}
		return 0;
	}
	default:
	{
		return DefWindowProc(WindowHandle, Message, WParam, LParam);
	}
	}
}

void Game::BeginPlay()
{
	CameraSubsystem* CameraSystem = GetSubsystem<CameraSubsystem>();
	if (FallbackCameraActor == nullptr)
	{
		std::unique_ptr<Actor> CameraActor = std::make_unique<Actor>();
		Transform CameraTransform;
		CameraTransform.Position = DirectX::XMFLOAT3(0.0f, 0.0f, -5.0f);
		CameraActor->SetTransform(CameraTransform);
		std::unique_ptr<CameraComponent> DefaultCameraComponent = std::make_unique<CameraComponent>();
		DefaultCameraComponent->SetRegisterInCameraSubsystem(false);
		FallbackCameraComponent = DefaultCameraComponent.get();
		FallbackCameraActor = CameraActor.get();
		CameraActor->AddComponent(std::move(DefaultCameraComponent));
		AddActor(std::move(CameraActor));
	}

	if (CameraSystem != nullptr)
	{
		CameraSystem->SetFallbackCamera(FallbackCameraComponent);
	}

	bool HasDirectionalLight = false;
	for (const std::unique_ptr<Actor>& ExistingActor : Actors)
	{
		for (const std::unique_ptr<ActorComponent>& ExistingComponent : ExistingActor->GetComponents())
		{
			LightComponent* ExistingLightComponent = dynamic_cast<LightComponent*>(ExistingComponent.get());
			if (ExistingLightComponent != nullptr && ExistingLightComponent->GetLightType() == LightType::Directional)
			{
				HasDirectionalLight = true;
				break;
			}
		}
		if (HasDirectionalLight)
		{
			break;
		}
	}

	if (!HasDirectionalLight)
	{
		std::unique_ptr<Actor> LightActor = std::make_unique<Actor>();
		Transform LightTransform;
		LightTransform.RotationEuler = DirectX::XMFLOAT3(-0.6f, 0.7f, 0.0f);
		LightActor->SetTransform(LightTransform);
		std::unique_ptr<LightComponent> DirectionalLight = std::make_unique<LightComponent>();
		DirectionalLight->SetLightType(LightType::Directional);
		DirectionalLight->SetIntensity(1.0f);
		DirectionalLight->SetColor(DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));
		LightActor->AddComponent(std::move(DirectionalLight));
		AddActor(std::move(LightActor));
	}
}

void Game::RegisterInputHandler(std::unique_ptr<GameInputHandler> NewInputHandler)
{
	if (!NewInputHandler)
	{
		return;
	}

	InputHandlers.push_back(std::move(NewInputHandler));
}

void Game::UnregisterInputHandler(GameInputHandler* ExistingInputHandler)
{
	if (ExistingInputHandler == nullptr)
	{
		return;
	}

	auto ExistingInputHandlerIterator = std::remove_if(
		InputHandlers.begin(),
		InputHandlers.end(),
		[ExistingInputHandler](const std::unique_ptr<GameInputHandler>& CurrentInputHandler)
		{
			return CurrentInputHandler.get() == ExistingInputHandler;
		});
	if (ExistingInputHandlerIterator != InputHandlers.end())
	{
		InputHandlers.erase(ExistingInputHandlerIterator, InputHandlers.end());
	}
}
