#include "Engine/Math/ConvexHull2D.hpp"

ConvexHull2D::ConvexHull2D(std::vector<Plane2D> planes)
{
	for (int i = 0; i < (int)planes.size(); i++) 
	{
		m_planes.push_back(planes[i]);
	}
}

ConvexHull2D::ConvexHull2D(ConvexPoly2D const& poly)
{
	std::vector<Vec2> point = poly.GetPoints();
	for (int i = 1; i < (int)point.size(); i++)
	{
		Plane2D plane = Plane2D(point[i - 1], point[i]);
		m_planes.push_back(plane);
		if (i == ((int)point.size() - 1)) 
		{
			Plane2D lastPlane = Plane2D(point[i], point[0]);
			m_planes.push_back(lastPlane);
		}
	}
}

bool ConvexHull2D::IsPointInside(Vec2 point)
{
	for (int i = 0; i < (int)m_planes.size(); i++)
	{
		Plane2D plane = m_planes[i];
		if (!plane.IsPointInsidePlant2D(point))
		{
			return false;
		}
	}
	return true;
}

std::vector<Plane2D> ConvexHull2D::GetPlanes() const
{
	return m_planes;
}
