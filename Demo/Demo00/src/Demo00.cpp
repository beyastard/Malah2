#include <Windows.h>

#include "Demo00.h"
#include "Render.h"

#include "ATime.h"

#include "../res/resource.h"

static void _LogOutput(const wchar_t* szMsg)
{
	l_Log.Log(szMsg);
}

// WinMain
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, char* lpCmdLine, int nCmdShow)
{
	// Register window class
	if (!RegisterWndClass(hInstance))
	{
		l_Log.Log(L"WinMain, Failed to register window class!");
		return -1;
	}

	// Create main window
	if (!CreateMainWnd(hInstance, nCmdShow))
	{
		l_Log.Log(L"WinMain, Failed to create main window!");
		return -1;
	}

	static DWORD dwLastFrame = ATime_GetTime();
	MSG	msg;

	while (true)
	{
		DWORD dwTickTime = ATime_GetTime() - dwLastFrame;

		if (dwTickTime)
			dwLastFrame = ATime_GetTime();

		while (PeekMessage(&msg, nullptr, 0, 0, PM_NOREMOVE))
		{
			GetMessage(&msg, nullptr, 0, 0);
			if (msg.message == WM_QUIT)
				return 0;

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		Sleep(5); // Sleep a little time
	}

	return 0;
}

ATOM RegisterWndClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
	wcex.lpszMenuName = nullptr;
	wcex.lpszClassName = l_szClassName;
	wcex.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));

	return RegisterClassEx(&wcex);
}

bool CreateMainWnd(HINSTANCE hInstance, int nCmdShow)
{
	l_hInstance = hInstance;

	DWORD dwStyles = WS_POPUP;
	if (!l_bFullScreen)
		dwStyles |= WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU | WS_THICKFRAME;

	DWORD dwExStyle = 0;
#ifdef UNICODE
	const wchar_t* l_szWndName = L"Aurora v0.1 Demo 00 Test Window";
#else
	const char* l_szWndName = "Aurora v0.1 Demo 00 Test Window";
#endif

	HWND hWnd = CreateWindowEx(dwExStyle, l_szClassName, l_szWndName, dwStyles, 0, 0,
		l_iRenderWidth, l_iRenderHeight, nullptr, nullptr, hInstance, nullptr);
	if (!hWnd)
		return false;

	l_hMainWnd = hWnd;

	// Adjust window position and size in non-fullscreen mode
	if (!l_bFullScreen)
	{
		RECT rcWnd = { 0, 0, l_iRenderWidth, l_iRenderHeight };
		AdjustWindowRect(&rcWnd, dwStyles, FALSE);

		const int w = rcWnd.right - rcWnd.left;
		const int h = rcWnd.bottom - rcWnd.top;

		const int x = (GetSystemMetrics(SM_CXSCREEN) - w) / 2;
		const int y = (GetSystemMetrics(SM_CYSCREEN) - h) / 2;

		MoveWindow(l_hMainWnd, x, y, w, h, FALSE);
	}

	// Show main window
	ShowWindow(l_hMainWnd, nCmdShow);
	UpdateWindow(l_hMainWnd);

	// we force set foreground window to ensure our main window is activated
	SetForegroundWindow(l_hMainWnd);

	return true;
}

LRESULT __stdcall WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	case WM_SETCURSOR:
		SetCursor(nullptr);
		break;

	case WM_KEYDOWN:
	{
		switch (wParam)
		{
		case VK_ESCAPE:
			PostMessage(hWnd, WM_CLOSE, 0, 0);
			return 0;
		default:;
		}
		break;
	}

	case WM_SIZE:
	{
		if (wParam == SIZE_MINIMIZED || wParam == SIZE_MAXHIDE || wParam == SIZE_MAXSHOW)
			break;

		const int cx = (lParam & 0x0000ffff);
		const int cy = (lParam & 0xffff0000) >> 16;

		if (!cx || !cy)
			return 0;

		g_Render.ResizeDevice(cx, cy);
		break;
	}
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}
