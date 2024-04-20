#include "Engine/Math/Plane3D.hpp"
#include "Engine/Math/MathUtils.hpp"
Plane3D::Plane3D(Vec3 const& normal, float dist) :m_normal(normal), m_distFromOrigin(dist) 
{

}

bool Plane3D::IsPointOnPlane(Vec3 const& point)
{
	if (DotProduct3D(m_normal, point) == m_distFromOrigin)
	{
		return false;
	}
	else
	{
		return true;
	}
}

bool Plane3D::IsPointInsidePlant3D(Vec3 const& point)
{
	if (DotProduct3D(m_normal, point) > m_distFromOrigin)
	{
		return false;
	}
	else
	{
		return true;
	}
}

