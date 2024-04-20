#include "Engine/Math/IntVec3.hpp"

IntVec3::IntVec3(const IntVec3& copyFrom)
{
	x = copyFrom.x;
	y = copyFrom.y;
	z = copyFrom.z;
}

IntVec3::IntVec3(int initialX, int initialY, int initialZ)
{
	x = initialX;
	y = initialY;
	z = initialZ;
}

IntVec3 IntVec3::operator-(const IntVec3& minusFrom)
{
	int rX = (x - minusFrom.x);
	int rY = (y - minusFrom.y);
	int rZ = (z - minusFrom.z);


	IntVec3 result = IntVec3(rX, rY, rZ);
	return result;
}

IntVec3 IntVec3::operator+(const IntVec3& minusFrom)
{
	int rX = (x + minusFrom.x);
	int rY = (y + minusFrom.y);
	int rZ = (z + minusFrom.z);


	IntVec3 result = IntVec3(rX, rY, rZ);
	return result;
}

void IntVec3::operator=(const IntVec3& copyFrom)
{
	x = copyFrom.x;
	y = copyFrom.y;
	z = copyFrom.z;
}
