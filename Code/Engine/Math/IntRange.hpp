#pragma once

struct IntRange
{
public:
	int m_min = 0;
	int m_max = 0;
public:
	//Construction/Destruction
	IntRange();
	IntRange(int a, int b);
	~IntRange();

	//Accessors(const methods)
	bool IsOnRange(int const& value)const;
	bool IsOverlappingWith(IntRange const& rangeToMeasure);

	// Operators (const)
	void	operator=(const IntRange& RangeToAssign);
	bool	operator==(const IntRange& compare) const;
	bool	operator!=(const IntRange& compare) const;

	static const IntRange ZERO;
	static const IntRange ONE;
	static const IntRange ZERO_TO_ONE;
};
