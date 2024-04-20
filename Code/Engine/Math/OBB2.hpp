#pragma once
#include "Engine/Math/Vec2.hpp"
struct OBB2
{
public:
	Vec2 m_center = Vec2(0.f, 0.f);
	Vec2 m_iBasisNormal = Vec2(1.f, 0.f);
	Vec2 m_halfDimensions = Vec2(1.f, 1.f);
public:
	void GetCornerPoints(Vec2* out_fourCornerWorldPositions) const;
	Vec2 GetLocalPosForWorldPos(Vec2 worldPos) const;
	Vec2 GetWorldPosForLocalPos(Vec2 localPos) const;
	void RotateAboutCenter(float rotationDeltaDegrees);
};