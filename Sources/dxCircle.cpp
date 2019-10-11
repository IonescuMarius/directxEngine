#include "..\Includes\dxCircle.h"

dxCircle::dxCircle() : r(0.0f), c(0.0f, 0.0f) 
{

}

dxCircle::dxCircle(float R, const dxVec2D& center) : r(R), c(center) 
{

}

bool dxCircle::hits(dxCircle& A, dxVec2D& normal)
{
	dxVec2D u = A.c - c;

	if( u.length() <= r + A.r )
	{
		normal = u.normalize();

		A.c = c + (normal * (r + A.r));

		return true;
	}
	return false;
}