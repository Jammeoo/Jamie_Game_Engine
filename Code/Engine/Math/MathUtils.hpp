#pragma once
//---------------------------------------------------------------------------------------------------------------------------------------------
//Forward type declarations
//
#include "Engine/Math/Vec2.hpp"

struct Vec3;
struct Vec4;
struct IntVec2;
struct AABB2;
struct OBB2;
struct Capsule2;
struct LineSegment2;
struct AABB3;
struct Rgba8;
struct Mat44;
struct Hexagon2D;

enum class BillBoardType
{
	NONE = -1,
	WORLD_UP_CAMERA_FACING,
	WORLD_UP_CAMERA_OPPOSING,
	FULL_CAMERA_FACING,
	FULL_CAMERA_OPPOSING,
	COUNT
};

//---------------------------------------------------------------------------------------------------------------------------------------------
//Clamp and Lerp
float GetClamped(float value, float minValue, float maxValue);
int GetClamped(int value, int minValue, int maxValue);
float GetClampedZeroToOne(float value);
float Interpolate(float start, float end, float fractionTowardEnd);
Rgba8 Interpolate(Rgba8 start, Rgba8 end, float fractionTowardEnd);

float GetFractionWithinRange(float value, float rangeStart, float rangeEnd);
float RangeMap(float inValue, float inStart, float inEnd, float outStart, float outEnd);
float RangeMapClamped(float inValue, float inStart, float inEnd, float outStart, float outEnd);
int	RoundDownToInt(float value);
int* SwapIntArrayElement(int* array, int firstIndex, int secondIndex);
//Angle utilities
float ConvertDegreesToRadians(float degrees);
float ConvertRadiansToDegrees(float radians);
float CosDegrees(float degrees);
float SinDegrees(float degrees);
float TanDegrees(float degrees);
float Atan2Degrees(float y, float x);
float GetShortestAngularDispDegrees(float startDegrees, float endDegrees);
float GetTurnedTowardDegrees(float currentDegrees, float goalDegrees, float maxDeltaDegrees);
float GetAngleDegreesBetweenVectors2D(Vec2 const& a, Vec2 const& b);
//Dot and Cross
float DotProduct2D(Vec2 const& a, Vec2 const& b);
float DotProduct3D(Vec3 const& a, Vec3 const& b);
float DotProduct4D(Vec4 const& a, Vec4 const& b);
float CrossProduct2D(Vec2 const& a, Vec2 const& b);
Vec3 CrossProduct3D(Vec3 const& a, Vec3 const& b);

float NormalizeByte(unsigned char byteValue);
unsigned char DenormalizeByte(float zeroToOne);
//--------------------------------------------------------------------------------------------------------------------------------------------
//Basic 2D & 3D utilities
//
float GetDistance2D(Vec2 const& positionA, Vec2 const& positionB);
float GetDistanceSquared2D(Vec2 const& positionA, Vec2 const& positionB);
float GetDistance3D(Vec3 const& positionA, Vec3 const& positionB);
float GetDistanceSquared3D(Vec3 const& positionA, Vec3 const& positionB);
float GetDistanceXY3D(Vec3 const& positionA, Vec3 const& positionB);
float GetDistanceXYSquared3D(Vec3 const& positionA, Vec3 const& positionB);
int	  GetTaxicabDistance2D(IntVec2 const& pointA, IntVec2 const& pointB);
float GetProjectedLength2D(Vec2 const& vectorToProject, Vec2 const& vectorToProjectOnto); // Works if Vectors not normalized
Vec2 const GetProjectedOnto2D(Vec2 const& vectorToProject, Vec2 const& vectorToProjectOnto); // Works if Vectors not normalized
//--------------------------------------------------------------------------------------------------------------------------------------------
//Geometric query utilities
//
bool IsPointInsideDisc2D(Vec2 const& point, Vec2 const& discCenter, float discRadius);
bool IsPointInsideAABB2D(Vec2 const& point,AABB2 const& box);
bool IsPointInsideCapsule2D(Vec2 const& point, Capsule2 const& capsule);
bool IsPointInsideCapsule2D(Vec2 const& point, Vec2 const& boneStart, Vec2 const& boneEnd, float radius);
bool IsPointInsideOBB2D(Vec2 const& point, OBB2 const& orientedBox);
bool IsPointInsideDirectedSector2D(Vec2 const& point, Vec2 const& sectorTip, Vec2 const& sectorForwardNormal, float sectorApertureDegrees, float sectorRadius);
bool IsPointInsideOrientedSector2D(Vec2 const& point, Vec2 const& sectorTip, float sectorForwardDegrees, float sectorApertureDegrees, float sectorRadius);
bool IsPointInsideCapsule3D(Vec3 const& point, Vec3 const& boneStart, Vec3 const& boneEnd, float radius);
bool IsPointInsideSphere3D(Vec3 const& point, Vec3 const& sphereCenter, float sphereRadius);
bool IsPointInsideHexagon2D(Vec3 const& point, Hexagon2D const& hexgon);


bool DoDiscsOverlap(Vec2 const& centerA, float radiusA, Vec2 const& centerB, float radiusB);
bool DoSpheresOverlap(Vec3 const& centerA, float radiusA, Vec3 const& centerB, float radiusB);
bool DoAABB3DOverlap(AABB3 const& first, AABB3 const& second);
bool DoesAABBOverLapWithOBB2D(AABB2 const& aabb, OBB2 const& obb);
bool DoesOBBOverlapWithDisc(OBB2 const& obb, Vec2 const& centerA, float radius);

Vec2 GetNearestPointOnDisc2D(Vec2 const& referencePosition, Vec2 const& discCenter, float discRadius);
Vec2 GetNearestPointOnAABB2D(Vec2 const& referencePos, AABB2& box);
Vec2 GetNearestPointOnInfiniteLine2D(Vec2 const& referencePos, LineSegment2 const& infiniteLine);
Vec2 GetNearestPointOnInfiniteLine2D(Vec2 const& referencePos, Vec2 const& pointOnLine, Vec2 const& anotherPointOnLine);
Vec2 GetNearestPointOnLineSegment2D(Vec2 const& referencePos, LineSegment2 const& infiniteLine);
Vec2 GetNearestPointOnLineSegment2D(Vec2 const& referencePos, Vec2 const& pointOnLine, Vec2 const& anotherPointOnLine);
Vec2 GetNearestPointOnCapsule2D(Vec2 const& referencePos, Capsule2 const& capsule);
Vec2 GetNearestPointOnCapsule2D(Vec2 const& referencePos, Vec2 const& boneStart, Vec2 const& boneEnd, float radius);
Vec2 GetNearestPointOnOBB2D(Vec2 const& referencePos, OBB2 const& orientedBox);
bool PushDiscOutOfFixedPoint2D(Vec2& mobileDiscCenter, float discRadius, Vec2 const& fixedPoint);
bool PushDiscOutOfFixedDisc2D(Vec2& mobileDiscCenter, float mobileDiscRadius, Vec2 const& fixedDiscCenter, float fixedDiscRadius);
bool PushDiscOutOfFixedOBB2D(Vec2& mobileDiscCenter, float discRadius, OBB2 const& obb);
bool PushDiscOutOfFixedCapsule2D(Vec2& mobileDiscCenter, float discRadius, Capsule2 const& cap);
bool PushDiscsOutOfEachOther2D(Vec2& aCenter, float aRadius, Vec2& bCenter, float bRadius);

bool PushDiscOutOfFixedAABB2D(Vec2& mobileDiscCenter, float discRadius, AABB2 const& fixedBox);
// bool PushDiscOutOfFixedAABBXY3D(Vec3& mobileDiscCenter, float discRadius, AABB2 const& fixedBox);
bool BounceDiscsOffFixedDisc2D(Vec2& posA, float radiusA, Vec2& velocityA, Vec2 const& fixedDiscCenter, float fixedDiscRadius, float elasticityA = 1.0f, float elasticityFixed = 1.0f);
bool BounceDiscsOffFixedPoint2D(Vec2& posA, float radiusA, Vec2& velocityA, Vec2 const& fixedP, float elasticityA = 1.0f, float elasticityFixed = 1.0f);
bool BounceDiscsOffFixedPointWithDivergenceCheck2D(Vec2& posA, float radiusA, Vec2& velocityA, Vec2 const& fixedP, float elasticityA = 1.0f, float elasticityFixed = 1.0f);

// bool BounceDiscOffFixedCapsule(Vec2& posA, float radiusA, Vec2& velocityA, Capsule2 const& cap, float elasticityA = 1.0f, float elasticityFixed = 1.0f);
// bool BounceDiscOffFixedObb(Vec2& posA, float radiusA, Vec2& velocityA, OBB2 const& obb, float elasticityA = 1.0f, float elasticityFixed = 1.0f);

bool BounceDiscsOffEachOtherWithDivergenceCheck2D(Vec2& posA, float radiusA, Vec2& velocityA, Vec2& posB, float radiusB, Vec2& velocityB, float elasticityA = 1.0f, float elasticityB = 1.0f);
bool BounceDiscsOffEachOther2D(Vec2& posA, float radiusA, Vec2& velocityA, Vec2& posB, float radiusB, Vec2& velocityB, float elasticityA = 1.0f, float elasticityB = 1.0f);
//--------------------------------------------------------------------------------------------------------------------------------------------
//Transform utilities
//
void TransformPosition2D(Vec2& posToTransform, float uniformScale, float rotationDegrees, Vec2 const& translation);
void TransformPosition2D(Vec2& posToTransform, Vec2 const& iBasis, Vec2 const& jBasis, Vec2 const& translation);
void TransformPositionXY3D(Vec3& positionToTransform, float scaleXY, float zRotationDegrees, Vec2 const& translationXY);
void TransformPositionXY3D(Vec3& posToTransform, Vec2 const& iBasis, Vec2 const& jBasis, Vec2 const& translation);
//--------------------------------------------------------------------------------------------------------------------------------------------
//Distance & Projections utilities
//

Mat44 GetBillboardMatrix(BillBoardType billboardType, Mat44 const& cameraMatrix, const Vec3& billboardPosition, const Vec2& billboardScale = Vec2(1.f, 1.f));

//Easing, Curves, and Splines  0 <= t <= 1
	//Basement
float ComputeCubicBezier1D(float A, float B, float C, float D, float t);
float ComputeQuinticBezier1D(float A, float B, float C, float D, float E, float F, float t);

	//Easing 
float SmoothStart2(float t);//EaseInQuadratic
float SmoothStart3(float t);//EaseInCubic
float SmoothStart4(float t);//EaseInQuartic
float SmoothStart5(float t);//EaseInQuintic
float SmoothStart6(float t);//EaseIn6thOrder

float SmoothStop2(float t);//EaseOutQuadratic
float SmoothStop3(float t);//EaseOutCubic
float SmoothStop4(float t);//EaseOutQuartic
float SmoothStop5(float t);//EaseOutQuintic
float SmoothStop6(float t);//EaseOut6thOrder

float SmoothStep3(float t);
float SmoothStep5(float t);
float Hesitate3(float t);
float Hesitate5(float t);
