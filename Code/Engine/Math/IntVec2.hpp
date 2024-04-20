#pragma once
#include "MathUtils.hpp"
//------------------------------------------------------------------------------------------------------------------------------------------------------------------

struct IntVec2 {
public:
	int x = 0;
	int y = 0;
public:
	//Construction/Destruction
	~IntVec2() {};										//destructor(do nothing)
	IntVec2() {};										//default constructor(do nothing)
	IntVec2(const IntVec2& copyFrom);					//copy constructor(from another IntVec2)
	explicit IntVec2(int initialX, int initialY);		//explicit constructor(from x,y)

	static const IntVec2 ZERO;
	static const IntVec2 ONE;
	static const IntVec2 NEGTIVEONE;

	//Accessors(const methods)
	float   GetLength() const;
	int		GetTaxicabLength() const;
	int		GetLengthSquared() const;
	float   GetOrientationRadians() const;
	float   GetOrientationDegrees() const;
	IntVec2 const GetRotated90Degrees() const;
	IntVec2 const GetRotatedMinus90Degrees() const;

	//Mutators(non-const methods)
	void   Rotate90Degrees();
	void   RotateMinus90Degrees();
	void   SetFromText(char const* text);
	//Operators(self-mutating/non-const)
	void operator+(const IntVec2& b);
	
	void operator=(const IntVec2& copyFrom);              //IntVec2=IntVec2
	bool operator==(const IntVec2& copyFrom)const; 
	bool operator!=(const IntVec2& copyFrom)const;
	bool operator<(const IntVec2& comparedAgainst)const;
	//bool operator<(const IntVec2& comparedAgainst)const;
};