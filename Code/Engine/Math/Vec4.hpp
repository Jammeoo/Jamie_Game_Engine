#pragma once


struct Vec4
{
public:
	Vec4() = default;// Construction/Destruction
	~Vec4() {};												// destructor (do nothing)
	Vec4(float initialX, float initialY, float initialZ,float initialW)
		:x(initialX)
		, y(initialY)
		, z(initialZ)
		, w(initialW)
	{}
	//public: // NOTE: this is one of the few cases where we break both the "m_" naming rule AND the avoid-public-members rule
	float x = 0.f;
	float y = 0.f;
	float z = 0.f;
	float w = 0.f;
	Vec4(const Vec4& copyFrom);
const Vec4	operator-(const Vec4& vecToSubtract) const;	// vec4 - vec2
void		operator*=(const Vec4& vecToMultiply);			// vec4 *= vec4
void		operator*=(const float& scale);			// vec4 *= vec4
};