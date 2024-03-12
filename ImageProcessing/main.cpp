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

void ColorToGray(QuadSet& image) {
	int size = image.width * image.height;
	RGBQUAD* quad = image.quad;
	for (int i = 0; i < size; i++) {
		auto& pixel = quad[i];
		int newVal = pixel.rgbRed * 0.299 + 0.587 * pixel.rgbGreen + pixel.rgbBlue * 0.114;
		pixel.rgbBlue = pixel.rgbRed = pixel.rgbGreen = newVal;
	}
}

void WorkHistogram(QuadSet& image) {
	RGBQUAD* quad = image.quad;
	int size = image.width * image.height;
	const int allColors = 256;
	int count[allColors] = { 0, };

	for (int i = 0; i < size; i++) {
		auto& pixel = quad[i];
		count[pixel.rgbRed] += 1;
	}

	int sum = 0;
	int changeTo[allColors] = { 0, };
	for (int i = 0; i < allColors; i++) {
		sum += count[i];
		changeTo[i] = allColors * (sum / (double)size);
		if (changeTo[i] == allColors)
			changeTo[i] = allColors-1;
	}
	for (int i = 0; i < size; i++) {
		auto& pixel = quad[i];
		pixel.rgbBlue = pixel.rgbRed = pixel.rgbGreen = changeTo[pixel.rgbRed];
	}
}

void WorkColorHistogram(QuadSet& image) {
	RGBQUAD* quad = image.quad;
	RGBQUAD* hsv = new RGBQUAD[image.width * image.height];
	int size = image.width * image.height;

	int pixelCount = 0;

	const int allColors = 101;
	int count[allColors] = { 0, };

	for (int i = 0; i < size; i++) {
		int r = quad[i].rgbRed;
		int g = quad[i].rgbRed;
		int b = quad[i].rgbRed;

		//RGB TO HSV
		double rt = r / 255.0;
		double gt = g / 255.0;
		double bt = b / 255.0;
		double cMax = max(r, max(g, b));
		double cMin = min(r, min(g, b));
		double delta = cMax - cMin;

		int H, S, V;
		if (r == g && g == b)
			H = 0;
		if (r > g && r > b) {
			H = 60 * ((gt - bt) / delta);
		}
		else if (g > b && g > r) {
			H = 60 * ((bt - rt) / delta + 2);
		}
		else H = 60 * ((rt - gt) / delta + 4);
		if (H < 0)H += 360;

		if (r == g && g == b && r == 0) {
			S = 0;
		}
		else S = delta*100 / cMax;

		V = cMax;

		hsv[i].rgbRed = H;
		hsv[i].rgbGreen = S;
		hsv[i].rgbBlue = V;

		count[S] += 1;
	}

	int sum = 0;
	int changeTo[allColors] = { 0, };
	for (int i = 0; i < allColors; i++) {
		sum += count[i];
		changeTo[i] = allColors * (sum / (double)size);
		if (changeTo[i] == allColors)
			changeTo[i] = allColors - 1;
	}
	for (int i = 0; i < size; i++) {
		auto& hsvPixel = hsv[i];
		hsvPixel.rgbBlue = changeTo[hsvPixel.rgbBlue];

		int H = hsvPixel.rgbRed;
		int S = hsvPixel.rgbGreen;
		int V = hsvPixel.rgbBlue;
		int C = V/100.0 * S;
		int X = C * (1 - abs((H / 60) % 2 - 1));
		int m = V-C;

		int rgb[3];
		rgb[(7-H/60) % 3] = X;
		switch (H / 60) {
		case 1:case 2:rgb[1] = C; break;
		case 3:case 4:rgb[2] = C; break;
		default:rgb[0] = C; break;
		}

		auto& pixel = quad[i];
		pixel.rgbRed = (rgb[0] + m) * 255;
		pixel.rgbGreen = (rgb[1] + m) * 255;
		pixel.rgbBlue = (rgb[2] + m) * 255;
	}
}

void Work() {
	ColorToGray(grayImage);
	grayImage.UpdateBitmap();

	memcpy(histogramImage.quad, grayImage.quad, image.quadMemorySize);
	WorkHistogram(histogramImage);
	histogramImage.UpdateBitmap();


}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int CmdShow)
{
	CCreateConsole();

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