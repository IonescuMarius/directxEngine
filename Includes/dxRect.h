/****************************************************************************
*                                                                           *
* dxRect.h -- RECT                                                          *
*                                                                           *
* Copyright (c) Ionescu Marius. All rights reserved(2010).                  *
*                                                                           *
****************************************************************************/

#ifndef RECT_H
#define RECT_H

#include "..\Includes\dxCircle.h"

class dxRect
{
	public:
		dxRect();
		dxRect(const dxVec2D& a, const dxVec2D& b);
		dxRect(float x0, float y0, float x1, float y1);
		void forceInside(dxCircle& A);
		bool isPtInside(const dxVec2D& pt);
		dxVec2D minPt;
		dxVec2D maxPt;
};

#endif // RECT_H