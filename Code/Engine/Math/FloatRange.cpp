#include "Engine/Math/FloatRange.hpp"

const FloatRange FloatRange::ZERO = FloatRange();
const FloatRange FloatRange::ONE = FloatRange(1.f,1.f);
const FloatRange FloatRange::ZERO_TO_ONE = FloatRange(0.f, 1.f);

FloatRange::FloatRange()
{
	m_min = 0.f;
	m_max = 0.f;
}

FloatRange::FloatRange(float a, float b)
{
	m_min = a;
	m_max = b;
}

FloatRange::~FloatRange()
{

}

bool FloatRange::IsOnRange(float const& value) const
{
	return (value >= m_min && value <= m_max);
}

bool FloatRange::IsOverlappingWith(FloatRange const& rangeToMeasure)
{
	//bool case1 = rangeToMeasure.m_min >= m_min && rangeToMeasure.m_max <= m_max;
	bool case2 = rangeToMeasure.m_min <= m_min && rangeToMeasure.m_max >= m_max;
	//bool case3 = rangeToMeasure.m_min <= m_min && rangeToMeasure.m_max >= m_min;
	bool case134 = IsOnRange(rangeToMeasure.m_min) || IsOnRange(rangeToMeasure.m_max);
	//bool case4 = rangeToMeasure.m_min >= m_min && rangeToMeasure.m_max >= m_max;
	return (case134 || case2);
}

void FloatRange::operator=(const FloatRange& RangeToAssign)
{
	m_min = RangeToAssign.m_min;
	m_max = RangeToAssign.m_max;
}
bool FloatRange::operator!=(const FloatRange& compare) const
{
	return m_min != compare.m_min || m_max != compare.m_max;
}

bool FloatRange::operator==(const FloatRange& compare) const
{
	return m_min == compare.m_min && m_max == compare.m_max;
}



