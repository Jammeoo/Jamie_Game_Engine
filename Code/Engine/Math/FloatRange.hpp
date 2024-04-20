#pragma once

struct FloatRange
{
public:
	float m_min = 0.f;
	float m_max = 0.f;
public:
	//Construction/Destruction
	FloatRange();
	FloatRange(float a, float b);
	~FloatRange();

	//Accessors(const methods)
	bool IsOnRange(float const& value)const;
	bool IsOverlappingWith(FloatRange const& rangeToMeasure);
	float GetDeltaRange()const { return (m_max - m_min); }
	// Operators (const)
	void	operator=(const FloatRange& RangeToAssign);
	bool	operator==(const FloatRange& compare) const;
	bool	operator!=(const FloatRange& compare) const;

	static const FloatRange ZERO;
	static const FloatRange ONE;
	static const FloatRange ZERO_TO_ONE;
};
