#include "Engine/Math/Vec4.hpp"


Vec4::Vec4(const Vec4& copyFrom)
{
	x = copyFrom.x;
	y = copyFrom.y;
	z = copyFrom.z;
	w = copyFrom.w;
}

const Vec4 Vec4::operator-(const Vec4& vecToSubtract) const
{
	return Vec4(x - vecToSubtract.x, y - vecToSubtract.y, z - vecToSubtract.z, w - vecToSubtract.w);
}

void Vec4::operator*=(const float& scale)
{
	x = x * scale;
	y = y * scale;
	z = z * scale;
	w = w * scale;
}

void Vec4::operator*=(const Vec4& vecToMultiply)
{
	x = x * vecToMultiply.x;
	y = y * vecToMultiply.y;
	z = z * vecToMultiply.z;
	w = w * vecToMultiply.w;
}



