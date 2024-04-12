#include "include.h"

#include "KMeans.h"
#include "QuadSet.h"
#include <random>
#include <iostream>
#include <string>

#define CONSOLE_DEBUG FALSE

HWND winHandle;
HDC winCon;
HINSTANCE instance;

std::thread* consoleThread;
KMeans* mean;
QuadSet buffer;

int graphHeight = 400;

const bool ONLY_COLOR = false;

using Real = float;

const wchar_t CLASSNAME[] = L"test";
const wchar_t WINDOWNAME[] = L"test";

int WINDOW_WIDTH = 1200;
int WINDOW_HEIGHT = 700;

int boardWidth = 800;
int boardHeight = 500;
int dots = 5;
int clusters = 700;

class cRGB {
public:
	cRGB(int r, int g, int b)
	{ 
		this->r = r; this->g = g; this->b = b;
	}
	int r, g, b;
};

cRGB kColor[6] = {
	{255,0,0},
	{0,255,0},
	{0,0,255},
	{0,255,255},
	{255,0,255},
	{255,255,0},
};

void PrintError(const wchar_t* err)
{
	MessageBox(winHandle, err, L"ERROR", MB_OK);
}

void ReDraw()
{
	InvalidateRect(winHandle, NULL, true);
}

void MessageLoop()
{
	MSG msg;

	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (msg.message == WM_QUIT)
		{
			PostQuitMessage(0);
			return;
		}
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		continue;
	}
}

void PrintClusterCount() {
	for (int i = 0; i < mean->meansCount; i++) {
		printf("[%d] : %d\n", i, mean->clusterCount[i]);
	}
}

void CCreateConsole()
{
	AllocConsole();
	freopen("CONOUT$", "w", stdout);
	freopen("CONIN$", "r", stdin);
}

LRESULT CALLBACK winProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_CREATE:
		winCon = GetDC(hwnd);
		buffer.Init(winCon, WINDOW_WIDTH, WINDOW_HEIGHT);
		return 0;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		BeginPaint(winHandle, &ps);

		BitBlt(ps.hdc, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, NULL, 0, 0, WHITENESS);

		buffer.FillWhite();
		auto& dots = mean->dotLocation;
		for (int i = 0; i < mean->dotCount; i++) {
			auto& dot = dots[i];
			int k = mean->neareastMean[i];

			auto& rgb = kColor[k];
			buffer.SetPixel(dot.x, dot.y, rgb.r, rgb.g, rgb.b);
		}

		for (int i = 0; i < mean->meansCount; i++) {
			auto& meanLoc = mean->meansLoc[i];
			auto& rgb = kColor[i];

			buffer.DrawCircle(4, meanLoc.x, meanLoc.y, 0, 0, 0);
			buffer.DrawCircle(3, meanLoc.x, meanLoc.y, rgb.r,rgb.g,rgb.b);
		}
		buffer.UpdateBitmap();
		BitBlt(ps.hdc, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, buffer.hdc, 0, 0, SRCCOPY);
		MoveToEx(ps.hdc, 0, boardHeight, NULL);
		LineTo(ps.hdc, boardWidth, boardHeight);
		LineTo(ps.hdc, boardWidth, 0);

		PrintClusterCount();

		EndPaint(winHandle, &ps);
		return 0;
	}
	case WM_DESTROY:
		ReleaseDC(hwnd, winCon);
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

void SetWindowClass(WNDCLASSEX& winc)
{
	winc.cbSize = sizeof(winc);
	winc.style = 0;
	winc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	winc.hInstance = instance;
	winc.hCursor = LoadCursor(NULL, IDC_ARROW);
	winc.cbClsExtra = 0;
	winc.cbWndExtra = 0;
	winc.hbrBackground = NULL;
	winc.lpszMenuName = WINDOWNAME;
	winc.lpszClassName = CLASSNAME;
	winc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	winc.lpfnWndProc = winProc;
}

void consoleAccpet() {
	std::string in;
	int count;
	while (true) {
		std::cin >> in;
		count = 0;
		for (char c : in) {
			if (c >= '0' && c <= '9') {
				count *= 10;
				count += c - '0';
			}
			else {
				c = 1; break;
			}
		}
		for (int i = 0; i < count; i++) {
			mean->FindNeareastMean();
			mean->ReCalculateMean();
		}

		ReDraw();
	}
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int CmdShow)
{
	setlocale(LC_ALL, "");
	CCreateConsole();
	consoleThread = new std::thread(consoleAccpet);

	mean = new KMeans(boardWidth, boardHeight, dots, clusters);
	mean->Init();
	mean->FindNeareastMean();

	WNDCLASSEX winc;
	instance = hInstance;
	SetWindowClass(winc);

	if (!(RegisterClassEx(&winc)))
	{
		PrintError(L"Cannot register class");
		return 0;
	}

	winHandle = CreateWindowEx(NULL, CLASSNAME, WINDOWNAME, WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, 0, 0,
		WINDOW_WIDTH, WINDOW_HEIGHT, NULL, NULL, hInstance, NULL);

	ShowWindow(winHandle, CmdShow);
	UpdateWindow(winHandle);

	MessageLoop();
	UnregisterClass(CLASSNAME, hInstance);
	return 0;
}