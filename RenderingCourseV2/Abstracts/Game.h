#pragma once

#include <windows.h>
#include <wrl.h>
#include <d3d11.h>
#include <vector>
#include <memory>
#include <chrono>

class DisplayWin32;
class InputDevice;
class GameComponent;

class Game
{
public:
	Game(LPCWSTR ApplicationName, int ScreenWidth, int ScreenHeight);
	virtual ~Game();

	Game(const Game&) = delete;
	Game& operator=(const Game&) = delete;

	void Initialize();
	void Run();

	ID3D11Device* GetDevice() const;
	ID3D11DeviceContext* GetDeviceContext() const;
	IDXGISwapChain* GetSwapChain() const;
	DisplayWin32* GetDisplay() const;
	InputDevice* GetInputDevice() const;
	int GetScreenWidth() const;
	int GetScreenHeight() const;
	float GetTotalTime() const;

	void AddComponent(std::unique_ptr<GameComponent> Component);

	LRESULT MessageHandler(HWND WindowHandle, UINT Message, WPARAM WParam, LPARAM LParam);

protected:
	virtual void PrepareResources();
	
	virtual void BeginPlay();
	virtual void Update(float DeltaTime);
	virtual void Draw();
	void PrepareFrame();
	void CreateBackBuffer();
	void RestoreTargets();
	void DestroyResources();

	LPCWSTR Name;
	int ScreenWidth;
	int ScreenHeight;

	std::unique_ptr<DisplayWin32> Display;
	std::unique_ptr<InputDevice> Input;

	Microsoft::WRL::ComPtr<ID3D11Device> Device;
	ID3D11DeviceContext* Context;
	IDXGISwapChain* SwapChain;
	ID3D11Texture2D* BackBuffer;
	ID3D11RenderTargetView* RenderView;

	std::vector<std::unique_ptr<GameComponent>> Components;

	std::chrono::time_point<std::chrono::steady_clock> StartTime;
	std::chrono::time_point<std::chrono::steady_clock> PreviousTime;
	float TotalTime;
	unsigned int FrameCount;
	bool IsExitRequested;
};
