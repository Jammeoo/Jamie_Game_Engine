#include "Engine/Math/ConvexHull3D.hpp"

ConvexHull3D::ConvexHull3D(std::vector<Plane3D*> planes)
{
	for (int i = 0; i < (int)planes.size(); i++) 
	{
		m_planes.push_back(planes[i]);
	}
}

bool ConvexHull3D::IsPointInside(Vec3 const& point)
{
	for (int i = 0; i < (int)m_planes.size(); i++)
	{
		Plane3D* plane = m_planes[i];
		if (!plane->IsPointInsidePlant3D(point))
		{
			return false;
		}
	}
	return true;
}
