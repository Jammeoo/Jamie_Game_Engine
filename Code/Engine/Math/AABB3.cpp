#include "Engine/Math/AABB3.hpp"

AABB3::~AABB3()
{

}

AABB3::AABB3(AABB3 const& copyFrom)
{
	this->m_mins = copyFrom.m_mins;
	this->m_maxs = copyFrom.m_maxs;
}

AABB3::AABB3(float minX, float minY, float minZ, float maxX, float maxY, float maxZ)
{
	m_mins = Vec3(minX, minY, minZ);
	m_maxs = Vec3(maxX, maxY, maxZ);
}

AABB3::AABB3(Vec3 const& mins, Vec3 const& maxs)
{
	m_mins = mins;
	m_maxs = maxs;
}

Vec3 const AABB3::GetDimensions() const
{
	return Vec3(m_maxs.x - m_mins.x, m_maxs.y - m_mins.y, m_maxs.z - m_mins.z);
}

Vec3 AABB3::GetCenter() const
{
	return (m_mins + m_maxs) * 0.5f;
}
