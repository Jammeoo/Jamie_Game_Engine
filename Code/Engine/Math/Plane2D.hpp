#pragma once
#include "Engine/Math/Vec2.hpp"
struct Plane2D 
{
public:
	Plane2D(Vec2 normal, float dist);
	Plane2D(Vec2 startPoint, Vec2 endPoint);
	Plane2D() = default;
	bool IsPointOnPlane(Vec2 point);
	bool IsPointInsidePlant2D(Vec2 point);
public:
	Vec2 m_normal = Vec2::ZERO;
	float m_distFromOrigin = 0.f;

};
