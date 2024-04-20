#pragma once
#include "Engine/Math/Vec2.hpp"
struct Capsule2
{
public:
	Vec2 m_start = Vec2(0.f, 0.f);
	Vec2 m_end = Vec2(0.f, 0.f);
	float m_radius = 0.f;
public:
	Capsule2() {};
	Capsule2(Vec2 start, Vec2 end, float radius);
	void Translate(Vec2 translation);
	void SetCenter(Vec2 newCenter);
	void RotateAboutCenter(float rotationDeltaDegrees);
};