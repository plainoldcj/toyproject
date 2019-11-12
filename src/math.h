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

// Creates a perspective projection matrix that transforms points from camera space to
// normalized device space (NDC). More precisely:
// Let P the projection matrix and v = Pu = (vx, vy, vz, vw).
// Then v/vw = (vx / vw, vy / vw, vz / vw, 1) is the point u in NDC.
// Here, NDC is a cube [-1,-1,-1]x[1,1,1] around the origin with left-handed coordinate space.
// Camera space is right-handed with the z-axis going out of the screen.
struct Mat4 M_CreatePerspective(float fovy, float aspect, float nearZ, float farZ);
