#include "Abstracts/Core/GameInstance.h"
#include "Abstracts/Subsystems/SceneViewportSubsystem.h"
#include <chrono>

GameInstance::GameInstance()
	: ApplicationName(L"GameInstance")
	, ScreenWidth(0)
	, ScreenHeight(0)
	, IsExitRequested(false)
	, TotalRunTimeSeconds(0.0f)
{
}

GameInstance::~GameInstance() = default;

void GameInstance::Initialize(LPCWSTR ApplicationName, int ScreenWidth, int ScreenHeight)
{
	this->ApplicationName = ApplicationName;
	this->ScreenWidth = ScreenWidth;
	this->ScreenHeight = ScreenHeight;
	SceneViewportSubsystemInstance = std::make_unique<SceneViewportSubsystem>();
	SceneViewportSubsystemInstance->InitializeStandalone(
		ApplicationName,
		ScreenWidth,
		ScreenHeight,
		[this](HWND WindowHandle, UINT Message, WPARAM WParam, LPARAM LParam) -> LRESULT
		{
			return this->MessageHandler(WindowHandle, Message, WParam, LParam);
		});
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
	if (SceneViewportSubsystemInstance == nullptr)
	{
		return;
	}

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

		SceneViewportSubsystemInstance->BeginFrame(TotalRunTimeSeconds);
		SessionManagerInstance.RenderFrame(
			SceneViewportSubsystemInstance.get(),
			&PlayerRenderTargetServiceInstance,
			ScreenWidth,
			ScreenHeight);
		SceneViewportSubsystemInstance->EndFrame();
	}
}

LRESULT GameInstance::MessageHandler(HWND WindowHandle, UINT Message, WPARAM WParam, LPARAM LParam)
{
	if (SessionManagerInstance.HandleMessage(WindowHandle, Message, WParam, LParam))
	{
		return 1;
	}

	return 0;
}
