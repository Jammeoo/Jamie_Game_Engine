#pragma once
#include "Engine/Math/Vec2.hpp"
struct LineSegment2
{
public:
	LineSegment2() = default;
	LineSegment2(Vec2 const& start, Vec2 const& end) :m_start(start),m_end(end)
	{

	}
	~LineSegment2() = default;
public:
	Vec2 m_start = Vec2(0.f, 0.f);
	Vec2 m_end = Vec2(0.f, 0.f);
public:
	void Translate(Vec2 translation);
	void SetCenter(Vec2 newCenter);
	void RotateAboutCenter(float rotationDeltaDegrees);
};