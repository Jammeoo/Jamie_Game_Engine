#include "Engine/Math/ConvexPoly3D.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
ConvexPoly3D::ConvexPoly3D(std::vector<Vec3>const& ccwOrderedPoints)
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
		Vec3 point = ccwOrderedPoints[i];
		m_points.push_back(point);
	}
}

void ConvexPoly3D::AddPointsForConvexPoly3D(Vec3 const& point)
{
	m_points.push_back(point);
}

bool ConvexPoly3D::IsPointInside(Vec3 const& point)
{
	if ((int)m_points.size() < 2)
	{
		return false;//if point num equal one, false
	}

	for (int i = 1; i < (int)m_points.size(); i++)
	{
		Vec3 firstPoint = m_points[i - 1];
		Vec3 secondPoint = m_points[i];

		Vec3 disp = secondPoint - firstPoint;
		Vec3 sToFNorm = disp.GetNormalized();
		if (disp.GetLengthSquared() == 0.f)
		{
			continue;
		}
		//get normal
		Vec3 curNormal = sToFNorm.GetRotatedAboutZDegrees(-90.f);
		//Pick one 0point
		//normal * 0point == D(because 0 is on the plane)
		float d = DotProduct3D(curNormal, firstPoint);
		//point * normal - D > 0 return false
		float curResult = DotProduct3D(curNormal, point) - d;
		if (curResult > 0.f)
		{
			return false;
		}
	}
	return true;
}

std::vector<Vec3> ConvexPoly3D::GetPoints() const
{
	return m_points;
}
