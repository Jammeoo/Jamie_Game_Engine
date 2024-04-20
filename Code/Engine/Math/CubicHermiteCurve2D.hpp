#pragma once

#include "Engine/Math/Vec2.hpp"
class CubicBezierCurve2D;

class CubicHermiteCurve2D
{
public:
	CubicHermiteCurve2D(Vec2 startPos, Vec2 velStart, Vec2 m_velEnd, Vec2 endPos);
	explicit CubicHermiteCurve2D(CubicBezierCurve2D const& fromBezier);
	Vec2 EvaluateAtParametric(float parametricZeroToOne)const;
	float GetApproximateLength(int numSubdivisions = 64)const;
	Vec2 EvaluateAtApproximateDistance(float distanceAlongCurve, int numSubdivisions = 64)const;
public:
	Vec2 m_startPos = Vec2::ZERO;
	Vec2 m_velStart = Vec2::ZERO;
	Vec2 m_endPos = Vec2::ZERO;
	Vec2 m_velEnd = Vec2::ZERO;
};