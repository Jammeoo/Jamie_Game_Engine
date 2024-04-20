#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Plane2D.hpp"
#include "Engine/Math/Plane3D.hpp"
#include "Engine/Math/ConvexPoly2D.hpp"
#include "Engine/Math/ConvexPoly3D.hpp"
#include "Engine/Math/ConvexHull2D.hpp"
#include "Engine/Math/ConvexHull3D.hpp"
struct RaycastResult2D
{
	// Basic ray cast result information (required)
	bool	m_didImpact = false;
	float	m_impactDist = 0.f;
	float   m_exitDist = 0.f;
	Vec2	m_impactPos = Vec2(0.f, 0.f);
	Vec2	m_impactNormal = Vec2(0.f, 0.f);

	// Original ray cast information (optional)
	Vec2	m_rayFwdNormal = Vec2(0.f, 0.f);
	Vec2	m_rayStartPos = Vec2(0.f, 0.f);
	float	m_rayMaxLength = 1.f;
	Plane2D m_plane;
	//bool m_isTrue = false;
	bool m_isEntry = false;
};
RaycastResult2D RaycastVsDisc2D(Vec2 startPos, Vec2 fwdNormal, float maxDist, Vec2 discCenter, float discRadius);
RaycastResult2D RaycastVsPlane2D(Vec2 startPos, Vec2 fwdNormal, float maxDist, Plane2D const& plane);
void SortRaycastResultsBasedOnImpactDist(std::vector<RaycastResult2D>& rayResults);

RaycastResult2D RaycastVsConvexHull2D(Vec2 startPos, Vec2 fwdNormal, float maxDist, ConvexHull2D const& conH, std::vector<Vec2>& IntersectedPoints, bool needPoints = false);
struct RaycastResult3D
{
	// Basic ray cast result information (required)
	bool	m_didImpact = false;
	float	m_impactDist = 0.f;
	Vec3	m_impactPos = Vec3::WORLD_ORIGIN;
	Vec3	m_impactNormal = Vec3::WORLD_ORIGIN;

	// Original ray cast information (optional)
	Vec3	m_rayStartPos = Vec3::WORLD_ORIGIN;
	Vec3	m_rayFwdNormal = Vec3::WORLD_ORIGIN;
	float	m_rayMaxLength = 1.f;
};


//bool IsPointInsideOrOnDisc2D(const Vec2& point, const Vec2& discCenter, float discRadius);
RaycastResult3D RaycastVsCylinderZ3D(const Vec3& start, const Vec3& direction, float distance, const Vec2& center, float minZ, float maxZ, float radius);
RaycastResult3D RaycastVsPlane3D(const Vec3& startPos, const Vec3& fwdNormal, float maxDist, Plane3D const& plane);