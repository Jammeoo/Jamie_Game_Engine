#include "Engine/Math/LineSegment2.hpp"

void LineSegment2::Translate(Vec2 translation)
{
	m_start += translation;
	m_end += translation;
}

void LineSegment2::SetCenter(Vec2 newCenter)
{
	Vec2 currentCenter = 0.5f * (m_end - m_start);
	Vec2 displace = newCenter - currentCenter;
	Translate(displace);
}

void LineSegment2::RotateAboutCenter(float rotationDeltaDegrees)
{
	//m_start remain the same change the m_end only
	Vec2 displacement =m_end - m_start;
	displacement.RotateDegrees(rotationDeltaDegrees);
	m_end = displacement + m_start;
}
