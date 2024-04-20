#pragma once
#include "Engine/Math/Vec2.hpp"

//-----------------------------------------------------------------------------------------------------------------------------------------------------------------
struct AABB2 {
public:
	Vec2 m_mins;
	Vec2 m_maxs;
public:
	//Construction/Destruction
	~AABB2() {};				//destructor(do nothing)
	AABB2() {};					//default constructor(do nothing)
	AABB2(AABB2 const& copyFrom);	//copy constructor(from another Vec2)
	explicit AABB2(float minX, float minY, float maxX, float maxY);		//explicit constructor(from x1, y1, x2, y2)
	explicit AABB2(Vec2 const& mins, Vec2 const& maxs); // explicit constructor(from mins,  maxs)

	//Accessors(const methods)
	bool IsPointInside(Vec2 const& point) const;
	Vec2 const GetCenter() const;
	Vec2 const GetDimensions() const;
	Vec2 const GetNearestPoint(Vec2 const& referencePosition) const;
	Vec2 const GetPointAtUV(Vec2 const& uv) const; // uv = (0, 0) is at mins; uv = (1,1) is at maxs
	Vec2 const GetUVForPoint(Vec2 const& point) const; // uv = (.5, .5) at center; u or v outside[0, 1] extrapolated
	AABB2 const GetBoxWithin(Vec2 uvMins, Vec2 uvMaxs)const;
	//Mutators(non-const methods)
	void Translate(Vec2 const& translationToApply);
	void SetCenter(Vec2 const& newCenter);
	void SetDimensions(Vec2 const& newDimensions);
	void StretchToIncludePoint(Vec2 const& point); // does minimal stretching required(none if already on point)

	static const AABB2 ZERO_TO_ONE;
};













