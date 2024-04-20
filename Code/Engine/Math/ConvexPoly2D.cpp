#include "Engine/Math/ConvexPoly2D.hpp"
#include "Engine/Math/Plane2D.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
ConvexPoly2D::ConvexPoly2D(std::vector<Vec2>const& ccwOrderedPoints)
{
	if (!m_points.empty()) 
	{
		ERROR_AND_DIE("New points try to hide the existed points.");
	}
	if (ccwOrderedPoints.empty())
	{
		ERROR_AND_DIE("Try to initialize a Convex Poly with an empty point array!");
	}
	for (int i = 0; i < (int)ccwOrderedPoints.size(); i++) 
	{
		Vec2 point = ccwOrderedPoints[i];
		m_points.push_back(point);
	}
}

void ConvexPoly2D::AddPointsForConvexPoly2D(Vec2 point)
{
	m_points.push_back(point);
}

bool ConvexPoly2D::IsPointInside(Vec2 point)
{
	if ((int)m_points.size() < 2) 
	{
		return false;//if point num equal one, false
	}

	for (int i = 1; i < (int)m_points.size(); i++) 
	{
		Vec2 firstPoint = m_points[i - 1];
		Vec2 secondPoint = m_points[i];

		Vec2 disp = secondPoint - firstPoint;
		Vec2 sToFNorm = disp.GetNormalized();
		if (disp.GetLengthSquared() == 0.f)
		{
			continue;
		}
		//get normal
		Vec2 curNormal = sToFNorm.GetRotatedMinus90Degrees();
		//Pick one 0point
		//normal * 0point == D(because 0 is on the plane)
		float d = DotProduct2D(curNormal, firstPoint);
		//point * normal - D > 0 return false
		float curResult = DotProduct2D(curNormal, point) - d;
		if (curResult > 0.f) 
		{
			return false;
		}
	}
	return true;
}

std::vector<Vec2> ConvexPoly2D::GetPoints() const
{
	return m_points;
}

void ConvexPoly2D::UpdateMyPoints(std::vector<Vec2>const& points)
{
	for (int i = 0; i < (int)m_points.size(); i++) 
	{
		m_points[i] = points[i];
	}
}

