#pragma once
#include "Engine/Math/Vec2.hpp"

//-----------------------------------------------------------------------------------------------
struct Vec3
{
	Vec3() = default;// Construction/Destruction
	//~Vec3() {}												// destructor (do nothing)
	Vec3(float initialX, float initialY, float initialZ) 
		:x(initialX)
		,y(initialY)
		,z(initialZ)
	{}
	Vec3(Vec2 XY, float initialZ)
		:x(XY.x)
		, y(XY.y)
		, z(initialZ)
	{}

	Vec3(float initialX, Vec2 YZ)
		:x(initialX)
		, y(YZ.x)
		, z(YZ.y)
	{}
//public: // NOTE: this is one of the few cases where we break both the "m_" naming rule AND the avoid-public-members rule
	float x = 0.f;
	float y = 0.f;
	float z = 0.f;
	// default constructor (do nothing)
	Vec3(const Vec3& copyFrom);							// copy constructor (from another vec2)
	//explicit Vec3(float initialX, float initialY, float initialZ);		// explicit constructor (from x, y, z)

	
	//Accessors(const methods)
	
	float       GetLength()const;
	float       GetLengthXY()const;
	float		GetLengthSquared() const;
	float		GetLengthXYSquared() const;
	float       GetAngleAboutZRadians()const;
	float       GetAngleAboutZDegrees()const;
	Vec3 const GetRotatedAboutZRadians(float deltaRadians)const;
	Vec3 const GetRotatedAboutZDegrees(float deltaDegrees)const;
	Vec3 const GetClamped(float maxLength)const;
	Vec3 const GetNormalized()const;
	void const Normalize();
	void SetFromText(char const* text, char delimiterToSplitOn);
	static Vec3 const MakeFromPolarRadians(float latitudeRadians, float longtitudeRadians, float length = 1.0f);
	static Vec3 const MakeFromPolarDegrees(float latitudeDegrees, float longtitudeDegrees, float length = 1.0f);
	

	// Operators (const)
	bool		operator==(const Vec3& compare) const;		// vec2 == vec2
	bool		operator!=(const Vec3& compare) const;		// vec2 != vec2
	const Vec3	operator+(const Vec3& vecToAdd) const;		// vec2 + vec2
	const Vec3	operator-(const Vec3& vecToSubtract) const;	// vec2 - vec2
	const Vec3	operator-() const;								// -vec2, i.e. "unary negation"
	const Vec3	operator*(float uniformScale) const;			// vec2 * float
	const Vec3	operator*(const Vec3& vecToMultiply) const;	// vec2 * vec2
	const Vec3	operator/(float inverseScale) const;			// vec2 / float

	// Operators (self-mutating / non-const)
	void		operator+=(const Vec3& vecToAdd);				// vec2 += vec2
	void		operator-=(const Vec3& vecToSubtract);		// vec2 -= vec2
	void		operator*=(const float uniformScale);			// vec2 *= float
	void		operator/=(const float uniformDivisor);		// vec2 /= float
	void		operator=(const Vec3& copyFrom);				// vec2 = vec2

	static const Vec3 X_AXIS;
	static const Vec3 Y_AXIS;
	static const Vec3 Z_AXIS;
	static const Vec3 WORLD_ORIGIN;
	// Standalone "friend" functions that are conceptually, but not actually, part of Vec2::
	friend const Vec3 operator*(float uniformScale, const Vec3& vecToScale);	// float * vec2
};


