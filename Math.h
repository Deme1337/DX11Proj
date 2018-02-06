#pragma once

#ifndef MATH_H
#define MATH_H

// Constants
const float Pi = 3.141592654f;
const float Pi2 = 6.283185307f;
const float Pi_2 = 1.570796327f;
const float Pi_4 = 0.7853981635f;
const float InvPi = 0.318309886f;
const float InvPi2 = 0.159154943f;

// Scale factor used for storing physical light units in fp16 floats (equal to 2^-10).
const float FP16Scale = 0.0009765625f;


inline float clamp(float x, float a, float b)
{
	return x < a ? a : (x > b ? b : x);
}


inline float GetDegrees(float rad)
{
	return rad * 180.0f / Pi;
}


struct INT4
{
	int x, y, z, w;

	INT4()
	{

	}

	INT4(const XMFLOAT4 &x)
	{
		this->x = x.x;
		this->y = x.y;
		this->z = x.z;
		this->w = x.w;
	}
};


#endif // !MATH_H
