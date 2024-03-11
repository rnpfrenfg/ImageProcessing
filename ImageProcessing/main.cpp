#include "include.h"

QuadSet buffer;
HWND winHandle;
HDC winCon;
HINSTANCE instance;

QuadSet image;
QuadSet grayImage;
QuadSet histogramImage;
QuadSet histogramColor;

using Real = float;

const wchar_t CLASSNAME[] = L"test";
const wchar_t WINDOWNAME[] = L"test";

int WINDOW_WIDTH = 1500;
int WINDOW_HEIGHT = 1;
float resizedImageWidth = 0;

void PrintError(const wchar_t* err)
{
	MessageBox(winHandle, err, L"ERROR", MB_OK);
}

void ReDraw()
{
	InvalidateRect(winHandle, NULL, true);
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
		PAINTSTRUCT ps;
		BeginPaint(winHandle, &ps);

		BitBlt(ps.hdc, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, NULL, 0, 0, BLACKNESS);

		StretchBlt(ps.hdc, 0, 0, resizedImageWidth, WINDOW_HEIGHT, image.hdc, 0, 0, image.width, image.height, SRCCOPY);
		StretchBlt(ps.hdc, resizedImageWidth *1, 0, resizedImageWidth, WINDOW_HEIGHT, grayImage.hdc, 0, 0, image.width, image.height, SRCCOPY);
		StretchBlt(ps.hdc, resizedImageWidth * 2, 0, resizedImageWidth, WINDOW_HEIGHT, histogramImage.hdc, 0, 0, image.width, image.height, SRCCOPY);
		StretchBlt(ps.hdc, resizedImageWidth * 3, 0, resizedImageWidth, WINDOW_HEIGHT, histogramColor.hdc, 0, 0, image.width, image.height, SRCCOPY);


		EndPaint(winHandle, &ps);
		return 0;
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

void CCreateConsole()
{
	AllocConsole();
	freopen("CONOUT$", "w", stdout);
	freopen("CONIN$", "r", stdin);
}

bool CLoadImage(std::wstring s) {

	Gdiplus::Bitmap* bitmap = Gdiplus::Bitmap::FromFile(s.c_str());
	if (bitmap == nullptr || Gdiplus::Ok != bitmap->GetLastStatus()) {
		return false;
	}

	image.Init(winCon, *bitmap);
	grayImage.Init(winCon, *bitmap);
	histogramImage.Init(winCon, *bitmap);
	histogramColor.Init(winCon, *bitmap);
	return true;
}

void Work() {
	RGBQUAD* quad = grayImage.quad;
	int size = grayImage.width * grayImage.height;
	for (int i = 0; i < size; i++) {
		auto& pixel = quad[i];
		int newVal= pixel.rgbRed * 0.299 + 0.587 * pixel.rgbGreen + pixel.rgbBlue * 0.114;
		pixel.rgbBlue = pixel.rgbRed = pixel.rgbGreen = newVal;
	}
	grayImage.UpdateBitmap();

	memcpy(histogramImage.quad, quad, size);
	quad = histogramImage.quad;
	const int allColors = 256;
	int count[allColors] = { 0, };
	int all = 0;
	for (int i = 0; i < size; i++) {
		auto& pixel = quad[i];
		all++;
		count[pixel.rgbRed] += 1;
	}
	int sum = 0;
	int changeTo[allColors] = { 0, };
	for (int i = 0; i < allColors; i++) {
		sum += count[i];
		changeTo[i] = allColors * (sum/(double)all);
		if (changeTo[i] == 256)
			changeTo[i] = 255;
	}
	for (int i = 0; i < size; i++) {
		auto& pixel = quad[i];
		pixel.rgbBlue = pixel.rgbRed = pixel.rgbGreen = changeTo[pixel.rgbRed];
	}
	histogramImage.UpdateBitmap();
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int CmdShow)
{
	WNDCLASSEX winc;
	instance = hInstance;
	SetWindowClass(winc);

	if (!(RegisterClassEx(&winc)))
	{
		PrintError(L"Cannot register class");
		return 0;
	}

	ULONG_PTR ptr;
	Gdiplus::GdiplusStartupInput in;
	Gdiplus::GdiplusStartup(&ptr, &in, 0);
	if (!CLoadImage(L"png.png")) {
		MessageBox(NULL, L"Cannot load image", L"error", MB_OK);
		exit(0);
	}
	Work();

	resizedImageWidth = (float)WINDOW_WIDTH / 4;
	WINDOW_HEIGHT = image.height * resizedImageWidth / (float) image.width;

	winHandle = CreateWindowEx(NULL, CLASSNAME, WINDOWNAME, WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, 0, 0,
		WINDOW_WIDTH, WINDOW_HEIGHT, NULL, NULL, hInstance, NULL);

	ShowWindow(winHandle, CmdShow);
	UpdateWindow(winHandle);

	MessageLoop();
	UnregisterClass(CLASSNAME, hInstance);
	return 0;
}