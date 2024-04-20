#define _USE_MATH_DEFINES
#include <math.h>
#include "Engine/Math/Vec2.hpp"
#include"Engine/Core/Vertex_PCU.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/Vec3.hpp"
//#include "Engine/Core/EngineCommon.hpp"

const Vec2 Vec2::ZERO = Vec2(0.f, 0.f);
const Vec2 Vec2::ONE = Vec2(1.f, 1.f);


//-----------------------------------------------------------------------------------------------
Vec2::Vec2( const Vec2& copy )
	: x(copy.x)
	, y(copy.y)
{
}


//-----------------------------------------------------------------------------------------------
Vec2::Vec2( float initialX, float initialY )
	: x(initialX)
	, y(initialY)
{
}

Vec2::Vec2(const Vec3& v3) 
	: x(v3.x)
	, y(v3.y)
{

}

Vec2 const Vec2::MakeFromPolarRadians(float orientationRadians, float length)
{
	return Vec2(length * CosDegrees(orientationRadians * 180.f / (float)M_PI), length * SinDegrees(orientationRadians * 180.f / (float)M_PI));
}

Vec2 const Vec2::MakeFromPolarDegrees(float orientationDegrees, float length)
{
	return Vec2(length * CosDegrees(orientationDegrees), length * SinDegrees(orientationDegrees));
}


float Vec2::GetLength() const
{
	return sqrtf(x * x + y * y);
}

float Vec2::GetLengthSquared() const
{
	return x * x + y * y;
}

float Vec2::GetOrientationRadians() const
{
	return float(atan2(y, x));
}

float Vec2::GetOrientationDegrees() const
{
	return float(atan2(y, x)) * (180.f / float(M_PI));
}

Vec2 const Vec2::GetRotated90Degrees() const
{
	return Vec2(-y, x);
}

Vec2 const Vec2::GetRotatedMinus90Degrees() const
{
	return Vec2(y, -x);
}


Vec2 const Vec2::GetRotatedRadians(float deltaRadians) const
{
	float degrees = ConvertRadiansToDegrees(atan2f(y, x) + deltaRadians);
	return Vec2(GetLength() * CosDegrees(degrees), GetLength() * SinDegrees(degrees));
}

Vec2 const Vec2::GetRotatedDegrees(float deltaDegrees) const
{
	float degrees = Atan2Degrees(y, x) + deltaDegrees;
	return Vec2(GetLength() * CosDegrees(degrees), GetLength() * SinDegrees(degrees));
}

Vec2 const Vec2::GetClamped(float maxLength) const
{
	if (GetLength() > maxLength) {
		float scale = maxLength / GetLength();
		return Vec2(scale * x, scale * y);
	}
	else {
		return Vec2(x, y);
	}
}

Vec2 const Vec2::GetNormalized() const
{
	if (GetLength() <= 0) 
	{
		return Vec2(0.f, 0.f);
	}
	float scale = 1 / GetLength();
	return Vec2(x* scale, y* scale);
}

Vec2 const Vec2::GetReflected(Vec2 normalVector) const
{
	Vec2 defaultVecter = Vec2(x, y);  

	Vec2 vecDefaultProjOntoN = DotProduct2D(defaultVecter, normalVector) * normalVector;
	Vec2 vecDefaultProjOntoSurface = defaultVecter - vecDefaultProjOntoN;
	Vec2 result = vecDefaultProjOntoSurface - vecDefaultProjOntoN;
	return result;
}

void Vec2::SetOrientationRadians(float newOrientationRadians)
{
	float tem_Length = GetLength();
	x = tem_Length * cosf(newOrientationRadians);
	y = tem_Length * sinf(newOrientationRadians);
}

void Vec2::SetOrientationDegrees(float newOrientationDegrees)
{
	float tem_Length = GetLength();
	x = tem_Length * CosDegrees(newOrientationDegrees);
	y = tem_Length * SinDegrees(newOrientationDegrees);
}

void Vec2::SetPolarRadians(float newOrientationRadians, float newLength)
{
	x = newLength * cosf(newOrientationRadians);
	y = newLength * sinf(newOrientationRadians);
}

void Vec2::SetPolarDegrees(float newOrientationDegrees, float newLength)
{
	x = newLength * CosDegrees(newOrientationDegrees);
	y = newLength * SinDegrees(newOrientationDegrees);
}

void Vec2::Rotate90Degrees()
{
	Vec2 temp_NewVertex(-y, x);
	x = temp_NewVertex.x;
	y = temp_NewVertex.y;
}

void Vec2::RotateMinus90Degrees()
{
	Vec2 temp_NewVertex(y, -x);
	x = temp_NewVertex.x;
	y = temp_NewVertex.y;
}

void Vec2::RotateRadians(float deltaRadians)
{
	float degrees = ConvertRadiansToDegrees(atan2f(y, x) + deltaRadians);
	Vec2 temp_NewVertex(GetLength() * CosDegrees(degrees), GetLength() * SinDegrees(degrees));
	x = temp_NewVertex.x;
	y = temp_NewVertex.y;
}

void Vec2::RotateDegrees(float deltaDegrees)
{
	float degrees = Atan2Degrees(y, x) + deltaDegrees;
	Vec2 temp_NewVertex(GetLength() * CosDegrees(degrees), GetLength() * SinDegrees(degrees));
	x = temp_NewVertex.x;
	y = temp_NewVertex.y;
}

void Vec2::SetLength(float newLength)
{
	Normalize();
	x = x * newLength;
	y = y * newLength;
}

void Vec2::ClampLength(float maxLength)
{
	if (GetLength() > maxLength) {
		float scale = maxLength / GetLength();
		Vec2 temp_NewVertex = Vec2(scale * x, scale * y);
		x = temp_NewVertex.x;
		y = temp_NewVertex.y;
		//x = scale * x;
		//y = scale * y;
	}
}


void Vec2::Normalize()
{
	if (!GetLength()==0)//!!!!
	{
		float scale = 1 / GetLength();
		x = x * scale;
		y = y * scale;
	}
}

float Vec2::NormalizeAndGetPreviousLength()
{
	float previousLength = sqrtf(x * x + y * y);
	float scale = 1 / previousLength;
	x = x * scale;
	y = y * scale;
	return previousLength;
}


void Vec2::Reflect(Vec2 normalVector)
{
	Vec2 result = GetReflected(normalVector);
	x = result.x;
	y = result.y;
}

void Vec2::SetFromText(char const* text)
{
	Strings originalText= SplitStringOnDelimiter(text, ',');
	x = (float)atof(originalText[0].c_str());
	y = (float)atof(originalText[1].c_str());
}

void Vec2::SetFromText(char const* text, char delimiterToSplitOn)
{
	Strings originalText = SplitStringOnDelimiter(text, delimiterToSplitOn);
	x = (float)atof(originalText[0].c_str());
	y = (float)atof(originalText[1].c_str());
}

//-----------------------------------------------------------------------------------------------
const Vec2 Vec2::operator + ( const Vec2& vecToAdd ) const
{
	return Vec2(x + vecToAdd.x, y + vecToAdd.y);
}


//-----------------------------------------------------------------------------------------------
const Vec2 Vec2::operator-( const Vec2& vecToSubtract ) const
{
	return Vec2(x - vecToSubtract.x, y - vecToSubtract.y);
}


//------------------------------------------------------------------------------------------------
const Vec2 Vec2::operator-() const
{
	return Vec2(-x, -y);
}


//-----------------------------------------------------------------------------------------------
const Vec2 Vec2::operator*( float uniformScale ) const
{
	return Vec2(x * uniformScale, y * uniformScale);
}


//------------------------------------------------------------------------------------------------
const Vec2 Vec2::operator*( const Vec2& vecToMultiply ) const
{
	return Vec2(x * vecToMultiply.x, y * vecToMultiply.y);
}


//-----------------------------------------------------------------------------------------------
const Vec2 Vec2::operator/( float inverseScale ) const
{
	float Scale = 1 / inverseScale;
	return Vec2(x * Scale, y * Scale);
}


//-----------------------------------------------------------------------------------------------
void Vec2::operator+=( const Vec2& vecToAdd )
{
	x += vecToAdd.x;
	y += vecToAdd.y;
}


//-----------------------------------------------------------------------------------------------
void Vec2::operator-=( const Vec2& vecToSubtract )
{
	x -= vecToSubtract.x;
	y -= vecToSubtract.y;
}


//-----------------------------------------------------------------------------------------------
void Vec2::operator*=( const float uniformScale )
{
	x *= uniformScale;
	y *= uniformScale;
}


//-----------------------------------------------------------------------------------------------
void Vec2::operator/=( const float uniformDivisor )
{
	float Scale = 1 / uniformDivisor;
	x *= Scale;
	y *= Scale;
}


//-----------------------------------------------------------------------------------------------
void Vec2::operator=( const Vec2& copyFrom )
{
	x = copyFrom.x;
	y = copyFrom.y;
}


//-----------------------------------------------------------------------------------------------
const Vec2 operator*( float uniformScale, const Vec2& vecToScale )
{
	return Vec2(vecToScale.x * uniformScale, vecToScale.y * uniformScale);
}


//-----------------------------------------------------------------------------------------------
bool Vec2::operator==( const Vec2& compare ) const
{
	return x == compare.x && y == compare.y;
}


//-----------------------------------------------------------------------------------------------
bool Vec2::operator!=( const Vec2& compare ) const
{
	return x != compare.x || y != compare.y;
}


