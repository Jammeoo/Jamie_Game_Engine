#include "Engine/Math/Plane2D.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
Plane2D::Plane2D(Vec2 normal, float dist) :m_normal(normal), m_distFromOrigin(dist) 
{

}

Plane2D::Plane2D(Vec2 startPoint, Vec2 endPoint)
{
	Vec2 sToPNormal = (endPoint - startPoint).GetNormalized();
	if (sToPNormal == Vec2(0.f, 0.f)) 
	{
		ERROR_AND_DIE("End point equals Start Point.");
	}
	m_normal = sToPNormal.GetRotatedMinus90Degrees();
	m_distFromOrigin = DotProduct2D(startPoint, m_normal);

}

bool Plane2D::IsPointOnPlane(Vec2 point)
{
	if (DotProduct2D(m_normal, point) == m_distFromOrigin)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool Plane2D::IsPointInsidePlant2D(Vec2 point)
{
	if (DotProduct2D(m_normal, point) > m_distFromOrigin)
	{
		return false;
	}
	else
	{
		return true;
	}
}

