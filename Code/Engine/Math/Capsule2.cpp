#include "Engine/Math/Capsule2.hpp"

Capsule2::Capsule2(Vec2 start, Vec2 end, float radius)
{
	m_start = start;
	m_end = end;
	m_radius = radius;
}

void Capsule2::Translate(Vec2 translation)
{
	m_start += translation;
	m_end += translation;
}

void Capsule2::SetCenter(Vec2 newCenter)
{
	Vec2 currentCenter = 0.5f * (m_end - m_start);
	Vec2 displace = newCenter - currentCenter;
	Translate(displace);
}

void Capsule2::RotateAboutCenter(float rotationDeltaDegrees)
{
	Vec2 displacement = m_end - m_start;
	displacement.RotateDegrees(rotationDeltaDegrees);
	m_end = displacement + m_start;
}
