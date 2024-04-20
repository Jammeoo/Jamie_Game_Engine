#include "Engine/Math/Hexagon.hpp"
#include <assert.h>
// Hexagon2D::Hexagon2D(Vec3 center, float circumRadius):
// 	m_center{ center.x,center.y },m_circumRadius(circumRadius),m_sideLength(circumRadius)
// {
// 	assert(circumRadius > 0.f);
// 	m_inRadius = m_circumRadius * sqrt3 * 0.5f;
// 	FillPointArray2D();
// }

Hexagon2D::Hexagon2D(Vec3 center, float inRadius) :
	m_center{ center.x,center.y }, m_inRadius(inRadius)
{
	assert(inRadius > 0.f);
	m_circumRadius = m_inRadius * oneOverSqrt3 * 2.f;
	m_sideLength = m_circumRadius;
	FillPointArray2D();
}

// Hexagon2D::Hexagon2D(Vec2 center, float circumRadius):
// 	m_center(center),m_circumRadius(circumRadius), m_sideLength(circumRadius)
// {
// 	assert(circumRadius > 0.f);
// 	m_inRadius = m_circumRadius * sqrt3 * 0.5f;
// 	FillPointArray2D();
// }

void Hexagon2D::FillPointArray2D()
{
	//prepare
	Vec2 centerPoint = m_center;
	Vec2 upDisp{ 0.f, 1.f };
	Vec2 downDisp = -upDisp;
	Vec2 rightDisp{ 1.f,0.f };
	Vec2 leftDisp = -rightDisp;

	Vec2 v0 = centerPoint + rightDisp * m_circumRadius;

	float rightSmallStepLength = 0.5f * m_circumRadius;
	Vec2 rightUpStepDisp = rightSmallStepLength * rightDisp + upDisp * m_inRadius;
	Vec2 v1 = centerPoint + rightUpStepDisp;

	float leftSmallStepLength = 0.5f * m_circumRadius;
	Vec2 leftUpStepDisp = leftSmallStepLength * leftDisp + upDisp * m_inRadius;
	Vec2 v2= centerPoint + leftUpStepDisp;

	Vec2 v3 = centerPoint - rightDisp * m_circumRadius;
	
	Vec2 v4 = centerPoint - rightUpStepDisp;
	Vec2 v5 = centerPoint - leftUpStepDisp;

	m_pointArray[0] = v0;
	m_pointArray[1] = v1;
	m_pointArray[2] = v2;
	m_pointArray[3] = v3;
	m_pointArray[4] = v4;
	m_pointArray[5] = v5;

	m_pointArray3D[0] = Vec3(v0.x, v0.y, 0.f);
	m_pointArray3D[1] = Vec3(v1.x, v1.y, 0.f);
	m_pointArray3D[2] = Vec3(v2.x, v2.y, 0.f);
	m_pointArray3D[3] = Vec3(v3.x, v3.y, 0.f);
	m_pointArray3D[4] = Vec3(v4.x, v4.y, 0.f);
	m_pointArray3D[5] = Vec3(v5.x, v5.y, 0.f);

}
