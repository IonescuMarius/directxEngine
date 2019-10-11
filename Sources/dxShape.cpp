#include "..\Includes\dxShape.h"

dxShape::dxShape(const POINT u, const POINT v, const LOGPEN& lp, const LOGBRUSH& lb)
{
	mPt0.x = u.x;
	mPt0.y = u.y;
	mPt1.x = v.x;
	mPt1.y = v.y;
	mhPen = CreatePenIndirect(&lp);
	mhBrush = CreateBrushIndirect(&lb);
	mhOldPen = 0;
	mhOldBrush = 0;
}

dxShape::~dxShape()
{
	DeleteObject(mhPen);
	DeleteObject(mhBrush);
}

void dxShape::setStartPt(const POINT& p0)
{
	mPt0 = p0;
}
void dxShape::setEndPt(const POINT& p1)
{
	mPt1 = p1;
}

LineShape::LineShape(const POINT u, const POINT v, const LOGPEN& lp, const LOGBRUSH& lb) : dxShape(u, v, lp, lb)
{

}

void LineShape::draw(HDC hdc)
{
	// Select the current pen and brush.
	mhOldPen = (HPEN)SelectObject(hdc, mhPen);
	mhOldBrush = (HBRUSH)SelectObject(hdc, mhBrush);
	// Draw the line.
	MoveToEx(hdc, mPt0.x, mPt0.y, 0);
	LineTo(hdc, mPt1.x, mPt1.y);
	// Restore the old pen and brush.
	SelectObject(hdc, mhOldPen);
	SelectObject(hdc, mhOldBrush);
}

RectShape::RectShape(const POINT u, const POINT v, const LOGPEN& lp, const LOGBRUSH& lb) : dxShape(u, v, lp, lb)
{

}

void RectShape::draw(HDC hdc)
{
	// Select the current pen and brush.
	mhOldPen = (HPEN)SelectObject(hdc, mhPen);
	mhOldBrush = (HBRUSH)SelectObject(hdc, mhBrush);
	// Draw the rectangle.
	Rectangle(hdc, mPt0.x, mPt0.y, mPt1.x, mPt1.y);
	// Restore the old pen and brush.
	SelectObject(hdc, mhOldPen);
	SelectObject(hdc, mhOldBrush);
}
EllipseShape::EllipseShape(const POINT u, const POINT v, const LOGPEN& lp, const LOGBRUSH& lb) : dxShape(u, v, lp, lb)
{

}

void EllipseShape::draw(HDC hdc)
{
	// Select the current pen and brush.
	mhOldPen = (HPEN)SelectObject(hdc, mhPen);
	mhOldBrush = (HBRUSH)SelectObject(hdc, mhBrush);
	// Draw the ellipse.
	Ellipse(hdc, mPt0.x, mPt0.y, mPt1.x, mPt1.y);
	// Restore the old pen and brush.
	SelectObject(hdc, mhOldPen);
	SelectObject(hdc, mhOldBrush);
}
CubeShape::CubeShape(const POINT u, const POINT v, const LOGPEN& lp, const LOGBRUSH& lb) : dxShape(u, v, lp, lb)
{

}

void CubeShape::draw(HDC hdc)
{
	mhOldPen = (HPEN)SelectObject(hdc, mhPen);
	mhOldBrush = (HBRUSH)SelectObject(hdc, mhBrush);
	
	MoveToEx(hdc, mPt0.x, mPt0.y, 0);
	LineTo(hdc, mPt0.x+40, mPt0.y);
	MoveToEx(hdc, mPt0.x, mPt0.y, 0);
	LineTo(hdc, mPt0.x, mPt0.y+40);
	MoveToEx(hdc, mPt0.x, mPt0.y+40, 0);
	LineTo(hdc, mPt0.x+40, mPt0.y+40);
	MoveToEx(hdc, mPt0.x+40, mPt0.y, 0);
	LineTo(hdc, mPt0.x+40, mPt0.y+40);
	MoveToEx(hdc, mPt0.x, mPt0.y, 0);
	LineTo(hdc, mPt0.x+20, mPt0.y-20);
	MoveToEx(hdc, mPt0.x+40, mPt0.y, 0);
	LineTo(hdc, mPt0.x+60, mPt0.y-20);
	MoveToEx(hdc, mPt0.x+20, mPt0.y-20, 0);
	LineTo(hdc, mPt0.x+60, mPt0.y-20);
	MoveToEx(hdc, mPt0.x, mPt0.y+40, 0);
	LineTo(hdc, mPt0.x+20, mPt0.y+20);
	MoveToEx(hdc, mPt0.x+40, mPt0.y+40, 0);
	LineTo(hdc, mPt0.x+60, mPt0.y+20);
	MoveToEx(hdc, mPt0.x+20, mPt0.y-20, 0);
	LineTo(hdc, mPt0.x+20, mPt0.y+20);
	MoveToEx(hdc, mPt0.x+60, mPt0.y-20, 0);
	LineTo(hdc, mPt0.x+60, mPt0.y+20);
	MoveToEx(hdc, mPt0.x+20, mPt0.y+20, 0);
	LineTo(hdc, mPt0.x+60, mPt0.y+20);

	SelectObject(hdc, mhOldPen);
	SelectObject(hdc, mhOldBrush);
}

TextShape::TextShape(const POINT u, const POINT v, const LOGPEN& lp, const LOGBRUSH& lb) : dxShape(u, v, lp, lb)
{

}

void TextShape::draw(HDC hdc)
{
	mhOldPen = (HPEN)SelectObject(hdc, mhPen);
	mhOldBrush = (HBRUSH)SelectObject(hdc, mhBrush);
	
	string s = "Hello, World!";
	TextOut(hdc, mPt0.x, mPt0.y, s.c_str(), s.size());
	
	SelectObject(hdc, mhOldPen);
	SelectObject(hdc, mhOldBrush);
}