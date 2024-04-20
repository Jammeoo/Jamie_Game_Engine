#include"Engine/Math/AABB2.hpp"

const AABB2 AABB2::ZERO_TO_ONE = AABB2(Vec2(0.f, 0.f), Vec2(1.f, 1.f));

AABB2::AABB2(AABB2 const& copyFrom)
{
	m_mins = copyFrom.m_mins;
	m_maxs = copyFrom.m_maxs;
}

AABB2::AABB2(float minX, float minY, float maxX, float maxY)
{
	m_mins.x = minX;
	m_mins.y= minY;
	m_maxs.x = maxX;
	m_maxs.y = maxY;
}

AABB2::AABB2(Vec2 const& mins, Vec2 const& maxs)
{
	m_mins = mins;
	m_maxs = maxs;
}

bool AABB2::IsPointInside(Vec2 const& point) const
{
	if (point.x > m_mins.x && point.x < m_maxs.x) {
		if (point.y > m_mins.y && point.y < m_maxs.y) {
			return true;
		}
		else return false;
	}
	else return false;
}

Vec2 const AABB2::GetCenter() const
{
	return Vec2((m_maxs.x - m_mins.x) * 0.5f + m_mins.x, (m_maxs.y - m_mins.y) * 0.5f + m_mins.y);
}

Vec2 const AABB2::GetDimensions() const
{
	return Vec2(m_maxs.x - m_mins.x, m_maxs.y - m_mins.y);
}

Vec2 const AABB2::GetNearestPoint(Vec2 const& referencePosition) const
{
	//Get Clamp to X and Y which is much easier
	if (referencePosition.x<m_mins.x&& referencePosition.y<m_mins.y)
	{
		return Vec2(m_mins.x, m_mins.y);
	}
	else if (referencePosition.x<m_mins.x&&m_mins.y<= referencePosition.y&& referencePosition.y<=m_maxs.y)
	{
		return Vec2(m_mins.x, referencePosition.y);
	}
	else if (referencePosition.x<m_mins.x&& referencePosition.y>m_maxs.y)
	{
		return Vec2(m_mins.x, m_maxs.y);
	}
	else if (referencePosition.x>= m_mins.x&&m_maxs.x>= referencePosition.x&& referencePosition.y<m_mins.y)
	{
		return Vec2(referencePosition.x, m_mins.y);
	}
	else if (referencePosition.x >= m_mins.x && m_maxs.x >= referencePosition.x&& m_mins.y <= referencePosition.y && referencePosition.y <= m_maxs.y)
	{
		return referencePosition;
	}
	else if (referencePosition.x >= m_mins.x && m_maxs.x >= referencePosition.x&& referencePosition.y > m_maxs.y)
	{
		return Vec2(referencePosition.x, m_maxs.y);
	}
	else if (referencePosition.x > m_maxs.x&& referencePosition.y > m_maxs.y)
	{
		return m_maxs;
	}
	else if (m_mins.y <= referencePosition.y && referencePosition.y <= m_maxs.y&&referencePosition.x>m_maxs.x)
	{
		return Vec2(m_maxs.x, referencePosition.y);
	}
	else
	{
		return Vec2(m_maxs.x, m_mins.y);
	}
}

Vec2 const AABB2::GetPointAtUV(Vec2 const& uv) const
{
	return Vec2(((m_maxs.x - m_mins.x) * uv.x + m_mins.x), ((m_maxs.y - m_mins.y) * uv.y + m_mins.y));
}

Vec2 const AABB2::GetUVForPoint(Vec2 const& point) const
{
	return Vec2(((point.x - m_mins.x) / (m_maxs.x - m_mins.x)), ((point.y - m_mins.y) / (m_maxs.y - m_mins.y)));
}

AABB2 const AABB2::GetBoxWithin(Vec2 uvMins, Vec2 uvMaxs) const
{
	Vec2 mins = GetPointAtUV(uvMins);
	Vec2 maxs = GetPointAtUV(uvMaxs);
	return AABB2(mins, maxs);
}

void AABB2::Translate(Vec2 const& translationToApply)
{
	m_mins.x += translationToApply.x;
	m_mins.y += translationToApply.y;
	m_maxs.x += translationToApply.x;
	m_maxs.y += translationToApply.y;
}

void AABB2::SetCenter(Vec2 const& newCenter)
{
	Vec2 oldCenter = GetCenter();
	Translate(newCenter - oldCenter);
}

void AABB2::SetDimensions(Vec2 const& newDimensions)
{
	Vec2 currentCenter = GetCenter();
	m_mins.x = currentCenter.x - newDimensions.x * 0.5f;
	m_mins.y = currentCenter.y - newDimensions.y * 0.5f;
	m_maxs.x = currentCenter.x + newDimensions.x * 0.5f;
	m_maxs.y = currentCenter.y + newDimensions.y * 0.5f;

}

void AABB2::StretchToIncludePoint(Vec2 const& point)
{
	if (point.x < m_mins.x && point.y < m_mins.y)
	{
		m_mins = point;
	}
	else if (point.x < m_mins.x && m_mins.y <= point.y && point.y <= m_maxs.y)
	{
		m_mins.x = point.x;
	}
	else if (point.x<m_mins.x && point.y>m_maxs.y)
	{
		m_mins.x = point.x;
		m_maxs.y = point.y;
	}
	else if (point.x >= m_mins.x && m_maxs.x >= point.x && point.y < m_mins.y)
	{
		m_mins.y = point.y;
	}
	else if (point.x >= m_mins.x && m_maxs.x >= point.x && m_mins.y <= point.y && point.y <= m_maxs.y){}
	else if (point.x >= m_mins.x && m_maxs.x >= point.x && point.y > m_maxs.y)
	{
		m_maxs.y = point.y;
	}
	else if (point.x > m_maxs.x && point.y > m_maxs.y)
	{
		m_maxs = point;
	}
	else if (m_mins.y <= point.y && point.y <= m_maxs.y && point.x > m_maxs.x)
	{
		m_maxs.x = point.x;
	}
	else
	{
		m_mins.y = point.y;
		m_maxs.x = point.x;
	}
}
