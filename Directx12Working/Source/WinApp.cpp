#include "WinApp.h"

DEVMODE _originalDevMode{};

void WinApp::Free()
{
}
//실행되기전 디스플레이 정보로 복원
void WinApp::RestoreDisplay()
{
	ChangeResolution(_originalDevMode.dmPelsWidth, _originalDevMode.dmPelsHeight);
}


void WinApp::SaveCurrentResolution()
{
}

void WinApp::ChangeResolution(int width, int height)
{
}

bool WinApp::Initialize(HINSTANCE hInstance, const TCHAR* appName, int width, int height, bool isFullScreen, bool showCursor)
{
	_width = width;
	_height = height;
	wc.hInstance = hInstance;
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	//wc.hCursor = LoadCursorFromFile(TEXT("../Resources/Cursor/black.cur"));
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = appName;

	RegisterClassExW(&wc);

	if (isFullScreen)
	{
		SaveCurrentResolution();
		ChangeResolution(_width, _height);
	}

	RECT rect{ _startLeft, _startTop, _startLeft + width, _startTop + height };
	::AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

	int adjustedWidth = rect.right - rect.left;
	int adjustedHeight = rect.bottom - rect.top;

	_hInstance = hInstance;

	if (isFullScreen)
	{
		_hWnd = CreateWindowW(appName, appName, WS_POPUP | WS_SYSMENU,
			_startLeft, _startTop, adjustedWidth, adjustedHeight, NULL, NULL, hInstance, NULL);

		if (!_hWnd)
			return false;

		ShowWindow(_hWnd, SW_MAXIMIZE);
	}
	else
	{
		_hWnd = CreateWindow(appName, appName, WS_OVERLAPPEDWINDOW,
			_startLeft, _startTop, adjustedWidth, adjustedHeight, NULL, NULL, hInstance, NULL);

		if (!_hWnd)
			return false;

		ShowWindow(_hWnd, SW_SHOW);
	}
	//ShowCursor(showCursor);
	UpdateWindow(_hWnd);

	return true;
}
WinApp* WinApp::Create(HINSTANCE hInstance, const TCHAR* appName, int width, int height, bool isFullScreen, bool showCursor)
{
	WinApp* pInstance = new WinApp;
	if (pInstance->Initialize(hInstance, appName, width, height, isFullScreen, showCursor))
	{
		return pInstance;
	}
	SafeRelease(pInstance);
	return nullptr;
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
#ifdef _DEBUG
#endif // 


LRESULT WinApp::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
		return true;
#ifdef _DEBUG
#endif // _DEBUG

	switch (message)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}
