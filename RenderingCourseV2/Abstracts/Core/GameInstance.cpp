#include "Abstracts/Core/GameInstance.h"
#include "Abstracts/Subsystems/GameInstanceSubsystem.h"
#include "Abstracts/Subsystems/RenderRuntimeGameInstanceSubsystem.h"
#include "Abstracts/Subsystems/PhysicsRuntimeGameInstanceSubsystem.h"
#include <chrono>

GameInstance* GlobalGameInstance = nullptr;

GameInstance::GameInstance()
	: ApplicationName(L"GameInstance")
	, ScreenWidth(0)
	, ScreenHeight(0)
	, IsExitRequested(false)
	, TotalRunTimeSeconds(0.0f)
{
	GlobalGameInstance = this;
}

GameInstance::~GameInstance()
{
	SessionManagerInstance.CloseAllGames();

	for (std::unique_ptr<GameInstanceSubsystem>& ExistingSubsystem : Subsystems)
	{
		if (ExistingSubsystem != nullptr)
		{
			ExistingSubsystem->Shutdown();
		}
	}
	Subsystems.clear();

	if (GlobalGameInstance == this)
	{
		GlobalGameInstance = nullptr;
	}
}

void GameInstance::Initialize(LPCWSTR ApplicationName, int ScreenWidth, int ScreenHeight)
{
	this->ApplicationName = ApplicationName;
	this->ScreenWidth = ScreenWidth;
	this->ScreenHeight = ScreenHeight;

	if (GetSubsystem<RenderRuntimeGameInstanceSubsystem>() == nullptr)
	{
		std::unique_ptr<RenderRuntimeGameInstanceSubsystem> NewRenderRuntimeSubsystem = std::make_unique<RenderRuntimeGameInstanceSubsystem>();
		AddSubsystem(std::move(NewRenderRuntimeSubsystem));
	}
	if (GetSubsystem<PhysicsRuntimeGameInstanceSubsystem>() == nullptr)
	{
		std::unique_ptr<PhysicsRuntimeGameInstanceSubsystem> NewPhysicsRuntimeSubsystem = std::make_unique<PhysicsRuntimeGameInstanceSubsystem>();
		AddSubsystem(std::move(NewPhysicsRuntimeSubsystem));
	}

	RenderRuntimeGameInstanceSubsystem* RenderRuntimeSubsystem = GetSubsystem<RenderRuntimeGameInstanceSubsystem>();
	if (RenderRuntimeSubsystem != nullptr)
	{
		RenderRuntimeSubsystem->InitializeRuntime(
			ApplicationName,
			ScreenWidth,
			ScreenHeight,
			[this](HWND WindowHandle, UINT Message, WPARAM WParam, LPARAM LParam) -> LRESULT
			{
				return this->MessageHandler(WindowHandle, Message, WParam, LParam);
			});
	}

	PreviousTime = std::chrono::steady_clock::now();
}

int GameInstance::OpenGame(const GameConfigurator& NewGameConfigurator)
{
	return SessionManagerInstance.OpenGame(NewGameConfigurator);
}

void GameInstance::OpenMultipleGames(const std::vector<GameConfigurator>& NewGameConfigurators)
{
	SessionManagerInstance.OpenMultipleGames(NewGameConfigurators);
}

void GameInstance::CloseGame(int SessionIdentifier)
{
	SessionManagerInstance.CloseGame(SessionIdentifier);
}

void GameInstance::Run()
{
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
		const float DeltaTime = std::chrono::duration_cast<std::chrono::microseconds>(CurrentTime - PreviousTime).count() / 1000000.0f;
		PreviousTime = CurrentTime;
		TotalRunTimeSeconds += DeltaTime;

		SessionManagerInstance.TickFrame(DeltaTime);

		RenderRuntimeGameInstanceSubsystem* RenderRuntimeSubsystem = GetSubsystem<RenderRuntimeGameInstanceSubsystem>();
		if (RenderRuntimeSubsystem != nullptr)
		{
			RenderRuntimeSubsystem->BeginRuntimeFrame(TotalRunTimeSeconds);
			SessionManagerInstance.RenderFrame(
				RenderRuntimeSubsystem,
				ScreenWidth,
				ScreenHeight);
			RenderRuntimeSubsystem->EndRuntimeFrame();
		}
	}
}

LRESULT GameInstance::MessageHandler(HWND WindowHandle, UINT Message, WPARAM WParam, LPARAM LParam)
{
	RenderRuntimeGameInstanceSubsystem* RenderRuntimeSubsystem = GetSubsystem<RenderRuntimeGameInstanceSubsystem>();
	if (Message == WM_CLOSE || Message == WM_DESTROY)
	{
		IsExitRequested = true;
		PostQuitMessage(0);
		return 0;
	}
	if (Message == WM_SIZE && RenderRuntimeSubsystem != nullptr)
	{
		const bool IsWindowMinimized = WParam == SIZE_MINIMIZED;
		RenderRuntimeSubsystem->SetIsWindowMinimized(IsWindowMinimized);
	}

	if (SessionManagerInstance.HandleMessage(WindowHandle, Message, WParam, LParam))
	{
		return 1;
	}

	return DefWindowProc(WindowHandle, Message, WParam, LParam);
}

void GameInstance::AddSubsystem(std::unique_ptr<GameInstanceSubsystem> NewSubsystem)
{
	if (NewSubsystem == nullptr)
	{
		return;
	}

	NewSubsystem->SetOwningGameInstance(this);
	Subsystems.push_back(std::move(NewSubsystem));
}
