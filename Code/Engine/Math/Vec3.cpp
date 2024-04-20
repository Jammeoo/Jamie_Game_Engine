#define _USE_MATH_DEFINES
#include <math.h>
#include"Vec3.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/StringUtils.hpp"

const Vec3 Vec3::X_AXIS = Vec3(1.f, 0.f, 0.f);
const Vec3 Vec3::Y_AXIS = Vec3(0.f, 1.f, 0.f);
const Vec3 Vec3::Z_AXIS = Vec3(0.f, 0.f, 1.f);
const Vec3 Vec3::WORLD_ORIGIN = Vec3(0.f, 0.f, 0.f);
//-----------------------------------------------------------------------------------------------
Vec3::Vec3(const Vec3& copy)
	: x(copy.x)
	, y(copy.y)
	, z(copy.z)
{
}


//-----------------------------------------------------------------------------------------------
//Vec3::Vec3(float initialX, float initialY, float initialZ)
//	: x(initialX)
//	, y(initialY)
//	, z(initialZ)
//{
//}

float Vec3::GetLength() const
{
	
	return sqrtf(x * x + y * y + z * z);
}

float Vec3::GetLengthXY() const
{
	return sqrtf(x * x + y * y);
}

float Vec3::GetLengthSquared() const
{
	return x * x + y * y + z * z;
}

float Vec3::GetLengthXYSquared() const
{
	return x * x + y * y;
}

float Vec3::GetAngleAboutZRadians() const
{
	return (float)atan2(y, x);
}

float Vec3::GetAngleAboutZDegrees() const
{
	return (float)atan2(y, x) * 180.f / (float)M_PI;
}

Vec3 const Vec3::GetRotatedAboutZRadians(float deltaRadians) const
{
	float degrees = ConvertRadiansToDegrees(atan2f(y, x) + deltaRadians);
	return Vec3(GetLengthXY() * CosDegrees(degrees), GetLengthXY() * SinDegrees(degrees), z);
}

Vec3 const Vec3::GetRotatedAboutZDegrees(float deltaDegrees) const
{
	float degrees = Atan2Degrees(y, x) + deltaDegrees;
	return Vec3(GetLengthXY() * CosDegrees(degrees), GetLengthXY() * SinDegrees(degrees), z);

}

Vec3 const Vec3::GetClamped(float maxLength) const
{
	if (GetLength() > maxLength) {
		float scale = maxLength / GetLength();
		return Vec3(scale * x, scale * y, scale * z);
	}
	else {
		return Vec3(x, y, z);
	}
}

Vec3 const Vec3::GetNormalized() const
{
	if (GetLength() == 0.f)  return Vec3::WORLD_ORIGIN;
	float scale = 1.f / GetLength();
	return Vec3(x * scale, y * scale, z * scale);
}


void const Vec3::Normalize()
{
	float scale = 1.f / GetLength();
	x *= scale;
	y *= scale;
	z *= scale;
}

void Vec3::SetFromText(char const* text, char delimiterToSplitOn)
{
	Strings originalText = SplitStringOnDelimiter(text, delimiterToSplitOn);
	if (originalText.size() >= 3) 
	{
		x = (float)atof(originalText[0].c_str());
		y = (float)atof(originalText[1].c_str());
		z = (float)atof(originalText[2].c_str());
	}
}

Vec3 const Vec3::MakeFromPolarRadians(float latitudeRadians, float longtitudeRadians, float length /*= 1.0f*/)
{
	float latitudeDegree = latitudeRadians * 180.f / (float)M_PI;
	float longtitudeDegree = longtitudeRadians * 180.f / (float)M_PI;
	return MakeFromPolarDegrees(latitudeDegree, longtitudeDegree, length);
}

Vec3 const Vec3::MakeFromPolarDegrees(float latitudeDegrees, float longtitudeDegrees, float length /*= 1.0f*/)
{
	float AC = length * CosDegrees(latitudeDegrees);
	float x = AC * CosDegrees(longtitudeDegrees);
	float y = AC * SinDegrees(longtitudeDegrees);
	float z = length * SinDegrees(latitudeDegrees);
	return Vec3(x, y, z);				//Make XYZ in a cube, easier to see the relationship between them
}

//-----------------------------------------------------------------------------------------------
const Vec3 Vec3::operator + (const Vec3& vecToAdd) const
{
	return Vec3(x + vecToAdd.x, y + vecToAdd.y, z + vecToAdd.z);
}


//-----------------------------------------------------------------------------------------------
const Vec3 Vec3::operator-(const Vec3& vecToSubtract) const
{
	return Vec3(x - vecToSubtract.x, y - vecToSubtract.y, z - vecToSubtract.z);
}


//------------------------------------------------------------------------------------------------
const Vec3 Vec3::operator-() const
{
	return Vec3(-x, -y, -z);
}


//-----------------------------------------------------------------------------------------------
const Vec3 Vec3::operator*(float uniformScale) const
{
	return Vec3(x * uniformScale, y * uniformScale, z * uniformScale);
}


//------------------------------------------------------------------------------------------------
const Vec3 Vec3::operator*(const Vec3& vecToMultiply) const
{
	return Vec3(x * vecToMultiply.x, y * vecToMultiply.y, z * vecToMultiply.z);
}


//-----------------------------------------------------------------------------------------------
const Vec3 Vec3::operator/(float inverseScale) const
{
	float Scale = 1 / inverseScale;
	return Vec3(x * Scale, y * Scale, z * Scale);
}


//-----------------------------------------------------------------------------------------------
void Vec3::operator+=(const Vec3& vecToAdd)
{
	x += vecToAdd.x;
	y += vecToAdd.y;
	z += vecToAdd.z;
}


//-----------------------------------------------------------------------------------------------
void Vec3::operator-=(const Vec3& vecToSubtract)
{
	x -= vecToSubtract.x;
	y -= vecToSubtract.y;
	z -= vecToSubtract.z;
}


//-----------------------------------------------------------------------------------------------
void Vec3::operator*=(const float uniformScale)
{
	x *= uniformScale;
	y *= uniformScale;
	z *= uniformScale;
}


//-----------------------------------------------------------------------------------------------
void Vec3::operator/=(const float uniformDivisor)
{
	float Scale = 1 / uniformDivisor;
	x *= Scale;
	y *= Scale;
	z *= Scale;
}


//-----------------------------------------------------------------------------------------------
void Vec3::operator=(const Vec3& copyFrom)
{
    	x = copyFrom.x;
	y = copyFrom.y;
	z = copyFrom.z;

}


//-----------------------------------------------------------------------------------------------
const Vec3 operator*(float uniformScale, const Vec3& vecToScale)
{
	return Vec3(vecToScale.x * uniformScale, vecToScale.y * uniformScale, vecToScale.z * uniformScale);
}


//-----------------------------------------------------------------------------------------------
bool Vec3::operator==(const Vec3& compare) const
{
	return x == compare.x && y == compare.y && z == compare.z;
}


//-----------------------------------------------------------------------------------------------
bool Vec3::operator!=(const Vec3& compare) const
{
	return x != compare.x || y != compare.y || z != compare.z;
}


