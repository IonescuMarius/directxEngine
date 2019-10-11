#include "..\Includes\dxBackBuffer.h"

dxBackBuffer::dxBackBuffer(HWND hWnd, int width, int height)
{
	mhWnd = hWnd;
	HDC hWndDC = GetDC(hWnd);
	mWidth = width;
	mHeight = height;
	mhDC = CreateCompatibleDC(hWndDC);
	mhSurface = CreateCompatibleBitmap(hWndDC, width, height);
	ReleaseDC(hWnd, hWndDC);
	mhOldObject = (HBITMAP)SelectObject(mhDC, mhSurface);
	HBRUSH white = (HBRUSH)GetStockObject(WHITE_BRUSH);
	HBRUSH oldBrush = (HBRUSH)SelectObject(mhDC, white);
	Rectangle(mhDC, 0, 0, mWidth, mHeight);
	SelectObject(mhDC, oldBrush);
}

dxBackBuffer::~dxBackBuffer()
{
	SelectObject(mhDC, mhOldObject);
	DeleteObject(mhSurface);
	DeleteDC(mhDC);
}

HDC dxBackBuffer::getDC()
{
	return mhDC;
}

int dxBackBuffer::width()
{
	return mWidth;
}

int dxBackBuffer::height()
{
	return mHeight;
}

void dxBackBuffer::present()
{
	HDC hWndDC = GetDC(mhWnd);
	BitBlt(hWndDC, 0, 0, mWidth, mHeight, mhDC, 0, 0, SRCCOPY);
	ReleaseDC(mhWnd, hWndDC);
}