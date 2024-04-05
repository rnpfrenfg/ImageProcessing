#include "QuadSet.h"

void QuadSet::Init(HDC& hdc, int sizeX, int sizeY)
{
	width = sizeX;
	height = sizeY;
	quadMemorySize = width * height * 4;

	quad = new RGBQUAD[width * height];

	this->hdc = CreateCompatibleDC(hdc);
	bitmap = CreateCompatibleBitmap(hdc, sizeX, sizeY);
	SelectObject(this->hdc, bitmap);

	GetBitmapBits(bitmap, quadMemorySize, quad);
}

void QuadSet::Init(HDC& hdc, Gdiplus::Bitmap& bitmap) {
	ClearImage();

	width = bitmap.GetWidth();
	height = bitmap.GetHeight();
	quadMemorySize = width * height * 4;
	quad = new RGBQUAD[width * height];
	this->hdc = CreateCompatibleDC(hdc);
	bitmap.GetHBITMAP(Gdiplus::Color(0, 0, 0, 0), &this->bitmap);
	GetBitmapBits(this->bitmap, quadMemorySize, quad);
	SelectObject(this->hdc, this->bitmap);
}

QuadSet::~QuadSet()
{
	ClearImage();
}

void QuadSet::ClearImage() {
	if (quad != nullptr) {
		DeleteDC(hdc);
		DeleteObject(bitmap);
		delete[] quad;
		quad = nullptr;
	}
}

void QuadSet::UpdateBitmap() {
	SetBitmapBits(this->bitmap, quadMemorySize, quad);
}

void QuadSet::SetPixel(int x, int y, int r, int g, int b)
{
	if (x >= width || y >= height)
		return;
	if (x < 0 || y < 0)
		return;
	int drawTo = x + width * y;

	quad[drawTo].rgbRed = r;
	quad[drawTo].rgbGreen = g;
	quad[drawTo].rgbBlue = b;
}

int QuadSet::GetPixel(int x, int y) {
	if (x < 0 || x >= width)
		return 0;
	if (y < 0 || y >= height)
		return 0;

	return quad[x + width * y].rgbRed;
}

void QuadSet::FillBlack()
{
	memset(quad, 0, quadMemorySize);
}

inline Real Max50(Real k)
{
	if (k > 50)
		return k;
	return 0;
}

void QuadSet::DrawCircle(Real radius, int r,int g, int b)
{
	int x, y;

	for (x = 0; x < radius * 2; x++)
		for (y = 0; y < radius * 2; y++)
		{
			Real xDis = x - r;
			Real yDis = y - r;
			Real distance = sqrt(xDis * xDis + yDis * yDis);

			if (distance <= r) {
				quad[x + width * y].rgbRed = r;
				quad[x + width * y].rgbGreen = g;
				quad[x + width * y].rgbBlue = b;
			}
		}
}