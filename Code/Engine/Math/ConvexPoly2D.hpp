#pragma once
#include <vector>
#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
struct  ConvexPoly2D
{
public:
	ConvexPoly2D() = default;
	ConvexPoly2D(std::vector<Vec2>const& ccwOrderedPoints);
	~ConvexPoly2D() 
	{
		m_points.clear();
	}
	void AddPointsForConvexPoly2D(Vec2 point);
	bool IsPointInside(Vec2 point);
	std::vector<Vec2> GetPoints()const;
	void UpdateMyPoints(std::vector<Vec2>const& points);
private:
	std::vector<Vec2> m_points;

};