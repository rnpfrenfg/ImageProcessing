#pragma once

#include "include.h"

#include <Windows.h>
#include <math.h>

using Real = float;

class QuadSet
{
public:
	QuadSet() = default;
	~QuadSet();

	void Init(HDC& hdc, int sizeX, int sizeY);
	void Init(HDC& hdc, Gdiplus::Bitmap& bitmap);

	void UpdateBitmap();
	void SetPixel_quad(int x, int y, int r, int g, int b);
	void FillBlack();
	void DrawCircle(Real radius, COLORREF color);

	HDC hdc;

	int quadMemorySize;
	int width;
	int height;

	RGBQUAD* quad = nullptr;
private:
	HBITMAP bitmap;
};