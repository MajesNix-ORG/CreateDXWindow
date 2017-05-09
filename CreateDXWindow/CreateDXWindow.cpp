#include "stdafx.h"
#include <string>
#include <windows.h>
#include <vector>
#include "renderer.h"

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

LRESULT WINAPI WndProc(HWND, UINT, WPARAM, LPARAM);

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int cmdShow)
{
	//Give the possibility to use relative file paths
	std::string currentDir = __argv[0];
	size_t index = currentDir.find_last_of('\\');
	currentDir.erase(index);
	SetCurrentDirectoryA(currentDir.c_str());

	WNDCLASSEX wc = {};
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WndProc;
	wc.hInstance = hInst;
	wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wc.lpszClassName = L"My Window";

	RegisterClassEx(&wc);

	DWORD wndStyle = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
	HWND hwnd = CreateWindowEx(0, wc.lpszClassName, L"My Window",
		wndStyle, 0, 0, 0, 0, nullptr, nullptr, wc.hInstance, 0);

	DEVMODE devmode = {};
	EnumDisplaySettings(nullptr, ENUM_CURRENT_SETTINGS, &devmode);

	const int width = 1366;
	const int height = 768;

	int left = (devmode.dmPelsWidth - width) / 2;
	int top = (devmode.dmPelsHeight - height) / 2;
	RECT rc = {};
	SetRect(&rc, left, top, left + width, top + height);

	AdjustWindowRectEx(&rc, wndStyle, false, 0);

	MoveWindow(hwnd, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, TRUE);

	Renderer renderer;
	if (!renderer.Init(hwnd, width, height)) {
		return 0;
	}

	bool open = true;
	MSG msg = {};
	while (open)
	{
		while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			if (msg.message == WM_QUIT)
				open = false;
		}

		if (!renderer.RenderBegin())
			break;
		if (!renderer.Render())
			break;
		if (!renderer.RenderEnd())
			break;
	}
}

LRESULT WINAPI WndProc(HWND hwnd, UINT msgId, WPARAM wparam, LPARAM lparam)
{
	switch (msgId)
	{
	case WM_CLOSE:
		DestroyWindow(hwnd);
		return 0;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	default:
		break;
	}

	return DefWindowProc(hwnd, msgId, wparam, lparam);
}