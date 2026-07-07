#include "vec2.h"

vec2::vec2(void)
: x(0)
, y(0)
{
}
vec2::vec2(float _x,float _y)
: x(_x)
, y(_y)
{
}

vec2::~vec2(void)
{
}

vec2 vec2::operator+(const vec2& v)
{
	return vec2(x+v.x,y+v.y);
}

vec2 vec2::operator-(void)
{
	return vec2(-x,-y);
}

vec2 vec2::operator-(const vec2& v)
{
	return vec2(x-v.x,y-v.y);
}

float vec2::Length(void)
{
	return sqrt(x*x+y*y);
}

vec2 vec2::operator*(float s)
{
	return vec2(x*s,y*s);
}

vec2 vec2::operator+=(const vec2& v)
{
	x+=v.x;
	y+=v.y;
	return *this;
}
