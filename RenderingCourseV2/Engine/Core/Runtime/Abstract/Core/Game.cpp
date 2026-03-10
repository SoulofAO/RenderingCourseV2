#include "Engine/Core/Runtime/Abstract/Core/Game.h"
#include "Engine/Core/Runtime/Abstract/Subsystems/InputDevice.h"
#include "Engine/Core/Runtime/Abstract/Subsystems/Subsystem.h"
#include "Engine/Core/Runtime/Abstract/Core/Actor.h"
#include "Engine/Core/Runtime/Abstract/Components/RenderingComponent.h"
#include "Engine/Core/Runtime/Abstract/Subsystems/SceneViewportSubsystem.h"
#include "Engine/Core/Runtime/Abstract/Subsystems/PhysicsSubsystem.h"
#include "Engine/Core/Runtime/Abstract/Subsystems/DisplayWin32.h"
#include <algorithm>
#include <iostream>

Game::Game(LPCWSTR ApplicationName, int ScreenWidth, int ScreenHeight)
	: Name(ApplicationName)
	, ScreenWidth(ScreenWidth)
	, ScreenHeight(ScreenHeight)
	, TotalRunTimeSeconds(0.0f)
	, FrameCount(0)
	, IsExitRequested(false)
{
}

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
}

void Game::Initialize()
{
	Input = std::make_unique<InputDevice>(this);

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
	for (std::unique_ptr<Actor>& ExistingActor : Actors)
	{
		ExistingActor->Update(DeltaTime);
	}

	for (std::unique_ptr<Subsystem>& ExistingSubsystem : Subsystems)
	{
		ExistingSubsystem->Update(DeltaTime);
	}
}

void Game::Draw()
{
	SceneViewportSubsystem* SceneViewport = GetSubsystem<SceneViewportSubsystem>();
	if (SceneViewport == nullptr)
	{
		return;
	}

	SceneViewport->BeginFrame(TotalRunTimeSeconds);

	std::vector<RenderingComponent*> RenderingComponents;
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
		}
	}

	std::stable_sort(
		RenderingComponents.begin(),
		RenderingComponents.end(),
		[](const RenderingComponent* LeftRenderingComponent, const RenderingComponent* RightRenderingComponent)
		{
			return LeftRenderingComponent->GetRenderOrder() < RightRenderingComponent->GetRenderOrder();
		});

	for (RenderingComponent* ExistingRenderingComponent : RenderingComponents)
	{
		ExistingRenderingComponent->Render(SceneViewport);
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

void Game::AddSubsystem(std::unique_ptr<Subsystem> NewSubsystem)
{
	if (NewSubsystem == nullptr)
	{
		return;
	}

	if (NewSubsystem->GetCategory() != SubsystemCategory::Game)
	{
		std::cerr << "Only game subsystems can be added to Game." << std::endl;
		return;
	}

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
}

