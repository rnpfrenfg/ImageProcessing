#pragma once

#include "include.h"

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
	int GetPixel(int x, int y);
	void SetPixel(int x, int y, int r, int g, int b);
	void FillBlack();
	void DrawCircle(Real radius, int r, int g, int b);

	HDC hdc;

	int quadMemorySize;
	int width;
	int height;

	RGBQUAD* quad = nullptr;
private:
	void ClearImage();

	HBITMAP bitmap;
};