#pragma once
#include <math.h>
class vec2
{
public:
	vec2(float _x,float _y);
	vec2(void);
	~vec2(void);
	float x;
	float y;
	vec2 operator+(const vec2& v);
	vec2 operator-(void);
	vec2 operator-(const vec2& v);
	float Length(void);
	vec2 operator*(float s);
	vec2 operator+=(const vec2& v);
};
