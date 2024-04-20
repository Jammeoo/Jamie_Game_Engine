#include "Engine/Math/IntRange.hpp"
const IntRange IntRange::ZERO = IntRange(0, 0);
const IntRange IntRange::ONE = IntRange(1, 1);
const IntRange IntRange::ZERO_TO_ONE = IntRange(0, 1);
IntRange::IntRange()
{
	m_min = 0;
	m_max = 0;
}

IntRange::IntRange(int a, int b)
{
	m_min = a;
	m_max = b;
}

IntRange::~IntRange()
{

}

bool IntRange::IsOnRange(int const& value) const
{
	return (value >= m_min && value <= m_max);
}

bool IntRange::IsOverlappingWith(IntRange const& rangeToMeasure)
{
	//bool case1 = rangeToMeasure.m_min >= m_min && rangeToMeasure.m_max <= m_max;
	bool case2 = rangeToMeasure.m_min <= m_min && rangeToMeasure.m_max >= m_max;
	//bool case3 = rangeToMeasure.m_min <= m_min && rangeToMeasure.m_max >= m_min;
	bool case134 = IsOnRange(rangeToMeasure.m_min) || IsOnRange(rangeToMeasure.m_max);
	//bool case4 = rangeToMeasure.m_min >= m_min && rangeToMeasure.m_max >= m_max;
	return (case134 || case2);
}

void IntRange::operator=(const IntRange& RangeToAssign)
{
	m_min = RangeToAssign.m_min;
	m_max = RangeToAssign.m_max;
}
bool IntRange::operator==(const IntRange& compare) const
{
	return m_min == compare.m_min || m_max == compare.m_max;
}


bool IntRange::operator!=(const IntRange& compare) const
{
	return m_min != compare.m_min || m_max != compare.m_max;
}

