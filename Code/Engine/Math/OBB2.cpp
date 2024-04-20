#include "Engine/Math/OBB2.hpp"
#include "Engine/Math/MathUtils.hpp"
void OBB2::GetCornerPoints(Vec2* out_fourCornerWorldPositions) const
{
	Vec2 jBasisNormal = Vec2(-m_iBasisNormal.y, m_iBasisNormal.x);
	Vec2 TR = m_iBasisNormal + jBasisNormal;
	Vec2 TL = jBasisNormal - m_iBasisNormal;
	Vec2 BL = -jBasisNormal - m_iBasisNormal;
	Vec2 BR = m_iBasisNormal - jBasisNormal;

	out_fourCornerWorldPositions[0] = TR;
	out_fourCornerWorldPositions[1] = TL;
	out_fourCornerWorldPositions[2] = BL;
	out_fourCornerWorldPositions[3] = BR;
}

Vec2 OBB2::GetLocalPosForWorldPos(Vec2 worldPos) const
{
	Vec2 displacement = worldPos - m_center;
	Vec2 jBasisNormal = Vec2(-m_iBasisNormal.y, m_iBasisNormal.x);
	Vec2 posiInLocal = Vec2(DotProduct2D(displacement, m_iBasisNormal), DotProduct2D(displacement, jBasisNormal));
	return posiInLocal;
}

Vec2 OBB2::GetWorldPosForLocalPos(Vec2 localPos) const
{
	Vec2 jBasisNormal = Vec2(-m_iBasisNormal.y, m_iBasisNormal.x);
	Vec2 worldPosition = m_center + localPos.x * m_iBasisNormal + localPos.y * jBasisNormal;
	return worldPosition;
}

void OBB2::RotateAboutCenter(float rotationDeltaDegrees)
{
	//center remain same
	Vec2 newIBase = m_iBasisNormal.GetRotatedDegrees(rotationDeltaDegrees);
}

