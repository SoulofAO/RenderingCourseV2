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
#include "Abstracts/Components/FPSSpectateCameraComponent.h"
#include "Abstracts/Components/UIRenderingComponent.h"
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

namespace
{
	void SetMouseCursorVisibleState(bool IsMouseCursorVisible)
	{
		int VisibilityCounter = ShowCursor(IsMouseCursorVisible ? TRUE : FALSE);
		if (IsMouseCursorVisible)
		{
			while (VisibilityCounter < 0)
			{
				VisibilityCounter = ShowCursor(TRUE);
			}
		}
		else
		{
			while (VisibilityCounter >= 0)
			{
				VisibilityCounter = ShowCursor(FALSE);
			}
		}
	}
}

Game::Game(LPCWSTR ApplicationName, int ScreenWidth, int ScreenHeight)
	: Name(ApplicationName)
	, ScreenWidth(ScreenWidth)
	, ScreenHeight(ScreenHeight)
	, TotalRunTimeSeconds(0.0f)
	, FrameCount(0)
	, IsExitRequested(false)
	, FallbackCameraActor(nullptr)
	, FallbackCameraComponentInstance(nullptr)
	, CurrentMouseInputMode(MouseInputMode::GameAndUI)
	, DefaultCameraSettingsWindowVisible(true)
{
}

/**
 * 
 */
Game::~Game()
{
	ClipCursor(nullptr);
	SetMouseCursorVisibleState(true);

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

	ApplyMouseInputMode();
	UpdateInputHandlerActivationState();
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
				if (ExistingInputHandler->bEnable)
				{
					ExistingInputHandler->HandleInput(this, Input.get(), DeltaTime);
				}
			}
		}
	}

	UpdateMouseInputModeState();

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

	bool HasDirectionalLight = false;
	for (LightComponent* ExistingLightComponent : LightComponents)
	{
		if (ExistingLightComponent->GetLightType() == LightType::Directional)
		{
			SceneViewport->SetDirectionalLightData(
				ExistingLightComponent->GetDirection(),
				ExistingLightComponent->GetColor(),
				ExistingLightComponent->GetIntensity(),
				0.0f);
			HasDirectionalLight = true;
			break;
		}
	}
	if (!HasDirectionalLight)
	{
		SceneViewport->SetDirectionalLightData(
			DirectX::XMFLOAT3(0.0f, -1.0f, 0.0f),
			DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
			0.0f,
			1.0f);
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
	if (Message == WM_KEYDOWN && static_cast<unsigned int>(WParam) == 27)
	{
		if (Input)
		{
			Input->OnKeyDown(static_cast<unsigned int>(WParam));
		}

		IsExitRequested = true;
		PostQuitMessage(0);
		return 0;
	}

	for (std::unique_ptr<Actor>& ExistingActor : Actors)
	{
		const std::vector<std::unique_ptr<ActorComponent>>& ActorComponents = ExistingActor->GetComponents();
		for (const std::unique_ptr<ActorComponent>& ExistingActorComponent : ActorComponents)
		{
			UIRenderingComponent* ExistingUIRenderingComponent = dynamic_cast<UIRenderingComponent*>(ExistingActorComponent.get());
			if (ExistingUIRenderingComponent != nullptr)
			{
				if (ExistingUIRenderingComponent->HandleMessage(WindowHandle, Message, WParam, LParam))
				{
					return 1;
				}
			}
		}
	}

	switch (Message)
	{
	case WM_CLOSE:
	case WM_DESTROY:
	{
		IsExitRequested = true;
		PostQuitMessage(0);
		return 0;
	}
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
			IsExitRequested = true;
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
	case WM_MOUSEWHEEL:
	{
		if (Input)
		{
			const int MouseWheelDelta = GET_WHEEL_DELTA_WPARAM(WParam);
			Input->OnMouseWheel(MouseWheelDelta);
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
		std::unique_ptr<FPSSpectateCameraComponent> NewFallbackCameraComponent = std::make_unique<FPSSpectateCameraComponent>();
		NewFallbackCameraComponent->SetRegisterInCameraSubsystem(false);
		FallbackCameraComponentInstance = NewFallbackCameraComponent.get();
		FallbackCameraActor = CameraActor.get();
		CameraActor->AddComponent(std::move(NewFallbackCameraComponent));
		AddActor(std::move(CameraActor));
	}

	if (CameraSystem != nullptr)
	{
		CameraSystem->SetFallbackCamera(FallbackCameraComponentInstance);
	}
}

void Game::RegisterInputHandler(std::unique_ptr<GameInputHandler> NewInputHandler)
{
	if (!NewInputHandler)
	{
		return;
	}

	InputHandlers.push_back(std::move(NewInputHandler));
	UpdateInputHandlerActivationState();
}

void Game::SetMouseInputMode(MouseInputMode NewMouseInputMode)
{
	if (CurrentMouseInputMode == NewMouseInputMode)
	{
		return;
	}

	CurrentMouseInputMode = NewMouseInputMode;
	ApplyMouseInputMode();
	UpdateInputHandlerActivationState();
}

void Game::ToggleMouseInputMode()
{
	if (CurrentMouseInputMode == MouseInputMode::GameOnly)
	{
		SetMouseInputMode(MouseInputMode::GameAndUI);
		return;
	}

	if (CurrentMouseInputMode == MouseInputMode::GameAndUI)
	{
		SetMouseInputMode(MouseInputMode::UIOnly);
		return;
	}

	SetMouseInputMode(MouseInputMode::GameOnly);
}

MouseInputMode Game::GetMouseInputMode() const
{
	return CurrentMouseInputMode;
}

void Game::SetDefaultCameraSettingsWindowVisible(bool NewDefaultCameraSettingsWindowVisible)
{
	DefaultCameraSettingsWindowVisible = NewDefaultCameraSettingsWindowVisible;
}

void Game::ToggleDefaultCameraSettingsWindowVisible()
{
	SetDefaultCameraSettingsWindowVisible(!DefaultCameraSettingsWindowVisible);
}

bool Game::GetDefaultCameraSettingsWindowVisible() const
{
	return DefaultCameraSettingsWindowVisible;
}

bool Game::GetIsFallbackCameraPossessed() const
{
	CameraSubsystem* CameraSystem = GetSubsystem<CameraSubsystem>();
	if (CameraSystem == nullptr)
	{
		return false;
	}

	return CameraSystem->GetActiveCamera() == FallbackCameraComponentInstance;
}

void Game::ApplyMouseInputMode()
{
	SceneViewportSubsystem* SceneViewport = GetSubsystem<SceneViewportSubsystem>();
	if (SceneViewport == nullptr)
	{
		return;
	}

	DisplayWin32* Display = SceneViewport->GetDisplay();
	if (Display == nullptr)
	{
		return;
	}

	HWND WindowHandle = Display->GetWindowHandle();
	if (WindowHandle == nullptr)
	{
		return;
	}

	if (CurrentMouseInputMode == MouseInputMode::GameOnly)
	{
		RECT ClientRectangle;
		GetClientRect(WindowHandle, &ClientRectangle);
		POINT TopLeftPoint = { ClientRectangle.left, ClientRectangle.top };
		POINT BottomRightPoint = { ClientRectangle.right, ClientRectangle.bottom };
		ClientToScreen(WindowHandle, &TopLeftPoint);
		ClientToScreen(WindowHandle, &BottomRightPoint);
		RECT ClipRectangle = { TopLeftPoint.x, TopLeftPoint.y, BottomRightPoint.x, BottomRightPoint.y };
		ClipCursor(&ClipRectangle);
		SetMouseCursorVisibleState(false);
	}
	else
	{
		ClipCursor(nullptr);
		SetMouseCursorVisibleState(true);
	}
}

void Game::UpdateMouseInputModeState()
{
	if (CurrentMouseInputMode != MouseInputMode::GameOnly)
	{
		return;
	}

	SceneViewportSubsystem* SceneViewport = GetSubsystem<SceneViewportSubsystem>();
	if (SceneViewport == nullptr)
	{
		return;
	}

	DisplayWin32* Display = SceneViewport->GetDisplay();
	if (Display == nullptr)
	{
		return;
	}

	HWND WindowHandle = Display->GetWindowHandle();
	if (WindowHandle == nullptr)
	{
		return;
	}

	RECT ClientRectangle;
	GetClientRect(WindowHandle, &ClientRectangle);
	POINT MouseCenterPoint =
	{
		(ClientRectangle.left + ClientRectangle.right) / 2,
		(ClientRectangle.top + ClientRectangle.bottom) / 2
	};

	if (Input != nullptr)
	{
		Input->ResetMouseTracking(MouseCenterPoint.x, MouseCenterPoint.y);
	}

	ClientToScreen(WindowHandle, &MouseCenterPoint);
	SetCursorPos(MouseCenterPoint.x, MouseCenterPoint.y);
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
		UpdateInputHandlerActivationState();
	}
}

void Game::UpdateInputHandlerActivationState()
{
	for (std::unique_ptr<GameInputHandler>& ExistingInputHandler : InputHandlers)
	{
		if (ExistingInputHandler == nullptr)
		{
			continue;
		}

		EngineHotkeyInputHandler* EngineHotkeyInputHandlerInstance = dynamic_cast<EngineHotkeyInputHandler*>(ExistingInputHandler.get());
		if (CurrentMouseInputMode == MouseInputMode::UIOnly)
		{
			ExistingInputHandler->bEnable = (EngineHotkeyInputHandlerInstance != nullptr);
		}
		else
		{
			ExistingInputHandler->bEnable = true;
		}
	}
}


