/****************************************************************************
*                                                                           *
* dxShape.h -- SHAPE                                                        *
*                                                                           *
* Copyright (c) Ionescu Marius. All rights reserved(2010).                  *
*                                                                           *
****************************************************************************/

#ifndef dxShape_H
#define dxShape_H

#include <windows.h>
#include <string>
using namespace std;

class dxShape
{
	public:
		dxShape(const POINT u, const POINT v, const LOGPEN& lp, const LOGBRUSH& lb);
		virtual~dxShape();
		void setStartPt(const POINT& p0);
		void setEndPt(const POINT& p1);
		virtual void draw(HDC hdc) = 0;
	protected:
		POINT mPt0;
		POINT mPt1;
		HPEN mhPen;
		HBRUSH mhBrush;
		HPEN mhOldPen;
		HBRUSH mhOldBrush;
};

class LineShape : public dxShape
{
	public:
		LineShape(const POINT u, const POINT v, const LOGPEN& lp, const LOGBRUSH& lb);
		void draw(HDC hdc);
};

class RectShape : public dxShape
{
	public:
		RectShape(const POINT u, const POINT v, const LOGPEN& lp, const LOGBRUSH& lb);
		void draw(HDC hdc);
};

class EllipseShape : public dxShape
{
	public:
		EllipseShape(const POINT u, const POINT v, const LOGPEN& lp, const LOGBRUSH& lb);
		void draw(HDC hdc);
};

class CubeShape : public dxShape
{
	public:
		CubeShape(const POINT u, const POINT v, const LOGPEN& lp, const LOGBRUSH& lb);
		void draw(HDC hdc);
};

class TextShape : public dxShape
{
	public:
		TextShape(const POINT u, const POINT v, const LOGPEN& lp, const LOGBRUSH& lb);
		void draw(HDC hdc);
};
#endif // dxShape_H