#pragma once
#include "Engine/Math/Vec3.hpp"
struct Plane3D 
{
public:
	Plane3D(Vec3 const& normal, float dist);
	bool IsPointOnPlane(Vec3 const& point);
	bool IsPointInsidePlant3D(Vec3 const& point);
public:
	Vec3 m_normal = Vec3::WORLD_ORIGIN;
	float m_distFromOrigin = 0.f;

};
