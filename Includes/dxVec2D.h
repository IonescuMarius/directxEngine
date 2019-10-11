/****************************************************************************
*                                                                           *
* dxVec2D.h -- VECTOR 2D                                                    *
*                                                                           *
* Copyright (c) Ionescu Marius. All rights reserved(2010).                  *
*                                                                           *
****************************************************************************/

#ifndef VEC2_H
#define VEC2_H

#include <windows.h>

class dxVec2D
{
	public:
		dxVec2D();
		dxVec2D(float s, float t);
		dxVec2D(float d[2]);
		dxVec2D(const POINT& p);

		dxVec2D operator+(const dxVec2D& rhs)const;
		dxVec2D operator-(const dxVec2D& rhs)const;
		dxVec2D operator-();

		operator POINT();

		void operator+=(const dxVec2D& rhs);
		void operator-=(const dxVec2D& rhs);
		void operator*=(float s);
		void operator/=(float s);

		float length();
		dxVec2D& normalize();

		float dot(const dxVec2D& rhs);

		dxVec2D& reflect(const dxVec2D& normal);

		dxVec2D& rotate(float t);

		// Data members.
		float x;
		float y;
};

dxVec2D operator*(const dxVec2D& v, float s);
dxVec2D operator*(float s, const dxVec2D& v);
dxVec2D operator/(const dxVec2D& v, float s);
dxVec2D operator/(float s, const dxVec2D& v);

#endif // VEC2_H