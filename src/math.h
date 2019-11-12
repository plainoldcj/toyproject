#pragma once

float DegToRad(float deg);
float RadToDeg(float rad);

// 4x4-matrix. Stored row-major.
struct Mat4
{
	float m00, m01, m02, m03;
	float m10, m11, m12, m13;
	float m20, m21, m22, m23;
	float m30, m31, m32, m33;
};

struct Vec3
{
	float x;
	float y;
	float z;
};

struct Vec4
{
	float x;
	float y;
	float z;
	float w;
};

void Vec4_DivI(struct Vec4* v, float f);

struct Vec4 Mat4_MulVec(const struct Mat4* m, const struct Vec4* v);

struct Mat4 M_CreatePerspective(float fovy, float aspect, float nearZ, float farZ);
