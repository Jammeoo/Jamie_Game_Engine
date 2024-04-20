#include "Engine/Math/CubicHermiteCurve2D.hpp"
#include "Engine/Math/CubicBezierCurve2D.hpp"
#include "Engine/Math/MathUtils.hpp"
CubicHermiteCurve2D::CubicHermiteCurve2D(Vec2 startPos, Vec2 velStart, Vec2 m_velEnd, Vec2 endPos)
	:m_startPos(startPos),
	m_velStart(velStart),
	m_velEnd(m_velEnd),
	m_endPos(endPos)
{

}

CubicHermiteCurve2D::CubicHermiteCurve2D(CubicBezierCurve2D const& fromBezier)
	:m_startPos(fromBezier.m_startPos),
	m_endPos(fromBezier.m_endPos),
	m_velStart(3.f * (fromBezier.m_guidePos1 - fromBezier.m_startPos)),
	m_velEnd(3.f * (fromBezier.m_endPos - fromBezier.m_guidePos2))
{

}

Vec2 CubicHermiteCurve2D::EvaluateAtParametric(float parametricZeroToOne) const
{
	Vec2 guidePos1 = (m_velStart / 3.f) + m_startPos;
	Vec2 guidePos2 = -(m_velEnd / 3.f) + m_endPos;
	float x = ComputeCubicBezier1D(m_startPos.x, guidePos1.x, guidePos2.x, m_endPos.x, parametricZeroToOne);
	float y = ComputeCubicBezier1D(m_startPos.y, guidePos1.y, guidePos2.y, m_endPos.y, parametricZeroToOne);
	return Vec2(x, y);
}

float CubicHermiteCurve2D::GetApproximateLength(int numSubdivisions /*= 64*/) const
{
	float length = 0.f;
	float deltaT = 1.f / (float)numSubdivisions;

	for (int subdivIndex = 0; subdivIndex < numSubdivisions; subdivIndex++)
	{
		float currentT = subdivIndex * deltaT;
		Vec2 currentPosi = EvaluateAtParametric(currentT);
		Vec2 nextPosi = EvaluateAtParametric(currentT + deltaT);
		float currentSubdivLength = GetDistance2D(currentPosi, nextPosi);
		length += currentSubdivLength;
	}
	return length;
}

Vec2 CubicHermiteCurve2D::EvaluateAtApproximateDistance(float distanceAlongCurve, int numSubdivisions /*= 64*/) const
{
	float fullLength = GetApproximateLength(numSubdivisions);
	if (distanceAlongCurve >= fullLength)
	{
		return m_endPos;
	}

	//float deltaSeg = fullLength / (float)numSubdivisions;
	float deltaT = 1.f / (float)numSubdivisions;

	float currentLengthSum = 0;
	float timeSum = 0;
	Vec2 resultLastCheckPoint = Vec2(0.f, 0.f);
	Vec2 resultNextCheckPoint = Vec2(0.f, 0.f);
	float distanceBtwCheckPoint = 0.f;

	for (int segmentIndex = 0; segmentIndex < numSubdivisions; segmentIndex++) //?
	{
		resultLastCheckPoint = EvaluateAtParametric(timeSum);
		resultNextCheckPoint = EvaluateAtParametric(timeSum + deltaT);
		distanceBtwCheckPoint = GetDistance2D(resultLastCheckPoint, resultNextCheckPoint);
		//compare
		if (currentLengthSum + distanceBtwCheckPoint > distanceAlongCurve)
		{
			break;
		}
		//add time
		currentLengthSum += distanceBtwCheckPoint;
		timeSum += deltaT;

	}

	float distanceFraction = distanceAlongCurve - currentLengthSum;
	float ratio = distanceFraction / distanceBtwCheckPoint;
	float resultX = Interpolate(resultLastCheckPoint.x, resultNextCheckPoint.x, ratio);
	float resultY = Interpolate(resultLastCheckPoint.y, resultNextCheckPoint.y, ratio);


	// 	float resultX = RangeMap(distanceFraction, 0.f, distanceBtwCheckPoint, resultLastCheckPoint.x, resultNextCheckPoint.x);
	// 	float resultY = RangeMap(distanceFraction, 0.f, distanceBtwCheckPoint, resultLastCheckPoint.y, resultNextCheckPoint.y);
	return Vec2(resultX, resultY);
// 	float fullLength = GetApproximateLength(numSubdivisions);
// 	if (distanceAlongCurve > fullLength)
// 	{
// 		return Vec2(0.f, 0.f);
// 	}
// 	float t = distanceAlongCurve / fullLength;
// 	Vec2 result = EvaluateAtParametric(t);
// 	return result;
}

