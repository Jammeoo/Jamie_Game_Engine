#pragma once
#include <vector>
#include "Engine/Math/Plane2D.hpp"
#include "Engine/Math/ConvexPoly2D.hpp"
struct ConvexHull2D 
{
public:
	ConvexHull2D(std::vector<Plane2D> planes);
	ConvexHull2D(ConvexPoly2D const& poly);
	bool IsPointInside(Vec2 point);
	std::vector<Plane2D> GetPlanes()const;
public:
	std::vector<Plane2D> m_planes;
};
