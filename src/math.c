#include "math.h"

#include <math.h>

const float PI = 3.14159265f;

float DegToRad(float deg)
{
	const float f = PI / 180.0f;
	return deg * f;
}

float RadToDeg(float rad)
{
	const float f = 180.0f / PI;
	return rad * f;
}

void Vec4_DivI(struct Vec4* v, float f)
{
	v->x /= f;
	v->y /= f;
	v->z /= f;
	v->w /= f;
}

struct Vec4 Mat4_MulVec(const struct Mat4* m, const struct Vec4* v)
{
	struct Vec4 out;
	out.x = m->m00 * v->x + m->m01 * v->y + m->m02 * v->z + m->m03 * v->w;
	out.y = m->m10 * v->x + m->m11 * v->y + m->m12 * v->z + m->m13 * v->w;
	out.z = m->m20 * v->x + m->m21 * v->y + m->m22 * v->z + m->m23 * v->w;
	out.w = m->m30 * v->x + m->m31 * v->y + m->m32 * v->z + m->m33 * v->w;
	return out;
}

struct Mat4 M_CreatePerspective(float fovy, float aspect, float zNear, float zFar)
{
	const float f = 1.0f / tan(0.5f * fovy);
	const float d = 1.0f / (zNear - zFar );

	struct Mat4 m =
	{
		f / aspect, 0.0f, 0.0f, 0.0f,
		0.0f, f, 0.0f, 0.0f,
		0.0f, 0.0f, d * (zFar + zNear), d * (2.0f * zFar * zNear),
		0.0f, 0.0f, -1.0f, 0.0f
	};
	return m;
}

struct Mat4 M_CreateTranslation(float x, float y, float z)
{
	struct Mat4 m =
	{
		1.0f, 0.0f, 0.0f, x,
		0.0f, 1.0f, 0.0f, y,
		0.0f, 0.0f, 1.0f, z,
		0.0f, 0.0f, 0.0f, 1.0f,
	};
	return m;
}
