#define _USE_MATH_DEFINES
#include <math.h>
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Core/StringUtils.hpp"
const IntVec2 IntVec2::ZERO = IntVec2(0, 0);
const IntVec2 IntVec2::ONE = IntVec2(1, 1);
const IntVec2 IntVec2::NEGTIVEONE = IntVec2(-1, -1);
IntVec2::IntVec2(const IntVec2& copy)
	: x(copy.x)
	, y(copy.y)
{
	
}
IntVec2::IntVec2(int initialX, int initialY)
	: x(initialX)
	, y(initialY)
{
}



float IntVec2::GetLength() const {
	return sqrtf((float)x * (float)x + (float)y * (float)y);
}
int IntVec2::GetTaxicabLength() const {
	if (x < 0 && y > 0) {
		return -x + y;
	}
	else if (x < 0 && y < 0) {
		return -x - y;
	}
	else if (x > 0 && y < 0) {
		return x - y;
	}
	else
	{
		return x + y;
	}
}
int IntVec2::GetLengthSquared() const {
	return (x * x + y * y);
}
float IntVec2::GetOrientationRadians() const {
	return float(atan2(float(y), float(x)));
}
float IntVec2::GetOrientationDegrees() const {

	return float(atan2(float(y), float(x))) * (180.f / float(M_PI));
}
IntVec2 const IntVec2::GetRotated90Degrees() const {
	return IntVec2(-y, x);
}
IntVec2 const IntVec2::GetRotatedMinus90Degrees() const {
	return IntVec2(y, -x);
}
void IntVec2::Rotate90Degrees()  {
	IntVec2 temp_IntVec2(-y, x);
	x = temp_IntVec2.x;
	y = temp_IntVec2.y;
}
void IntVec2::RotateMinus90Degrees()  {
	IntVec2 temp_IntVec2(y, -x);

	x = temp_IntVec2.x;
	y = temp_IntVec2.y;
}


void IntVec2::SetFromText(char const* text)
{
	Strings originalText = SplitStringOnDelimiter(text, ',');
	x = atoi(originalText[0].c_str());
	y = atoi(originalText[1].c_str());
}

void IntVec2::operator+(const IntVec2& b)
{
	x += b.x;
	y += b.y;
}

bool IntVec2::operator!=(const IntVec2& copyFrom) const
{
	return (x != copyFrom.x || y != copyFrom.y);
}

bool IntVec2::operator<(const IntVec2& comparedAgainst) const
{
	if (y < comparedAgainst.y)
	{
		return true;
	}
	else if (comparedAgainst.y < y) 
	{
		return false;
	}
	else
	{
		return x < comparedAgainst.x;
	}
}

bool IntVec2::operator==(const IntVec2& copyFrom) const
{
	return (x == copyFrom.x && y == copyFrom.y);
}


void IntVec2::operator=(const IntVec2& copyFrom) {
	x = copyFrom.x;
	y = copyFrom.y;
}