/****************************************************************************
*                                                                           *
* dxCircle.h -- CIRCLE                                                      *
*                                                                           *
* Copyright (c) Ionescu Marius. All rights reserved(2010).                  *
*                                                                           *
****************************************************************************/

#ifndef CIRCLE_H
#define CIRCLE_H

#include "dxVec2D.h"

class dxCircle
{
	public:
		dxCircle();
		dxCircle(float R, const dxVec2D& center);

		bool hits(dxCircle& A, dxVec2D& normal);

		float r; // radius
		dxVec2D  c; // center point
};
#endif 