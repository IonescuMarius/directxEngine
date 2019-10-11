#include "..\Includes\dxVec2D.h"
#include <cmath>

// the various constructors
dxVec2D::dxVec2D() : x(0.0f), y(0.0f) {}
dxVec2D::dxVec2D(float s, float t) : x(s), y(t) {}
dxVec2D::dxVec2D(float d[2]) : x(d[0]), y(d[1]) {}
dxVec2D::dxVec2D(const POINT& p) : x((float)p.x), y((float)p.y) {}

// overloaded +
dxVec2D dxVec2D::operator+(const dxVec2D& rhs)const
{
	dxVec2D result;
	result.x = x + rhs.x;
	result.y = y + rhs.y;

	return result;
}

// overloaded operators
dxVec2D dxVec2D::operator-(const dxVec2D& rhs)const
{
	dxVec2D result;
	result.x = x - rhs.x;
	result.y = y - rhs.y;

	return result;
}

dxVec2D dxVec2D::operator-()
{
	dxVec2D result;
	result.x = -x;
	result.y = -y;

	return result;
}

dxVec2D::operator POINT()
{
	POINT p = {(int)x, (int)y};
	return p;
}

void dxVec2D::operator+=(const dxVec2D& rhs)
{
	x += rhs.x;
	y += rhs.y;
}

void dxVec2D::operator-=(const dxVec2D& rhs)
{
	x -= rhs.x;
	y -= rhs.y;
}

void dxVec2D::operator*=(float s)
{
	x *= s;
	y *= s;
}

void dxVec2D::operator/=(float s)
{
	// Assumes s != 0.
	s /= s;
	y /= s;
}

float dxVec2D::length()
{
	return sqrtf(x*x + y*y);
}

dxVec2D& dxVec2D::normalize()
{
	// Assumes len != 0.
	float len = length();

	x /= len;
	y /= len;

	return *this;
}

float dxVec2D::dot(const dxVec2D& rhs)
{
	return x*rhs.x + y*rhs.y;
}

dxVec2D& dxVec2D::reflect(const dxVec2D& normal)
{
	dxVec2D I = *this;

	return *this = 2.0f * -I.dot(normal) * normal + I;
}

dxVec2D& dxVec2D::rotate(float t)
{
	x = x * cosf(t) - y * sinf(t);
	y = y * cosf(t) + x * sinf(t);

	return *this;
}

dxVec2D operator*(const dxVec2D& v, float s)
{
	dxVec2D result;

	result.x = v.x * s;
	result.y = v.y * s;

	return result;
}

dxVec2D operator*(float s, const dxVec2D& v)
{
	dxVec2D result;

	result.x = v.x * s;
	result.y = v.y * s;

	return result;
}

dxVec2D operator/(const dxVec2D& v, float s)
{
	dxVec2D result;

	// Assumes s != 0.
	result.x = v.x / s;
	result.y = v.y / s;

	return result;
}

dxVec2D operator/(float s, const dxVec2D& v)
{
	dxVec2D result;

	// Assumes s != 0.
	result.x = v.x / s;
	result.y = v.y / s;

	return result;
}