#include "..\Includes\dxRect.h"

dxRect::dxRect()
{

}

dxRect::dxRect(const dxVec2D& a, const dxVec2D& b) : minPt(a), maxPt(b)
{

}

dxRect::dxRect(float x0, float y0, float x1, float y1) : minPt(x0, y0), maxPt(x1, y1)
{

}

void dxRect::forceInside(dxCircle& A)
{
	dxVec2D p = A.c;
	float r = A.r;
	
	// Modify coordinates to force inside.
	if(p.x - r < minPt.x)
		p.x = minPt.x + r;
	if(p.x + r > maxPt.x)
		p.x = maxPt.x - r;
	if(p.y - r < minPt.y)
		p.y = minPt.y + r;
	if(p.y + r > maxPt.y)
		p.y = maxPt.y - r;
	
	// Save forced position.
	A.c = p;
}

bool dxRect::isPtInside(const dxVec2D& pt)
{
	return pt.x >= minPt.x && pt.y >= minPt.y && pt.x <= maxPt.x && pt.y <= maxPt.y;
}