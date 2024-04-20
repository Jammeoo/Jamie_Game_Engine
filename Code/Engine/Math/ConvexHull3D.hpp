#pragma once
#include <vector>
#include "Engine/Math/Plane3D.hpp"
struct ConvexHull3D 
{
public:
	ConvexHull3D(std::vector<Plane3D*> planes);
	bool IsPointInside(Vec3 const& point);
public:
	std::vector<Plane3D*> m_planes;
};
