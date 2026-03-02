#include "DisplayWin32.h"

DisplayWin32::DisplayWin32(LPCWSTR ApplicationName, HINSTANCE InstanceHandle, int Width, int Height, WndProcCallback MessageCallback)
	: InstanceHandle(InstanceHandle)
	, ClientWidth(Width)
	, ClientHeight(Height)
	, MessageCallback(MessageCallback)
	, WindowHandle(nullptr)
	, WindowClass{}
{
	WindowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	WindowClass.lpfnWndProc = WndProc;
	WindowClass.cbClsExtra = 0;
	WindowClass.cbWndExtra = 0;
	WindowClass.hInstance = InstanceHandle;
	WindowClass.hIcon = LoadIcon(nullptr, IDI_WINLOGO);
	WindowClass.hIconSm = WindowClass.hIcon;
	WindowClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
	WindowClass.hbrBackground = static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
	WindowClass.lpszMenuName = nullptr;
	WindowClass.lpszClassName = ApplicationName;
	WindowClass.cbSize = sizeof(WNDCLASSEX);

	RegisterClassEx(&WindowClass);

	RECT AdjustedRect = { 0, 0, static_cast<LONG>(ClientWidth), static_cast<LONG>(ClientHeight) };
	AdjustWindowRect(&AdjustedRect, WS_OVERLAPPEDWINDOW, FALSE);

	auto Style = WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX | WS_THICKFRAME;
	auto PositionX = (GetSystemMetrics(SM_CXSCREEN) - ClientWidth) / 2;
	auto PositionY = (GetSystemMetrics(SM_CYSCREEN) - ClientHeight) / 2;

	WindowHandle = CreateWindowEx(
		WS_EX_APPWINDOW,
		ApplicationName,
		ApplicationName,
		Style,
		PositionX, PositionY,
		AdjustedRect.right - AdjustedRect.left,
		AdjustedRect.bottom - AdjustedRect.top,
		nullptr, nullptr, InstanceHandle,
		this);

	ShowWindow(WindowHandle, SW_SHOW);
	SetForegroundWindow(WindowHandle);
	SetFocus(WindowHandle);
	ShowCursor(true);
}

DisplayWin32::~DisplayWin32()
{
	if (WindowHandle)
	{
		DestroyWindow(WindowHandle);
		WindowHandle = nullptr;
	}
}

HWND DisplayWin32::GetWindowHandle() const
{
	return WindowHandle;
}

HINSTANCE DisplayWin32::GetInstanceHandle() const
{
	return InstanceHandle;
}

int DisplayWin32::GetClientWidth() const
{
	return ClientWidth;
}

int DisplayWin32::GetClientHeight() const
{
	return ClientHeight;
}

LRESULT CALLBACK DisplayWin32::WndProc(HWND WindowHandle, UINT Message, WPARAM WParam, LPARAM LParam)
{
	if (Message == WM_CREATE)
	{
		auto CreateData = reinterpret_cast<CREATESTRUCT*>(LParam);
		auto Display = reinterpret_cast<DisplayWin32*>(CreateData->lpCreateParams);
		SetWindowLongPtr(WindowHandle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(Display));
	}

	auto Display = reinterpret_cast<DisplayWin32*>(GetWindowLongPtr(WindowHandle, GWLP_USERDATA));
	if (Display && Display->MessageCallback)
	{
		return Display->MessageCallback(WindowHandle, Message, WParam, LParam);
	}

	return DefWindowProc(WindowHandle, Message, WParam, LParam);
}
