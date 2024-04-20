#pragma once
#include <vector>
#include "Engine/Math/Vec3.hpp"

struct  ConvexPoly3D
{
public:
	ConvexPoly3D() = default;
	ConvexPoly3D(std::vector<Vec3>const& ccwOrderedPoints);
	void AddPointsForConvexPoly3D(Vec3 const& point);
	bool IsPointInside(Vec3 const& point);
	std::vector<Vec3> GetPoints()const;
private:
	std::vector<Vec3> m_points;
};