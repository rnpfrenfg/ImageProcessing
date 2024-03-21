#include "include.h"

#include <random>

HWND winHandle;
HDC winCon;
HINSTANCE instance;

QuadSet image;
QuadSet grayImage;
QuadSet histogramImage;
QuadSet histogramColor;

const bool ONLY_COLOR = false;

class HSV {
public:
	double rgbRed;
	double rgbBlue, rgbGreen;
};

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

void ReCalWindowSize() {
	if (ONLY_COLOR) {
		WINDOW_WIDTH = image.width * 2;
		resizedImageWidth = image.width;
		WINDOW_HEIGHT = image.height;
	}
	else {
		WINDOW_WIDTH = 1500;
		resizedImageWidth = (float)WINDOW_WIDTH / 4;
		WINDOW_HEIGHT = image.height * resizedImageWidth / (float)image.width;
	}
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

void RGBtoHSV(RGBQUAD& rgb, HSV& hsv) {
	int r = rgb.rgbRed;
	int g = rgb.rgbGreen;
	int b = rgb.rgbBlue;

	double rt = r / 255.0;
	double gt = g / 255.0;
	double bt = b / 255.0;
	double cMax = max(rt, max(gt, bt));
	double cMin = min(rt, min(gt, bt));
	double delta = cMax - cMin;

	double H, S, V;

	if (r == g && g == b)
		H = 0;
	else if (r >= g && r >= b) {
		H = 60 * ((gt - bt) / delta);
	}
	else if (g >= b && g >= r) {
		H = 60 * ((bt - rt) / delta + 2);
	}
	else H = 60 * ((rt - gt) / delta + 4);
	if (H < 0)
		H += 360;

	if (r == g && g == b) {
		S = 0;
	}
	else S = delta / cMax;

	hsv.rgbRed = H;
	hsv.rgbGreen = S;
	hsv.rgbBlue = max(r, max(g, b));
}

void HSVtoRGB(HSV& hsv, RGBQUAD& pixel) {
	double H = hsv.rgbRed/60.;
	double S = hsv.rgbGreen;
	double V = hsv.rgbBlue;
	double ff = H - ((int)H);
	int p = V * (1.0 - S);
	int q = V * (1.0 - (S * ff));
	int t = V * (1.0 - (S * (1.0 - ff)));

	if (S <= 0.00001f) {
		pixel.rgbBlue = pixel.rgbRed = pixel.rgbGreen = V;
		return;
	}

	switch ((int)H) {
	case 0:
		pixel.rgbRed = V;
		pixel.rgbGreen = t;
		pixel.rgbBlue = p;
		break;
	case 1:
		pixel.rgbRed = q;
		pixel.rgbGreen = V;
		pixel.rgbBlue = p;
		break;
	case 2:
		pixel.rgbRed = p;
		pixel.rgbGreen = V;
		pixel.rgbBlue = t;
		break;
	case 3:
		pixel.rgbRed = p;
		pixel.rgbGreen = q;
		pixel.rgbBlue = V;
		break;
	case 4:
		pixel.rgbRed = t;
		pixel.rgbGreen = p;
		pixel.rgbBlue = V;
		break;
	default:
		pixel.rgbRed = V;
		pixel.rgbGreen = p;
		pixel.rgbBlue = q;
		break;
	}
}

void WorkColorHistogram(QuadSet& image) {
	RGBQUAD* quad = image.quad;
	HSV* hsv = new HSV[image.width * image.height];
	int size = image.width * image.height;

	const int allColors = 256;
	int count[allColors] = { 0, };

	for (int i = 0; i < size; i++) {
		RGBtoHSV(quad[i], hsv[i]);

		count[(int) hsv[i].rgbBlue] += 1;
	}

	int sum = 0;
	int changeTo[allColors] = { 0, };
	for (int i = 0; i < allColors; i++) {
		sum += count[i];
		changeTo[i] = round(allColors * (sum / (double)size));
		if (changeTo[i] >= allColors)
			changeTo[i] = allColors - 1;
	}

	for (int i = 0; i < size; i++) {
		auto& hsvPixel = hsv[i];
		hsvPixel.rgbBlue = changeTo[(int) hsvPixel.rgbBlue];
		HSVtoRGB(hsvPixel, quad[i]);
	}
	delete[] hsv;
}

void Work() {
	ColorToGray(grayImage);
	grayImage.UpdateBitmap();

	memcpy(histogramImage.quad, grayImage.quad, image.quadMemorySize);
	WorkHistogram(histogramImage);
	histogramImage.UpdateBitmap();

	WorkColorHistogram(histogramColor);
	histogramColor.UpdateBitmap();
}

int Random()
{
	static std::random_device rd;
	static std::mt19937 gen;
	static std::uniform_int_distribution<int> distribution;
	return distribution(gen);
}

void Test_HsvRgb() {
	{
		RGBQUAD rgb;
		RGBQUAD newRgb;
		rgb.rgbRed = 29;
		rgb.rgbGreen = 29;
		rgb.rgbBlue = 9;

		HSV hsv;
		RGBtoHSV(rgb, hsv);
		HSVtoRGB(hsv, newRgb);

		if (rgb.rgbRed != newRgb.rgbRed || rgb.rgbGreen != newRgb.rgbGreen || rgb.rgbBlue != newRgb.rgbBlue) {
			printf("[%d, %d, %d] ", rgb.rgbRed, rgb.rgbGreen, rgb.rgbBlue);
			printf("[%lf, %lf, %lf] ", hsv.rgbRed, hsv.rgbGreen, hsv.rgbBlue);
			printf("[%d, %d, %d] \n", newRgb.rgbRed, newRgb.rgbGreen, newRgb.rgbBlue);
		}
	}
	for (int i = 0; i < 1000; i++) {
		RGBQUAD rgb;
		RGBQUAD newRgb;
		rgb.rgbRed = Random() % 256;
		rgb.rgbGreen = Random() % 256;
		rgb.rgbBlue = Random() % 256;

		HSV hsv;
		RGBtoHSV(rgb, hsv);
		HSVtoRGB(hsv, newRgb);

		int dif = abs(rgb.rgbRed - newRgb.rgbRed) + abs(rgb.rgbGreen - newRgb.rgbGreen) + abs(rgb.rgbBlue - newRgb.rgbBlue);

		if (dif > 3) {
			printf("[%d, %d, %d] ", rgb.rgbRed, rgb.rgbGreen, rgb.rgbBlue);
			printf("[%lf, %lf, %lf] ", hsv.rgbRed, hsv.rgbGreen, hsv.rgbBlue);
			printf("[%d, %d, %d] \n", newRgb.rgbRed, newRgb.rgbGreen, newRgb.rgbBlue);
		}
	}
}

LRESULT CALLBACK winProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_CREATE:
		winCon = GetDC(hwnd);
		DragAcceptFiles(hwnd, true);
		return 0;
	case WM_PAINT:
		PAINTSTRUCT ps;
		BeginPaint(winHandle, &ps);

		BitBlt(ps.hdc, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, NULL, 0, 0, BLACKNESS);

		if (!ONLY_COLOR) {
			StretchBlt(ps.hdc, 0, 0, resizedImageWidth, WINDOW_HEIGHT, image.hdc, 0, 0, image.width, image.height, SRCCOPY);
			StretchBlt(ps.hdc, resizedImageWidth * 1, 0, resizedImageWidth, WINDOW_HEIGHT, grayImage.hdc, 0, 0, image.width, image.height, SRCCOPY);
			StretchBlt(ps.hdc, resizedImageWidth * 2, 0, resizedImageWidth, WINDOW_HEIGHT, histogramImage.hdc, 0, 0, image.width, image.height, SRCCOPY);
			StretchBlt(ps.hdc, resizedImageWidth * 3, 0, resizedImageWidth, WINDOW_HEIGHT, histogramColor.hdc, 0, 0, image.width, image.height, SRCCOPY);
		}
		else {
			BitBlt(ps.hdc, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, image.hdc, 0, 0, SRCCOPY);
			BitBlt(ps.hdc, image.width, 0, WINDOW_WIDTH, WINDOW_HEIGHT, histogramColor.hdc, 0, 0, SRCCOPY);
		}

		EndPaint(winHandle, &ps);
		return 0;
	case WM_DROPFILES:
	{
		HDROP drop = (HDROP)wParam;
		UINT fileCount = DragQueryFile(drop, 0xFFFFFFFF, NULL, 0);

		if (fileCount != 1) {
			break;
		}
		int fileLen = DragQueryFile(drop, 0, NULL, 0) + 1;
		TCHAR* fileName = new TCHAR[fileLen];
		memset(fileName, 0, sizeof(TCHAR) * fileLen);
		DragQueryFile(drop, 0, fileName, 1000);

		CLoadImage(fileName);
		ReCalWindowSize();
		Work();
		SetWindowPos(hwnd, HWND_TOP, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, SWP_NOMOVE);
		ReDraw();
		
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

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int CmdShow)
{
	///CCreateConsole();
	//Test_HsvRgb();

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
	if (!CLoadImage(L"test.bmp")) {
		MessageBox(NULL, L"Cannot load image", L"error", MB_OK);
		exit(0);
	}
	Work();
	
	ReCalWindowSize();

	winHandle = CreateWindowEx(NULL, CLASSNAME, WINDOWNAME, WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, 0, 0,
		WINDOW_WIDTH, WINDOW_HEIGHT, NULL, NULL, hInstance, NULL);

	ShowWindow(winHandle, CmdShow);
	UpdateWindow(winHandle);

	MessageLoop();
	UnregisterClass(CLASSNAME, hInstance);
	return 0;
}