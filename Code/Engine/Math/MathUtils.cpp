#define _USE_MATH_DEFINES
#include <math.h>
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Vec4.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/OBB2.hpp"
#include "Engine/Math/Capsule2.hpp"
#include "Engine/Math/LineSegment2.hpp"
#include"Engine/Math/MathUtils.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/Hexagon.hpp"


float ConvertDegreesToRadians(const float degrees)
{
	return (float)degrees * (float(M_PI) / 180.0f);
}

float ConvertRadiansToDegrees(const float radians)
{
	return radians * (180.f / float(M_PI));
}

float CosDegrees(const float degrees)
{
	return cosf(ConvertDegreesToRadians(degrees));
}

float SinDegrees(const float degrees)
{
	return sinf(ConvertDegreesToRadians(degrees));
}

float TanDegrees(float degrees)
{
	float degreesToRad = ConvertDegreesToRadians(degrees);
	float result = tanf(degreesToRad);
	return result;
}

float Atan2Degrees(const float y, const float x)
{
	return ConvertRadiansToDegrees(atan2f(y, x));
}

float GetShortestAngularDispDegrees(float startDegrees, float endDegrees)
{
	float deltaDegrees = endDegrees - startDegrees;
	if (deltaDegrees >180.f)
	{
		while (fabsf(deltaDegrees)>180)
		{
			deltaDegrees -= 360.f;
		}
		return deltaDegrees;
	}
	else if (deltaDegrees < -180.f) {
		while (fabsf(deltaDegrees) > 180)
		{
			deltaDegrees += 360.f;
		}
		return deltaDegrees;
	} else return deltaDegrees;
}

float GetTurnedTowardDegrees(float currentDegrees, float goalDegrees, float maxDeltaDegrees)
{
// 	float difference = goalDegrees - currentDegrees;
// 	if (difference < 0) 
// 	{
// 		difference = -difference;
// 	}
// 	while (difference > 360.f)
// 	{
// 		difference -= 360.f;
// 	}
// 	if (difference<= maxDeltaDegrees||(360.f-difference) < maxDeltaDegrees)
// 	{
// 		return goalDegrees;
// 	}
// 	if (currentDegrees < goalDegrees && difference<180.f)
// 	{
// 		return currentDegrees + maxDeltaDegrees;
// 	}
// 	else
// 	{
// 		return currentDegrees - maxDeltaDegrees;
// 	}
	float distance = GetShortestAngularDispDegrees(currentDegrees, goalDegrees);

	if (distance > 0)
	{
		if (distance <= maxDeltaDegrees)
		{
			return goalDegrees;
		}
		else if (distance > maxDeltaDegrees)
		{
			currentDegrees += maxDeltaDegrees;
			return currentDegrees;
		}
	}
	else
	{
		if (distance <= -maxDeltaDegrees)
		{
			currentDegrees -= maxDeltaDegrees;
			return currentDegrees;
		}
		else if (distance > -maxDeltaDegrees)
		{
			return goalDegrees;
		}
	}
	return maxDeltaDegrees;
}

float GetAngleDegreesBetweenVectors2D(Vec2 const& a, Vec2 const& b)
{
	float dotValue = DotProduct2D(a, b);
	float resultInRadius = acosf(dotValue / (a.GetLength() * b.GetLength()));
	return ConvertRadiansToDegrees(resultInRadius);
}

float DotProduct2D(Vec2 const& a, Vec2 const& b)
{
	return (a.x * b.x + a.y * b.y);
}
float DotProduct3D(Vec3 const& a, Vec3 const& b)
{
	return (a.x * b.x + a.y * b.y + a.z * b.z);
}

float DotProduct4D(Vec4 const& a, Vec4 const& b)
{
	return (a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w);
}

float CrossProduct2D(Vec2 const& a, Vec2 const& b)
{
	return a.x * b.y - b.x * a.y;
}

Vec3 CrossProduct3D(Vec3 const& a, Vec3 const& b)
{
	return Vec3(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
}

float NormalizeByte(unsigned char byteValue)
{
	float value = (float)byteValue;
	return value / 255.f;
}

unsigned char DenormalizeByte(float zeroToOne)
{
	float magnified = zeroToOne * 256.f;
	int intValue = RoundDownToInt(magnified);
	if (zeroToOne == 1.f)
	{
		return 255;
	}
	return (unsigned char)intValue;

}

float GetDistance2D(Vec2 const& positionA, Vec2 const& positionB)
{
	return sqrtf((positionA.x - positionB.x) * (positionA.x - positionB.x) + (positionA.y - positionB.y) * (positionA.y - positionB.y));
}

float GetDistanceSquared2D(Vec2 const& positionA, Vec2 const& positionB)
{
	float dx = positionB.x - positionA.x;
	float dy = positionB.y - positionA.y;
	return (dx * dx) + (dy * dy);
}

float GetDistance3D(Vec3 const& positionA, Vec3 const& positionB)
{
	return sqrtf((positionA.x - positionB.x) * (positionA.x - positionB.x) + (positionA.y - positionB.y) * (positionA.y - positionB.y) 
		+ (positionA.z - positionB.z) * (positionA.z - positionB.z));
}

float GetDistanceSquared3D(Vec3 const& positionA, Vec3 const& positionB)
{
	return (positionA.x - positionB.x) * (positionA.x - positionB.x) + (positionA.y - positionB.y) * (positionA.y - positionB.y)
		+ (positionA.z - positionB.z) * (positionA.z - positionB.z);
}

float GetDistanceXY3D(Vec3 const& positionA, Vec3 const& positionB)
{
	return sqrtf((positionA.x - positionB.x) * (positionA.x - positionB.x) + (positionA.y - positionB.y) * (positionA.y - positionB.y));
}

float GetDistanceXYSquared3D(Vec3 const& positionA, Vec3 const& positionB)
{
	return (positionA.x - positionB.x) * (positionA.x - positionB.x) + (positionA.y - positionB.y) * (positionA.y - positionB.y);
}

int GetTaxicabDistance2D(IntVec2 const& pointA, IntVec2 const& pointB)
{
	return (abs(pointB.x - pointA.x) + abs(pointB.y - pointA.y));
}

float GetProjectedLength2D(Vec2 const& vectorToProject, Vec2 const& vectorToProjectOnto)
{
	float dotProduct = DotProduct2D(vectorToProject, vectorToProjectOnto);
	return (dotProduct / vectorToProjectOnto.GetLength());
}

Vec2 const GetProjectedOnto2D(Vec2 const& vectorToProject, Vec2 const& vectorToProjectOnto)
{
// 	float projectedLength = GetProjectedLength2D(vectorToProject, vectorToProjectOnto);
// 	Vec2 result = vectorToProjectOnto;
// 	result.SetLength(projectedLength);
// 	return result;
	return GetProjectedLength2D(vectorToProject, vectorToProjectOnto) * vectorToProjectOnto.GetNormalized();
}

bool IsPointInsideDisc2D(Vec2 const& point, Vec2 const& discCenter, float discRadius)
{
	return (GetDistance2D(point, discCenter) < discRadius);
}

bool IsPointInsideOrientedSector2D(Vec2 const& point, Vec2 const& sectorTip, float sectorForwardDegrees, float sectorApertureDegrees, float sectorRadius)
{
	if (GetDistance2D(point, sectorTip) >= sectorRadius) 
	{
		return false;
	}
	else
	{
		Vec2 displacementBetweenPoints = point - sectorTip;
		Vec2 forwardVector = Vec2(CosDegrees(sectorForwardDegrees), SinDegrees(sectorForwardDegrees));
		float degree = GetAngleDegreesBetweenVectors2D(displacementBetweenPoints, forwardVector);
		if (degree < 0.5f * sectorApertureDegrees) {
			return true;
		}
		else
		{
			return false;
		}
	}
}

bool IsPointInsideCapsule3D(Vec3 const& point, Vec3 const& boneStart, Vec3 const& boneEnd, float radius)
{

	Vec3 SE = boneEnd - boneStart;
	Vec3 SP = point - boneStart;
	if (DotProduct3D(SE, SP) < 0.f) 
	{
		return (GetDistanceSquared3D(point, boneStart) < radius * radius);
	}

	Vec3 EP = point - boneEnd;
	if (DotProduct3D(SE, EP) > 0.f)
	{
		return (GetDistanceSquared3D(point, boneEnd) < radius * radius);

	}

	Vec3 dispStartToEndNorm = SE.GetNormalized();
	float distSPOnToSE = DotProduct3D(SP, dispStartToEndNorm);
	Vec3 SN = dispStartToEndNorm * distSPOnToSE;
	Vec3 N = boneStart + SN;
	return (GetDistanceSquared3D(point, N) < radius * radius);
}

bool IsPointInsideSphere3D(Vec3 const& point, Vec3 const& sphereCenter, float sphereRadius)
{
	float distSquarePoint = GetDistanceSquared3D(point, sphereCenter);
	if (distSquarePoint < sphereRadius * sphereRadius) 
	{
		return true;
	}
	return false;
}

bool IsPointInsideHexagon2D(Vec3 const& point, Hexagon2D const& hexgon)
{
	for (int i = 0; i < 6; i++) 
	{
		int index = i % 6;
		Vec3 hexPoint1(hexgon.m_pointArray[index], 0.f);
		int nextIndex = (i + 1) % 6;
		Vec3 hexPoint2(hexgon.m_pointArray[nextIndex], 0.f);

		Vec3 hexP2ToHexp1 = hexPoint2 - hexPoint1;
		Vec3 pToHexP1 = point - hexPoint1;

		if (CrossProduct3D(hexP2ToHexp1, pToHexP1).z < 0.f) 
		{
			return false;
		}
	}
	return true;;

}

bool IsPointInsideDirectedSector2D(Vec2 const& point, Vec2 const& sectorTip, Vec2 const& sectorForwardNormal, float sectorApertureDegrees, float sectorRadius)
{
	if (GetDistance2D(point, sectorTip) >= sectorRadius)
	{
		return false;
	}
	else
	{
		Vec2 displacementBetweenPoints = point - sectorTip;
		float degree = GetAngleDegreesBetweenVectors2D(displacementBetweenPoints, sectorForwardNormal);
		if (degree < 0.5f * sectorApertureDegrees) 
		{
			return true;
		}
		else
		{
			return false;
		}
	}
}

bool IsPointInsideAABB2D(Vec2 const& point, AABB2 const& box)
{
	if (point.x > box.m_mins.x && point.x < box.m_maxs.x && point.y > box.m_mins.y && point.y < box.m_maxs.y) {
		 return true;
	}
	else return false;
}

bool IsPointInsideOBB2D(Vec2 const& point, OBB2 const& orientedBox)
{
	Vec2 localPoint = orientedBox.GetLocalPosForWorldPos(point);
	Vec2 localDisplace = localPoint - orientedBox.m_center;
	AABB2 aabbBox = AABB2();
	aabbBox.m_mins = -orientedBox.m_halfDimensions;
	aabbBox.m_maxs = orientedBox.m_halfDimensions;
	if (IsPointInsideAABB2D(localPoint, aabbBox)) {
		return true;
	}
	else
	{
		return false;
	}
// 	if (fabsf(localDisplace.x) < orientedBox.m_halfDimensions.x && fabsf(localDisplace.y) < orientedBox.m_halfDimensions.y) 
// 	{
// 		return true;
// 	}
// 	else
// 	{
// 		return false;
// 	}
}

bool IsPointInsideCapsule2D(Vec2 const& point, Capsule2 const& capsule)
{
	Vec2 displaceToLeft = point - capsule.m_start;
	float distanceToLeft = displaceToLeft.GetLength();
	Vec2 displaceToRight = point - capsule.m_end;
	float distanceToRight = displaceToRight.GetLength();
	//case 1 and 2
	if (distanceToLeft < capsule.m_radius || distanceToRight < capsule.m_radius) {
		return true;
	}
	//case 3
	OBB2 obbBox = OBB2();
	obbBox.m_center = capsule.m_start + 0.5f * (capsule.m_end - capsule.m_start);
	obbBox.m_iBasisNormal = (capsule.m_end - capsule.m_start).GetNormalized();
	obbBox.m_halfDimensions = Vec2((capsule.m_end - capsule.m_start).GetLength()*0.5f, capsule.m_radius);
	if (IsPointInsideOBB2D(point, obbBox)) {
		return true;
	}
	return false;
// 	Vec2 displaceNormalizedBetweenCenters = (capsule.m_end - capsule.m_start).GetNormalized();
// 	float projectLength = GetProjectedLength2D(displaceToLeft, displaceNormalizedBetweenCenters);
// 	Vec2 projectedVector = displaceNormalizedBetweenCenters * projectLength;
// 	Vec2 verticalLine = displaceToLeft - projectedVector;
// 	//case 3
// 	if (verticalLine.GetLength() < capsule.radius)
// 	{
// 		return true;
// 	}
// 	return false;
}

bool IsPointInsideCapsule2D(Vec2 const& point, Vec2 const& boneStart, Vec2 const& boneEnd, float radius)
{
	Vec2 displaceToLeft = point - boneStart;
	float distanceToLeft = displaceToLeft.GetLength();
	Vec2 displaceToRight = point - boneEnd;
	float distanceToRight = displaceToRight.GetLength();
	//case 1 and 2
	if (distanceToLeft < radius || distanceToRight < radius) {
		return true;
	}

	Vec2 displaceNormalizedBetweenCenters = (boneEnd - boneStart).GetNormalized();
	float projectLength = GetProjectedLength2D(displaceToLeft, displaceNormalizedBetweenCenters);
	Vec2 projectedVector = displaceNormalizedBetweenCenters * projectLength;
	Vec2 verticalLine = displaceToLeft - projectedVector;
	//case 3
	if (verticalLine.GetLength() < radius)
	{
		return true;
	}
	return false;
}

bool DoDiscsOverlap(Vec2 const& centerA, float radiusA, Vec2 const& centerB, float radiusB)
{
	return GetDistanceSquared2D(centerA, centerB) < (radiusA + radiusB) * (radiusA + radiusB);
}

bool DoSpheresOverlap(Vec3 const& centerA, float radiusA, Vec3 const& centerB, float radiusB)
{
	return GetDistance3D(centerA, centerB) < radiusA + radiusB;
}


bool DoAABB3DOverlap(AABB3 const& first, AABB3 const& second)
{
	if (first.m_mins.x >= second.m_maxs.x ||
		first.m_maxs.x <= second.m_mins.x ||
		first.m_mins.y >= second.m_maxs.y ||
		first.m_maxs.y <= second.m_mins.y ||
		first.m_mins.z >= second.m_maxs.z ||
		first.m_maxs.z <= second.m_mins.z)
	{
		return false;
	}
	else
	{
		return true;
	}
}

bool DoesAABBOverLapWithOBB2D(AABB2 const& aabb, OBB2 const& obb)
{
	Vec2 aabbCenter = aabb.GetCenter();
	Vec2 aabbHalfExtents = aabb.GetDimensions() * 0.5f;

	//Relative position of AABB in OBB space
	Vec2 aabbCenterInOBBSpace = Vec2(
		(aabbCenter.x - obb.m_center.x) * obb.m_iBasisNormal.x +
		(aabbCenter.y - obb.m_center.y) * obb.m_iBasisNormal.y,
		(aabbCenter.x - obb.m_center.x) * (-obb.m_iBasisNormal.y) +
		(aabbCenter.y - obb.m_center.y) * obb.m_iBasisNormal.x
	);
	//displacement from AABB center to OBB Center
	float dispAabbCenterToObbCenterX =fabsf(aabbCenterInOBBSpace.x) - (aabbHalfExtents.x + obb.m_halfDimensions.x);
	float dispAabbCenterToObbCenterY = fabsf(aabbCenterInOBBSpace.y) - (aabbHalfExtents.y + obb.m_halfDimensions.y);


	if (dispAabbCenterToObbCenterX <= 0.f && dispAabbCenterToObbCenterY <= 0.f) 
	{
		return true;
	}
	return false;
}

bool DoesOBBOverlapWithDisc(OBB2 const& obb, Vec2 const& centerA, float radius)
{
	Vec2 nearestPoint = GetNearestPointOnDisc2D(obb.m_center, centerA, radius);
	if (IsPointInsideOBB2D(nearestPoint, obb)) 
	{
		return true;
	}
	return false;
}

Vec2 GetNearestPointOnDisc2D(Vec2 const& referencePosition, Vec2 const& discCenter, float discRadius)
{
	float dis = GetDistance2D(referencePosition, discCenter);
	if (dis <= discRadius)
	{
		return referencePosition;
	}
	else
	{
		Vec2 centerToPoint = referencePosition - discCenter;
		Vec2 displacement = centerToPoint;
		displacement.Normalize();
		Vec2 point = displacement * discRadius + discCenter;
		return point;
	}
}

Vec2 GetNearestPointOnAABB2D(Vec2 const& referencePos, AABB2& box)
{
	if (IsPointInsideAABB2D(referencePos, box)) {
		return referencePos;
	}
	return box.GetNearestPoint(referencePos);
}

Vec2 GetNearestPointOnInfiniteLine2D(Vec2 const& referencePos, LineSegment2 const& infiniteLine)
{
	Vec2 startToEnd = infiniteLine.m_end - infiniteLine.m_start;
	Vec2 startToPosition = referencePos - infiniteLine.m_start;
	//Vec2 endToPosition = referencePos - infiniteLine.m_end;
	//case 3
	Vec2 startToNearestPoint = GetProjectedOnto2D(startToPosition, startToEnd);
	return infiniteLine.m_start + startToNearestPoint;
}

Vec2 GetNearestPointOnInfiniteLine2D(Vec2 const& referencePos, Vec2 const& pointOnLine, Vec2 const& anotherPointOnLine)
{
	Vec2 startToEnd = anotherPointOnLine - pointOnLine;
	Vec2 startToPosition = referencePos - pointOnLine;
	//Vec2 endToPosition = referencePos - anotherPointOnLine;

	//case 3
	Vec2 startToNearestPoint = GetProjectedOnto2D(startToPosition, startToEnd);
	return pointOnLine + startToNearestPoint;
}

Vec2 GetNearestPointOnLineSegment2D(Vec2 const& referencePos, LineSegment2 const& infiniteLine)
{
	Vec2 startToEnd = infiniteLine.m_end - infiniteLine.m_start;
	Vec2 startToPosition = referencePos - infiniteLine.m_start;
	Vec2 endToPosition = referencePos - infiniteLine.m_end;
	//case 1
	if (DotProduct2D(startToEnd, startToPosition) < 0)
	{
		return infiniteLine.m_start;
	}
	//case 2
	if (DotProduct2D(startToEnd, endToPosition) > 0)
	{
		return infiniteLine.m_end;
	}
	//case 3
	Vec2 startToNearestPoint = GetProjectedOnto2D(startToPosition, startToEnd);
	return infiniteLine.m_start + startToNearestPoint;
}

Vec2 GetNearestPointOnLineSegment2D(Vec2 const& referencePos, Vec2 const& pointOnLine, Vec2 const& anotherPointOnLine)
{
	Vec2 startToEnd = anotherPointOnLine - pointOnLine;
	Vec2 startToPosition = referencePos - pointOnLine;
	Vec2 endToPosition = referencePos - anotherPointOnLine;
	//case 1
	if (DotProduct2D(startToEnd, startToPosition) < 0)
	{
		return pointOnLine;
	}
	//case 2
	if (DotProduct2D(startToEnd, endToPosition) > 0)
	{
		return anotherPointOnLine;
	}
	//case 3
	Vec2 startToNearestPoint = GetProjectedOnto2D(startToPosition, startToEnd);
	return pointOnLine + startToNearestPoint;
}

Vec2 GetNearestPointOnCapsule2D(Vec2 const& referencePos, Capsule2 const& capsule)
{
	if (IsPointInsideCapsule2D(referencePos, capsule)) {
		return referencePos;
	}
	Vec2 nearestPointOnBone = GetNearestPointOnLineSegment2D(referencePos, capsule.m_start, capsule.m_end);
	Vec2 nOnBoneToPos = referencePos - nearestPointOnBone;
	Vec2 nOnBoneToCap = nOnBoneToPos.GetClamped(capsule.m_radius);
	return nearestPointOnBone + nOnBoneToCap;//Which is N=N point+N to P
}

Vec2 GetNearestPointOnCapsule2D(Vec2 const& referencePos, Vec2 const& boneStart, Vec2 const& boneEnd, float radius)
{
	if (IsPointInsideCapsule2D(referencePos, boneStart,boneEnd,radius)) 
	{
		return referencePos;
	}
	Vec2 nearestPointOnBone = GetNearestPointOnLineSegment2D(referencePos, boneStart, boneEnd);
	Vec2 nOnBoneToPos = referencePos - nearestPointOnBone;
	Vec2 nOnBoneToCap = nOnBoneToPos.GetClamped(radius);
	return nearestPointOnBone + nOnBoneToCap;//Which is N=N point+N to P
}

Vec2 GetNearestPointOnOBB2D(Vec2 const& referencePos, OBB2 const& orientedBox)
{
	if (IsPointInsideOBB2D(referencePos, orientedBox)) {
		return referencePos;
	}
	Vec2 centerToPos = referencePos - orientedBox.m_center;
	Vec2 jBaseNormal = Vec2(-orientedBox.m_iBasisNormal.y, orientedBox.m_iBasisNormal.x);
	float centerToPosIbase = DotProduct2D(centerToPos, orientedBox.m_iBasisNormal);
	float centerToPosJbase = DotProduct2D(centerToPos, jBaseNormal);
	float nearestPointIbase = GetClamped(centerToPosIbase, -orientedBox.m_halfDimensions.x, orientedBox.m_halfDimensions.x);
	float nearestPointJbase = GetClamped(centerToPosJbase, -orientedBox.m_halfDimensions.y, orientedBox.m_halfDimensions.y);
	Vec2 nearestPoint = orientedBox.m_center + nearestPointIbase * orientedBox.m_iBasisNormal + nearestPointJbase * jBaseNormal;
	return nearestPoint;
}

bool PushDiscOutOfFixedPoint2D(Vec2& mobileDiscCenter, float discRadius, Vec2 const& fixedPoint)
{
	if (!IsPointInsideDisc2D(fixedPoint,mobileDiscCenter,discRadius))
	{
		return false;
	}
	else
	{
		Vec2 fromPointToCenter = fixedPoint - mobileDiscCenter;
		float length = fromPointToCenter.GetLength() - discRadius;
		fromPointToCenter.SetLength(length);
		mobileDiscCenter = mobileDiscCenter + fromPointToCenter;

		return true;
	}

}


bool PushDiscOutOfFixedDisc2D(Vec2& mobileDiscCenter, float mobileDiscRadius, Vec2 const& fixedDiscCenter, float fixedDiscRadius)
{
	if (!DoDiscsOverlap(mobileDiscCenter, mobileDiscRadius, fixedDiscCenter, fixedDiscRadius))
	{
		return false;
	}
	else
	{
		Vec2 fullDisplacement = mobileDiscCenter - fixedDiscCenter;
		Vec2 mobieDisplacement = fullDisplacement;
		mobieDisplacement.SetLength(mobileDiscRadius);

		Vec2 fixedDisplacement = fullDisplacement;
		fixedDisplacement.SetLength(fixedDiscRadius);
		Vec2 displacement = mobieDisplacement + fixedDisplacement - fullDisplacement;
		mobileDiscCenter += displacement;
		return true;
	}
	
}

bool PushDiscOutOfFixedOBB2D(Vec2& mobileDiscCenter, float discRadius, OBB2 const& obb)
{
	Vec2 nearestPoint = GetNearestPointOnOBB2D(mobileDiscCenter, obb);
	if (!IsPointInsideDisc2D(nearestPoint,mobileDiscCenter, discRadius))
	{
		return false;
	}
	else
	{
		PushDiscOutOfFixedPoint2D(mobileDiscCenter, discRadius, nearestPoint);
		return true;
	}
}

bool PushDiscOutOfFixedCapsule2D(Vec2& mobileDiscCenter, float discRadius, Capsule2 const& cap)
{
	Vec2 nearestPoint = GetNearestPointOnCapsule2D(mobileDiscCenter, cap);
	if (!IsPointInsideDisc2D(nearestPoint, mobileDiscCenter, discRadius))
	{
		return false;
	}
	else
	{
		PushDiscOutOfFixedPoint2D(mobileDiscCenter, discRadius, nearestPoint);
		return true;
	}
}

// bool PushDiscOutOfFixedDiscXY3D(Vec3& mobileDiscCenter, float mobileDiscRadius, Vec3 const& fixedDiscCenter, float fixedDiscRadius)
// {
// 	Vec2 mobileCenter2D = Vec2(mobileDiscCenter.x, mobileDiscCenter.y);
// 	Vec2 fixedCenter2D = Vec2(fixedDiscCenter.x, fixedDiscCenter.y);
// 	if (!DoDiscsOverlap(mobileCenter2D, mobileDiscRadius, fixedCenter2D, fixedDiscRadius))
// 	{
// 		return false;
// 	}
// 	else
// 	{
// 		Vec2 fullDisplacement = mobileCenter2D - fixedCenter2D;
// 		Vec2 mobieDisplacement = fullDisplacement;
// 		mobieDisplacement.SetLength(mobileDiscRadius);
// 
// 		Vec2 fixedDisplacement = fullDisplacement;
// 		fixedDisplacement.SetLength(fixedDiscRadius);
// 		Vec2 displacement = mobieDisplacement + fixedDisplacement - fullDisplacement;
// 
// 		mobileDiscCenter.x += displacement.x;
// 		mobileDiscCenter.y += displacement.y;
// 		return true;
// 	}
// }

bool PushDiscsOutOfEachOther2D(Vec2& aCenter, float aRadius, Vec2& bCenter, float bRadius)
{
	if (!DoDiscsOverlap(aCenter, aRadius, bCenter, bRadius))
	{
		return false;
	}
	else 
	{
		Vec2 fullDisplacement = aCenter - bCenter;
		Vec2 aDisplacement = fullDisplacement;
		aDisplacement.SetLength(aRadius);
		Vec2 bDisplacement = fullDisplacement;
		bDisplacement.SetLength(bRadius);

		Vec2 displacement = aDisplacement + bDisplacement - fullDisplacement;
		aCenter += 0.5f * displacement;
		bCenter -= 0.5f * displacement;
		return true;
	}
	
}


// bool PushDiscsOutOfEachOtherXY3D(Vec3& aCenter, float aRadius, Vec3& bCenter, float bRadius)
// {
// 	Vec2 aCenter2D = Vec2(aCenter.x, aCenter.y);
// 	Vec2 bCenter2D = Vec2(bCenter.x, bCenter.y);
// 	if (!DoDiscsOverlap(aCenter2D, aRadius, bCenter2D, bRadius))
// 	{
// 		return false;
// 	}
// 	else
// 	{
// 		
// 		Vec2 fullDisplacement = aCenter2D - bCenter2D;
// 		Vec2 aDisplacement = fullDisplacement;
// 		aDisplacement.SetLength(aRadius);
// 		Vec2 bDisplacement = fullDisplacement;
// 		bDisplacement.SetLength(bRadius);
// 		Vec2 displacement = aDisplacement + bDisplacement - fullDisplacement;
// 		aCenter.x += 0.5f * displacement.x;
// 		aCenter.y += 0.5f * displacement.y;
// 		bCenter.x -= 0.5f * displacement.x;
// 		bCenter.y -= 0.5f * displacement.y;
// 		return true;
// 	}
// }

bool PushDiscOutOfFixedAABB2D(Vec2& mobileDiscCenter, float discRadius, AABB2 const& fixedBox)
{
	Vec2 nearestPosition = fixedBox.GetNearestPoint(mobileDiscCenter);
	if (GetDistance2D(nearestPosition, mobileDiscCenter)>= discRadius)
	{
		return false;
	}
	else
	{
		Vec2 displacement = mobileDiscCenter - nearestPosition;
		Vec2 radiusDisplacement = displacement;
		radiusDisplacement.SetLength(discRadius);
		Vec2 targetDisplacement = -displacement + radiusDisplacement;
		mobileDiscCenter += targetDisplacement;
		return true;
	}

}

// bool PushDiscOutOfFixedAABBXY3D(Vec3& mobileDiscCenter, float discRadius, AABB2 const& fixedBox)
// {
// 	Vec2 mobileCenterXY2D = Vec2(mobileDiscCenter.x, mobileDiscCenter.y);
// 	Vec2 nearestPosition = fixedBox.GetNearestPoint(mobileCenterXY2D);
// 	if (GetDistance2D(nearestPosition, mobileCenterXY2D) >= discRadius)
// 	{
// 		return false;
// 	}
// 	else
// 	{
// 		Vec2 displacement = mobileCenterXY2D - nearestPosition;
// 		Vec2 radiusDisplacement = displacement;
// 		radiusDisplacement.SetLength(discRadius);
// 		Vec2 targetDisplacement = -displacement + radiusDisplacement;
// 
// 		mobileDiscCenter.x += targetDisplacement.x;
// 		mobileDiscCenter.y += targetDisplacement.y;
// 
// 		return true;
// 	}
// 
// }

bool BounceDiscsOffFixedDisc2D(Vec2& posA, float radiusA, Vec2& velocityA, Vec2 const& fixedDiscCenter, float fixedDiscRadius, float elasticityA /*= 1.0f*/, float elasticityFixed/* = 1.0f*/)
{
	if (((posA.x - fixedDiscCenter.x) * (posA.x - fixedDiscCenter.x) + (posA.y - fixedDiscCenter.y) * (posA.y - fixedDiscCenter.y)) > (radiusA + fixedDiscRadius) * (radiusA + fixedDiscRadius))
	{
		return false;
	}
	float elasticity = elasticityA * elasticityFixed;
	Vec2 vecAtoFNormal = (fixedDiscCenter - posA).GetNormalized();
	Vec2 vecVAN = vecAtoFNormal * DotProduct2D(vecAtoFNormal, velocityA);
	Vec2 vecAV = velocityA - vecVAN;
	Vec2 vecFtoANormal = (posA - fixedDiscCenter).GetNormalized();
	Vec2 vecAtoFNew = vecFtoANormal * DotProduct2D(vecAtoFNormal, velocityA) * elasticity;
	//re-combine

	Vec2 newVelA = vecAV + vecAtoFNew;
	velocityA = newVelA;
	return true;
}


bool BounceDiscsOffFixedPoint2D(Vec2& posA, float radiusA, Vec2& velocityA, Vec2 const& fixedP, float elasticityA /*= 1.0f*/, float elasticityFixed /*= 1.0f*/)
{
	if (!IsPointInsideDisc2D(fixedP, posA, radiusA)) 
	{
		return false;
	}
	float elasticity = elasticityA * elasticityFixed;
	Vec2 vecAtoFNormal = (fixedP - posA).GetNormalized();
	Vec2 vecVAN = vecAtoFNormal * DotProduct2D(vecAtoFNormal, velocityA);
	Vec2 vecAV = velocityA - vecVAN;
	Vec2 vecFtoANormal = (posA - fixedP).GetNormalized();
	Vec2 vecAtoFNew = vecFtoANormal * DotProduct2D(vecAtoFNormal, velocityA) * elasticity;
	//Re-combine
	Vec2 newVelA = vecAV + vecAtoFNew;
	velocityA = newVelA;
	return true;
}

bool BounceDiscsOffFixedPointWithDivergenceCheck2D(Vec2& posA, float radiusA, Vec2& velocityA, Vec2 const& fixedP, float elasticityA /*= 1.0f*/, float elasticityFixed /*= 1.0f*/)
{
	if (!PushDiscOutOfFixedPoint2D(posA, radiusA, fixedP)) 
	{
		return false;
	}
// 	if (!IsPointInsideDisc2D(fixedP, posA, radiusA))
// 	{
// 		return false;
// 	}
	float deltaSeconds = 0.05f;
	float elasticity = elasticityA * elasticityFixed;
	Vec2 vecAtoFNormal = (fixedP - posA).GetNormalized();
	Vec2 vecVAN = vecAtoFNormal * DotProduct2D(vecAtoFNormal, velocityA);
	Vec2 vecAV = velocityA - vecVAN;
	Vec2 vecFtoANormal = (posA - fixedP).GetNormalized();
	Vec2 vecAtoFNew = vecFtoANormal * DotProduct2D(vecAtoFNormal, velocityA) * elasticity;
	//Re-combine
	Vec2 newVelA = vecAV + vecAtoFNew;
	
	//Divergence Check
	float distSquaredNextFrameIfNotChange = GetDistanceSquared2D(posA + velocityA * deltaSeconds, fixedP);
	float distSquaredNextFrameIfChange = GetDistanceSquared2D(posA + newVelA * deltaSeconds, fixedP);
	if (distSquaredNextFrameIfNotChange < distSquaredNextFrameIfChange) 
	{
		velocityA = newVelA;
		return true;
	}
	else
	{
		Vec2 vecVANLess = vecVAN * elasticity;
		Vec2 newVelALess = vecVANLess + vecAV;
		velocityA = newVelALess;
		return false;
	}
}

bool BounceDiscsOffEachOtherWithDivergenceCheck2D(Vec2& posA, float radiusA, Vec2& velocityA, Vec2& posB, float radiusB, Vec2& velocityB, float elasticityA /*= 1.0f*/, float elasticityB /*= 1.0f*/)
{
	if (!PushDiscsOutOfEachOther2D(posA, radiusA, posB, radiusB))
	{
		return false;
	}
	//Prepare
	float jointElasticity = elasticityA * elasticityB;

	Vec2 dispAToB = posB - posA;
	Vec2 collisionNormal = dispAToB.GetNormalized();
	float speedAlongNormalA= DotProduct2D(velocityA, collisionNormal);
	Vec2 velAlongNormalA = speedAlongNormalA * collisionNormal;
	Vec2 velAlongTangentA = velocityA - velAlongNormalA;						//What's left after Bounce

	float speedAlongNormalB = DotProduct2D(velocityB, collisionNormal);
	Vec2  velAlongNormalB = speedAlongNormalB * collisionNormal;
	Vec2 velAlongTangentB = velocityB - velAlongNormalB;
	//Check if they are convergent
	if (speedAlongNormalA > speedAlongNormalB) 
	{//exchange velocity
		velocityA = velAlongTangentA + (velAlongNormalB * jointElasticity);
		velocityB = velAlongTangentB + (velAlongNormalA * jointElasticity);
	}
	return true;
// 	if (PushDiscsOutOfEachOther2D(posA, radiusA, posB, radiusB))
// 	{
// 		Vec2 normal = posB - posA;
// 
// 		float elasticity = elasticityA * elasticityB;
// 
// 		Vec2 velocityAN = GetProjectedOnto2D(velocityA, normal) * elasticity;
// 		Vec2 velocityAT = velocityA - velocityAN;
// 
// 		Vec2 velocityBN = GetProjectedOnto2D(velocityB, normal) * elasticity;
// 		Vec2 velocityBT = velocityB - velocityBN;
// 
// 		// Divergence check
// 		if (DotProduct2D(velocityB, normal) - DotProduct2D(velocityA, normal) > 0.f)
// 		{
// 			velocityA = velocityAT + velocityAN;
// 			velocityB = velocityBT + velocityBN;
// 			return false;
// 		}
// 
// 		velocityA = velocityAT + velocityBN;
// 		velocityB = velocityBT + velocityAN;
// 		return true;
// 	}
// 	return false;
// 	if (DotProduct2D(velocityB, vecNNormal) - DotProduct2D(velocityA, vecNNormal) > 0.f)
// 	{
// 		velocityA = vecVAT + vecVeAN;
// 		velocityB = vecVBT + vecVeBN;
// 		return false;
// 	}
// 	velocityA = vecVAT + vecVeBN;
// 	velocityB = vecVBT + vecVeAN;
// 	return true;
}

bool BounceDiscsOffEachOther2D(Vec2& posA, float radiusA, Vec2& velocityA, Vec2& posB, float radiusB, Vec2& velocityB, float elasticityA /*= 1.0f*/, float elasticityB /*= 1.0f*/)
{
	if (((posA.x - posB.x) * (posA.x - posB.x) + (posA.y - posB.y) * (posA.y - posB.y)) > (radiusA + radiusB) * (radiusA + radiusB))
	{
		return false;
	}
	//Prepare
	float elasticity = elasticityA * elasticityB;

	Vec2 vecNInAB = posB - posA;
	Vec2 vecNNormal = vecNInAB.GetNormalized();
	Vec2 vecVeAN = DotProduct2D(velocityA, vecNNormal) * vecNNormal * elasticity;
	Vec2 vecVAT = velocityA - vecVeAN;						//What's left after Bounce

	Vec2 vecNNormalInBDIr = -vecNNormal;
	Vec2 vecVeBN = DotProduct2D(velocityB, vecNNormalInBDIr) * elasticity * vecNNormalInBDIr;
	Vec2 vecVBT = velocityB - vecVeBN;




	//Exchange and re-combine
	Vec2 velA = vecVAT + vecVeBN;
	Vec2 velB = vecVBT + vecVeAN;
	velocityA = velA;
	velocityB = velB;
	return true;

	//Radius?Position & ?
	//elasticity = elasticityA * elasticityB?
	
}

void TransformPosition2D(Vec2& posToTransform, float uniformScale, float rotationDegrees, Vec2 const& translation)
{

	//#TODO:Uniform Scaling
	//float temp_IniLength = sqrtf((posToTransform.x * posToTransform.x + posToTransform.y * posToTransform.y));
	posToTransform = posToTransform * uniformScale;
	//#TODO:2D Rotation
	posToTransform = posToTransform.GetRotatedDegrees(rotationDegrees);
	//#TODO:2D Translation
	posToTransform.x += translation.x;
	posToTransform.y += translation.y;
}

void TransformPosition2D(Vec2& posToTransform, Vec2 const& iBasis, Vec2 const& jBasis, Vec2 const& translation)
{
	posToTransform = posToTransform.x * iBasis + posToTransform.y * jBasis + translation;

}

void TransformPositionXY3D(Vec3& positionToTransform, float scaleXY, float zRotationDegrees, Vec2 const& translationXY)
{
	//#TODO:Scaling
	positionToTransform.x *= scaleXY;
	positionToTransform.y *= scaleXY;
	//#TODO:3D Rotation
	positionToTransform = positionToTransform.GetRotatedAboutZDegrees(zRotationDegrees);
	//#TODO:2D Translation
	positionToTransform.x += translationXY.x;
	positionToTransform.y += translationXY.y;
}

void TransformPositionXY3D(Vec3& posToTransform, Vec2 const& iBasis, Vec2 const& jBasis, Vec2 const& translation)
{
	Vec2 tempValue = Vec2(posToTransform.x, posToTransform.y);
	TransformPosition2D(tempValue, iBasis, jBasis, translation);
	posToTransform = Vec3(tempValue.x, tempValue.y, posToTransform.z);
}

Mat44 GetBillboardMatrix(BillBoardType billboardType, Mat44 const& cameraMatrix, const Vec3& billboardPosition, const Vec2& billboardScale /*= Vec2(1.f, 1.f)*/)
{

	Vec3 transformPosi = billboardPosition;

	//The camera or target basis and position
	Vec3 forwardTarget = cameraMatrix.GetIBasis3D();
	Vec3 leftTarget = cameraMatrix.GetJBasis3D();
	Vec3 upTarget = cameraMatrix.GetKBasis3D();
	Vec3 posiTarget = cameraMatrix.GetTranslation3D();

	Vec3 z_Axis = Vec3::Z_AXIS;
	Vec3 y_Axis = Vec3::Y_AXIS;
	Mat44 mat44;
	switch (billboardType)
	{
	case BillBoardType::WORLD_UP_CAMERA_FACING: 
	{
		Vec3 kBResult = Vec3::Z_AXIS;
		Vec3 iBResult = posiTarget - transformPosi;
		iBResult.z = 0.f;
		iBResult.Normalize();
		Vec3 jBResult = CrossProduct3D(kBResult, iBResult);
		jBResult.Normalize();
		mat44.SetIJKT3D(iBResult, jBResult, kBResult, billboardPosition);
		return mat44;
		break;

	}
	case BillBoardType::WORLD_UP_CAMERA_OPPOSING: 
	{
		mat44.SetIJKT3D(-forwardTarget, -leftTarget, upTarget, billboardPosition);
		return mat44;
		break;

	}
	case BillBoardType::FULL_CAMERA_FACING: 
	{
		Vec3 iBResult = posiTarget - transformPosi;
		iBResult.Normalize();
		Vec3 jBResult = Vec3::WORLD_ORIGIN;
		Vec3 kBResult = Vec3::WORLD_ORIGIN;
		if (fabsf(DotProduct3D(iBResult, z_Axis)) < 1.f)
		{
			jBResult = CrossProduct3D(z_Axis, iBResult);
			jBResult.Normalize();
			kBResult = CrossProduct3D(iBResult, jBResult);
		}
		else
		{
			kBResult = CrossProduct3D(iBResult, y_Axis);
			kBResult.Normalize();
			jBResult = CrossProduct3D(kBResult, iBResult);
			jBResult.Normalize();
		}
		mat44.SetIJK3D(iBResult, jBResult, kBResult);
		return mat44;
		break;
	}
	case BillBoardType::FULL_CAMERA_OPPOSING: 
	{
		mat44.SetIJKT3D(-forwardTarget, -leftTarget * billboardScale.x, upTarget * billboardScale.y, billboardPosition);
		return mat44;
		break;
	}
	case BillBoardType::NONE:
	{
		return mat44;
		break;
	}
	default:
		return mat44;
		break;
	}
}

float ComputeCubicBezier1D(float A, float B, float C, float D, float t)//Cubic = 3
{
	if (t > 1 || t < 0) return 0;
	float s = 1 - t;

	float EOnAB = s * A + t * B;
	float FOnBC = s * B + t * C;
	float GOnCD = s * C + t * D;

	float HOnEF = s * EOnAB + t * FOnBC;
	float IOnFG = s * FOnBC + t * GOnCD;

	float POnHI = s * HOnEF + t * IOnFG;

	return POnHI;
}

float ComputeQuinticBezier1D(float A, float B, float C, float D, float E, float F, float t)//Quintic = 5
{
	if (t > 1 || t < 0) return 0;
	float s = 1 - t;

	float GOnAB = s * A + t * B;
	float HOnBC = s * B + t * C;
	float IOnCD = s * C + t * D;
	float JOnDE = s * D + t * E;
	float KOnEF = s * E + t * F;

	float LOnGH = s * GOnAB + t * HOnBC;
	float MOnHI = s * HOnBC + t * IOnCD;
	float NOnIJ = s * IOnCD + t * JOnDE;
	float OOnJK = s * JOnDE + t * KOnEF;

	float POnLM = s * LOnGH + t * MOnHI;
	float QOnMN = s * MOnHI + t * NOnIJ;
	float ROnNO = s * NOnIJ + t * OOnJK;

	float SOnLM = s * POnLM + t * QOnMN;
	float TOnMN = s * QOnMN + t * ROnNO;

	float UOnLM = s * SOnLM + t * TOnMN;

	return UOnLM;
}

float SmoothStart2(float t)
{
	return t * t;
}

float SmoothStart3(float t)
{
	return t * t * t;
}

float SmoothStart4(float t)
{
	return  t * t * t * t;
}

float SmoothStart5(float t)
{
	return  t * t * t * t * t;
}

float SmoothStart6(float t)
{
	return  t * t * t * t * t * t;
}

float SmoothStop2(float t)
{
	float s = 1 - t;
	return (1 - SmoothStart2(s));
}

float SmoothStop3(float t)
{
	float s = 1 - t;
	return (1 - SmoothStart3(s));
}

float SmoothStop4(float t)
{
	float s = 1 - t;
	return (1 - SmoothStart4(s));
}

float SmoothStop5(float t)
{
	float s = 1 - t;
	return (1 - SmoothStart5(s));
}

float SmoothStop6(float t)
{
	float s = 1 - t;
	return (1 - SmoothStart6(s));
}

float SmoothStep3(float t)
{
	return ComputeCubicBezier1D(0.f, 0.f, 1.f, 1.f, t);
}

float SmoothStep5(float t)
{
	return ComputeQuinticBezier1D(0.f, 0.f, 0.f, 1.f, 1.f, 1.f, t);
}

float Hesitate3(float t)
{
	return ComputeCubicBezier1D(0.f, 1.f, 0.f, 1.f, t);
}

float Hesitate5(float t)
{
	return ComputeQuinticBezier1D(0.f, 1.f, 0.f, 1.f, 0.f, 1.f, t);
}



float GetClamped(float value, float minValue, float maxValue)
{
	if (value > maxValue) 
	{
		return maxValue;
	}
	else if (value<minValue)
		{
			return minValue;
		}
	else
	{
	    return value;
	}
}

int GetClamped(int value, int minValue, int maxValue)
{
	if (value > maxValue)
	{
		return maxValue;
	}
	else if (value < minValue)
	{
		return minValue;
	}
	else
	{
		return value;
	}
}

float GetClampedZeroToOne(float value)
{
	if (value > 1.f)
	{
		return 1.f;
	}
	else if (value < 0.f)
	{
		return 0.f;
	}
	else
	{
		return value;
	}
}

float Interpolate(float start, float end, float fractionTowardEnd)
{
	return fractionTowardEnd * (end - start) + start;
}

Rgba8 Interpolate(Rgba8 start, Rgba8 end, float fractionTowardEnd)
{
	if (start.r == end.r && start.g == end.g && start.b == end.b && start.a == end.a) 
	{
		return start;
	}
	unsigned char r = (unsigned char)Interpolate(start.r, end.r, fractionTowardEnd);
	unsigned char g = (unsigned char)Interpolate(start.g, end.g, fractionTowardEnd);
	unsigned char b = (unsigned char)Interpolate(start.b, end.b, fractionTowardEnd);
	unsigned char a = (unsigned char)Interpolate(start.a, end.a, fractionTowardEnd);

	return Rgba8(r, g, b, a);
}

float GetFractionWithinRange(float value, float rangeStart, float rangeEnd)
{
	return (value - rangeStart) / (rangeEnd - rangeStart);
}

float RangeMap(float inValue, float inStart, float inEnd, float outStart, float outEnd)
{
	float ratio = (inValue - inStart) / (inEnd - inStart);
	return ratio * (outEnd - outStart) + outStart;
}

float RangeMapClamped(float inValue, float inStart, float inEnd, float outStart, float outEnd)
{
	inValue = GetClamped(inValue, inStart, inEnd);
	float ratio = (inValue - inStart) / (inEnd - inStart);
	return ratio * (outEnd - outStart) + outStart;
}

int RoundDownToInt(float value)
{
	return 	int(floorf(value));
}

int* SwapIntArrayElement(int* array, int firstIndex, int secondIndex)
{
	int temp = array[firstIndex];
	array[firstIndex] = array[secondIndex];
	array[secondIndex] = temp;
	return array;
}
