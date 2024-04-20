#pragma once

struct IntVec3 {
public:
	int x = 0;
	int y = 0;
	int z = 0;
public:
	//Construction/Destruction
	~IntVec3() {};										//destructor(do nothing)
	IntVec3() {};										//default constructor(do nothing)
	IntVec3(const IntVec3& copyFrom);					//copy constructor(from another IntVec2)
	explicit IntVec3(int initialX, int initialY, int initialZ);		//explicit constructor(from x,y)

	static const IntVec3 ZERO;
	static const IntVec3 ONE;

	//Accessors(const methods)
// 	float   GetLength() const;
// 	int		GetTaxicabLength() const;
// 	int		GetLengthSquared() const;


	//Mutators(non-const methods)
// 	void   Rotate90Degrees();
// 	void   RotateMinus90Degrees();
// 	void   SetFromText(char const* text);

	//Operators(self-mutating/non-const)
	void operator=(const IntVec3& copyFrom);              //IntVec2=IntVec2
	IntVec3 operator-(const IntVec3& minusFrom);              //IntVec2=IntVec2
	IntVec3 operator+(const IntVec3& minusFrom);
};