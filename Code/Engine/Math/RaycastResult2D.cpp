#include "Engine/Math/RaycastResult2D.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/FloatRange.hpp"
#include <math.h>
RaycastResult2D RaycastVsDisc2D(Vec2 startPos, Vec2 fwdNormal, float maxDist, Vec2 discCenter, float discRadius)
{
	if (IsPointInsideDisc2D(startPos,discCenter,discRadius)) 
	{
		RaycastResult2D raycastHitResult;
		raycastHitResult.m_didImpact = true;
		raycastHitResult.m_impactDist = 0.f;
		raycastHitResult.m_exitDist = 0.f;
		raycastHitResult.m_impactPos = startPos;
		raycastHitResult.m_impactNormal = -fwdNormal;
		raycastHitResult.m_rayFwdNormal = fwdNormal;
		raycastHitResult.m_rayStartPos = startPos;
		raycastHitResult.m_rayMaxLength = maxDist;
		return raycastHitResult;
	}
	Vec2 endPoint = startPos + fwdNormal * maxDist;
	Vec2 iBase = fwdNormal;
	Vec2 jBase = iBase.GetRotated90Degrees();
	Vec2 startToCenterDis = discCenter - startPos;
	float startCenteriBase = DotProduct2D(startToCenterDis, iBase);
	float startCenterjBase = DotProduct2D(startToCenterDis, jBase);

	RaycastResult2D raycastMissResult;
	raycastMissResult.m_didImpact = false;
	raycastMissResult.m_impactDist = 0.f;
	raycastMissResult.m_impactPos = Vec2(0.f, 0.f);
	raycastMissResult.m_impactNormal = Vec2(0.f, 0.f);
	raycastMissResult.m_rayFwdNormal = fwdNormal;
	raycastMissResult.m_rayStartPos = startPos;
	raycastMissResult.m_rayMaxLength = maxDist;

	//Case 1:
	if (startCenterjBase >= discRadius || startCenterjBase <= -discRadius)
	{
		return raycastMissResult;
	}
	//Case 2:
	if (startCenteriBase <= -discRadius) 
	{
		return raycastMissResult;
	}
	//Case 3:
	if (startCenteriBase >= maxDist + discRadius) 
	{
		return raycastMissResult;
	}
	//Case 4:
	if ((discRadius * discRadius - startCenterjBase * startCenterjBase) <= 0.f) 
	{
		return raycastMissResult;
	}
	RaycastResult2D raycastHitResult;
	float a = sqrtf(discRadius * discRadius - startCenterjBase * startCenterjBase);

	//Case 5:
	if (startCenteriBase - a >= maxDist) 
	{
		return raycastMissResult;
	}
	raycastHitResult.m_didImpact = true;
	raycastHitResult.m_impactDist = fabsf(startCenteriBase - a);
	raycastHitResult.m_impactPos = startPos + (raycastHitResult.m_impactDist * iBase);
	raycastHitResult.m_exitDist = fabsf(startCenteriBase + a);
	raycastHitResult.m_impactNormal = (raycastHitResult.m_impactPos - discCenter).GetNormalized();
	raycastHitResult.m_rayFwdNormal = fwdNormal;
	raycastHitResult.m_rayStartPos = startPos;
	raycastHitResult.m_rayMaxLength = maxDist;

	return raycastHitResult;
}

RaycastResult2D RaycastVsPlane2D(Vec2 startPos, Vec2 fwdNormal, float maxDist, Plane2D const& plane)
{
	Vec2 endPos = startPos + fwdNormal * maxDist;
	float altitudeStart = DotProduct2D(startPos, plane.m_normal) - plane.m_distFromOrigin;
	float altitudeEnd = DotProduct2D(endPos, plane.m_normal) - plane.m_distFromOrigin;

	if (altitudeStart * altitudeEnd >= 0) 
	{
		RaycastResult2D missResult;
		missResult.m_didImpact = false;
		return missResult;
	}

	//Similar triangle
	float fwdNormalOntoPlaneNorm = DotProduct2D(fwdNormal, plane.m_normal);
	float impactDist = (-(altitudeStart / fwdNormalOntoPlaneNorm));
	RaycastResult2D hitResult;
	hitResult.m_didImpact = true;
	hitResult.m_impactDist = impactDist;
	hitResult.m_impactPos = startPos + fwdNormal * impactDist;
	hitResult.m_impactNormal = plane.m_normal;
	if (altitudeStart < 0.f) 
	{
		hitResult.m_impactNormal = -plane.m_normal;
	}
	hitResult.m_plane = plane;
	hitResult.m_isEntry = (DotProduct2D(fwdNormal,plane.m_normal) < 0.f);
	return hitResult;
}


void SortRaycastResultsBasedOnImpactDist(std::vector<RaycastResult2D>& rayResults)
{
	if ((int)rayResults.size() == 1) 
	{
		return;
	}
	if ((int)rayResults.size() < 3) 
	{
		if (rayResults[0].m_impactDist > rayResults[1].m_impactDist) 
		{
			RaycastResult2D temp = rayResults[0];
			rayResults[0] = rayResults[1];
			rayResults[1] = temp;
		}

		return;
	}

	bool swapped = false;
	do {
		swapped = false;
		for (int i = 0; i < (int)rayResults.size() - 1; i++) {
			if (rayResults[i].m_impactDist > rayResults[i + 1].m_impactDist) {
				//Swap two result in the ray results
				RaycastResult2D temp = rayResults[i];
				rayResults[i] = rayResults[i + 1];
				rayResults[i + 1] = temp;
				swapped = true;
			}
		}
	} while (swapped);

// 	for (int j = 0; j < (int)rayResults.size(); j++) 
// 	{
// 		RaycastResult2D& localBestResult = rayResults[j];
// 		float localBestDist = localBestResult.m_impactDist;
// 		for (int i = 1; i < (int)rayResults.size() - 1; i++) 
// 		{
// 			RaycastResult2D&  curResult = rayResults[i];
// 			float curBestDist = curResult.m_impactDist;
// 
// 			if (localBestDist > curBestDist) 
// 			{
// 				//Swap two result in the ray results
// 				RaycastResult2D temp = localBestResult;
// 				localBestResult = curResult;
// 				curResult = temp;
// 			}
// 		}
// 	}
}


RaycastResult2D RaycastVsConvexHull2D(Vec2 startPos, Vec2 fwdNormal, float maxDist, ConvexHull2D const& conH, std::vector<Vec2>& IntersectedPoints, bool needPoints /*= false*/)
{
	std::vector<Plane2D> planes;
	planes = conH.GetPlanes();

	std::vector<RaycastResult2D> rayResults;
	for (int planeIndex = 0; planeIndex < (int)planes.size(); planeIndex++)
	{
		Plane2D curPlane = planes[planeIndex];
		RaycastResult2D curRayResult = RaycastVsPlane2D(startPos, fwdNormal, maxDist, curPlane);
		if (curRayResult.m_didImpact)
		{
			rayResults.push_back(curRayResult);
		}
	}

	if (needPoints) 
	{
		for (int resultIndex = 0; resultIndex < (int)rayResults.size(); resultIndex++)
		{
			Vec2 intersectedPoint = rayResults[resultIndex].m_impactPos;
			IntersectedPoints.push_back(intersectedPoint);
		}
	}
	RaycastResult2D miss;
	miss.m_didImpact = false;
	if (rayResults.empty())
	{
		return miss;
	}
	SortRaycastResultsBasedOnImpactDist(rayResults);
	bool hasExit = false;
	RaycastResult2D potentialRayResult;

	for (int resultIndex = 0; resultIndex < (int)rayResults.size(); resultIndex++)
	{
		if (rayResults[resultIndex].m_isEntry)
		{
			if (hasExit)
			{
				return miss;//Entry after Exit
			}
			else
			{
				potentialRayResult = rayResults[resultIndex];
			}
		}
		else
		{
			hasExit = true;
		}
	}

	//Getting the index of result plane and remove the plane
	int potentialIndex = 0;
	if (potentialRayResult.m_didImpact)
	{
		for (int i = 0; i < (int)planes.size(); i++)
		{
			if (planes[i].m_normal == potentialRayResult.m_plane.m_normal &&
				planes[i].m_distFromOrigin == potentialRayResult.m_plane.m_distFromOrigin)
			{
				potentialIndex = i;
			}
		}
	}

	if (!hasExit)//No exit
	{
		//check whether last entry point is the result
		//Last pass of check

		planes.erase(planes.begin() + potentialIndex);
		for (int planeIndex = 0; planeIndex < (int)planes.size(); planeIndex++)
		{
			if (!planes[planeIndex].IsPointInsidePlant2D(potentialRayResult.m_impactPos))
			{
				return miss;
			}
		}
		return potentialRayResult;
	}
	else
	{
		if (potentialRayResult.m_didImpact)
		{
			//Last pass of check
			planes.erase(planes.begin() + potentialIndex);
			for (int planeIndex = 0; planeIndex < (int)planes.size(); planeIndex++)
			{
				if (!planes[planeIndex].IsPointInsidePlant2D(potentialRayResult.m_impactPos))
				{
					return miss;
				}
			}
			return potentialRayResult;
		}
	}
	return miss;
}

RaycastResult3D RaycastVsCylinderZ3D(const Vec3& start, const Vec3& direction, float distance, const Vec2& center, float minZ, float maxZ, float radius)
{
	RaycastResult3D raycastMiss3D;
	RaycastResult3D raycastResult3D;
	//if the ray start is inside the cylinder
	if ((start.z >= minZ && start.z <= maxZ) && IsPointInsideDisc2D(Vec2(start.x, start.y), center, radius)) 
	{
		raycastResult3D.m_didImpact = true;
		raycastResult3D.m_impactDist = 0.f;
		raycastResult3D.m_impactPos = start;
		raycastResult3D.m_impactNormal = start + direction;
		raycastResult3D.m_rayStartPos = start;
		raycastResult3D.m_rayFwdNormal = direction;
		raycastResult3D.m_rayMaxLength = distance;
		return raycastResult3D;
	}

	//[Zero check]if the ray intersects the x y - disc at a position between the cylinder min and max z
	if (direction.z == 0.f) 
	{
		if (start.z >= minZ && start.z <= maxZ) 
		{
			Vec2 dirXY = Vec2(direction.x, direction.y).GetNormalized();
			float distance2D = distance * Vec2(direction.x, direction.y).GetLength();
			RaycastResult2D rResultXY2D = RaycastVsDisc2D(Vec2(start.x, start.y), dirXY, distance2D, center, radius);
			raycastResult3D.m_didImpact = rResultXY2D.m_didImpact;
			raycastResult3D.m_impactDist = rResultXY2D.m_impactDist;
			raycastResult3D.m_impactPos = Vec3(rResultXY2D.m_impactPos.x, rResultXY2D.m_impactPos.y, start.z);
			raycastResult3D.m_impactNormal = Vec3(rResultXY2D.m_impactNormal.x, rResultXY2D.m_impactNormal.y, 0.f);
			raycastResult3D.m_rayStartPos = start;
			raycastResult3D.m_rayFwdNormal = Vec3(rResultXY2D.m_rayFwdNormal.x, rResultXY2D.m_rayFwdNormal.y, 0.f);
			raycastResult3D.m_rayMaxLength = distance;
			return raycastResult3D;
		}
		else
		{
			return raycastMiss3D;
		}
	}
	
	if (direction.z > 0) 
	{
		float starter = GetClamped(start.z, minZ, maxZ);
		float tStartZ = (starter - start.z) / direction.z;
		float tEndZ = (maxZ - start.z) / direction.z;

		Vec2 dirXY = Vec2(direction.x, direction.y).GetNormalized();
		float distance2D = distance * Vec2(direction.x, direction.y).GetLength();
		RaycastResult2D rResultXY2D = RaycastVsDisc2D(Vec2(start.x, start.y), dirXY, distance2D, center, radius);

		if (!rResultXY2D.m_didImpact) return raycastMiss3D;				//Raycast never gonna hit the disc, overlapping with Disc

		float enterDiscDist = rResultXY2D.m_impactDist;
		float escapeDiscDist = rResultXY2D.m_exitDist;

		float tStartDisc = enterDiscDist / Vec2(direction.x, direction.y).GetLength();
		float tEndDisc = escapeDiscDist / Vec2(direction.x, direction.y).GetLength();

		FloatRange timeZ = FloatRange(tStartZ, tEndZ);
		FloatRange timeDisc = FloatRange(tStartDisc, tEndDisc);
		if (tStartDisc == 0.f && tEndDisc == 0.f) 
		{
			timeDisc = FloatRange(-99.f, 99.f);
		}
		if (!timeZ.IsOverlappingWith(timeDisc)) 
		{
			return raycastMiss3D;										//Raycast won't hit the Z and the Disc at same time
		}
		bool case1 = timeDisc.IsOnRange(timeZ.m_min) && timeDisc.IsOnRange(timeZ.m_max);
		bool case2 = timeDisc.m_min >= timeZ.m_min && timeDisc.m_max >= timeZ.m_max;
		bool case3 = timeDisc.m_min < timeZ.m_min&& timeDisc.m_max < timeZ.m_max;
		bool case4 = timeZ.IsOnRange(timeDisc.m_min) && timeZ.IsOnRange(timeDisc.m_max);

		if (case1) 
		{
			float timeEnter = timeZ.m_min;
			
			raycastResult3D.m_didImpact = true;
			raycastResult3D.m_impactDist = timeEnter;
			raycastResult3D.m_impactPos = start + timeEnter * direction;
			raycastResult3D.m_impactNormal = -Vec3::Z_AXIS;
			raycastResult3D.m_rayStartPos = start;
			raycastResult3D.m_rayFwdNormal = direction;
			raycastResult3D.m_rayMaxLength = distance;
			return raycastResult3D;
		}
		if (case2) 
		{
			float timeEnter = timeDisc.m_min;

			raycastResult3D.m_didImpact = rResultXY2D.m_didImpact;
			raycastResult3D.m_impactDist = timeEnter;
			raycastResult3D.m_impactPos = Vec3(rResultXY2D.m_impactPos.x, rResultXY2D.m_impactPos.y, start.z + timeEnter * direction.z);
			raycastResult3D.m_impactNormal = Vec3(rResultXY2D.m_impactNormal.x, rResultXY2D.m_impactNormal.y, 0.f);
			raycastResult3D.m_rayStartPos = start;
			raycastResult3D.m_rayFwdNormal = direction;
			raycastResult3D.m_rayMaxLength = distance;
			return raycastResult3D;
		}

		if (case3) 
		{
			float timeEnter = timeZ.m_min;

			raycastResult3D.m_didImpact = true;
			raycastResult3D.m_impactDist = timeEnter;
			raycastResult3D.m_impactPos = start + timeEnter * direction;
			raycastResult3D.m_impactNormal = -Vec3::Z_AXIS;
			raycastResult3D.m_rayStartPos = start;
			raycastResult3D.m_rayFwdNormal = direction;
			raycastResult3D.m_rayMaxLength = distance;
			return raycastResult3D;
		}

		if (case4) 
		{
			float timeEnter = timeDisc.m_min;
// 			Vec2 dirXY = Vec2(direction.x, direction.y).GetNormalized();
// 			float distance2D = distance * Vec2(direction.x, direction.y).GetLength();
// 			RaycastResult2D rResultXY2D = RaycastVsDisc2D(Vec2(start.x, start.y), dirXY, distance2D, center, radius);
			raycastResult3D.m_didImpact = rResultXY2D.m_didImpact;
			raycastResult3D.m_impactDist = timeEnter;
			raycastResult3D.m_impactPos = Vec3(rResultXY2D.m_impactPos.x, rResultXY2D.m_impactPos.y, start.z + timeEnter * direction.z);
			raycastResult3D.m_impactNormal = Vec3(rResultXY2D.m_impactNormal.x, rResultXY2D.m_impactNormal.y, 0.f);
			raycastResult3D.m_rayStartPos = start;
			raycastResult3D.m_rayFwdNormal = direction;
			raycastResult3D.m_rayMaxLength = distance;
			return raycastResult3D;
		}
	}

	if (direction.z < 0)
	{
		float starter = GetClamped(start.z, minZ, maxZ);
		float tStartZ = (starter - start.z) / direction.z;
		float tEndZ = (minZ - start.z) / direction.z;

		Vec2 dirXY = Vec2(direction.x, direction.y).GetNormalized();
		float distance2D = distance * Vec2(direction.x, direction.y).GetLength();
		RaycastResult2D rResultXY2D = RaycastVsDisc2D(Vec2(start.x, start.y), dirXY, distance2D, center, radius);

		if (!rResultXY2D.m_didImpact) return raycastMiss3D;				//Raycast never gonna hit the disc, overlapping with Disc

// 		Vec2 enterDiscPosi = rResultXY2D.m_impactPos;
// 		Vec2 escapeDiscPosi = enterDiscPosi + DotProduct2D(dirXY, (enterDiscPosi - center)) * dirXY * 2.f;
		float enterDiscDist = rResultXY2D.m_impactDist;
		float escapeDiscDist = rResultXY2D.m_exitDist;

		float tStartDisc = enterDiscDist / Vec2(direction.x, direction.y).GetLength();
		float tEndDisc = escapeDiscDist / Vec2(direction.x, direction.y).GetLength();

		FloatRange timeZ = FloatRange(tStartZ, tEndZ);
		FloatRange timeDisc = FloatRange(tStartDisc, tEndDisc);
		if (tStartDisc == 0.f && tEndDisc == 0.f)
		{
			timeDisc = FloatRange(-99.f, 99.f);
		}

		if (!timeZ.IsOverlappingWith(timeDisc))
		{
			return raycastMiss3D;										//Raycast won't hit the Z and the Disc at same time
		}
		bool case1 = timeDisc.IsOnRange(timeZ.m_min) && timeDisc.IsOnRange(timeZ.m_max);
		bool case2 = timeDisc.m_min >= timeZ.m_min && timeDisc.m_max >= timeZ.m_max;
		bool case3 = timeDisc.m_min < timeZ.m_min&& timeDisc.m_max < timeZ.m_max;
		bool case4 = timeZ.IsOnRange(timeDisc.m_min) && timeZ.IsOnRange(timeDisc.m_max);

		if (case1)
		{
			float timeEnter = timeZ.m_min;

			raycastResult3D.m_didImpact = true;
			raycastResult3D.m_impactDist = timeEnter;
			raycastResult3D.m_impactPos = start + timeEnter * direction;
			raycastResult3D.m_impactNormal = Vec3::Z_AXIS;
			raycastResult3D.m_rayStartPos = start;
			raycastResult3D.m_rayFwdNormal = direction;
			raycastResult3D.m_rayMaxLength = distance;
			return raycastResult3D;
		}
		if (case2)
		{
			float timeEnter = timeDisc.m_min;

			raycastResult3D.m_didImpact = rResultXY2D.m_didImpact;
			raycastResult3D.m_impactDist = timeEnter;
			raycastResult3D.m_impactPos = Vec3(rResultXY2D.m_impactPos.x, rResultXY2D.m_impactPos.y, start.z + timeEnter * direction.z);
			raycastResult3D.m_impactNormal = Vec3(rResultXY2D.m_impactNormal.x, rResultXY2D.m_impactNormal.y, 0.f);
			raycastResult3D.m_rayStartPos = start;
			raycastResult3D.m_rayFwdNormal = direction;
			raycastResult3D.m_rayMaxLength = distance;
			return raycastResult3D;
		}

		if (case3)
		{
			float timeEnter = timeZ.m_min;

			raycastResult3D.m_didImpact = true;
			raycastResult3D.m_impactDist = timeEnter;
			raycastResult3D.m_impactPos = start + timeEnter * direction;
			raycastResult3D.m_impactNormal = Vec3::Z_AXIS;
			raycastResult3D.m_rayStartPos = start;
			raycastResult3D.m_rayFwdNormal = direction;
			raycastResult3D.m_rayMaxLength = distance;
			return raycastResult3D;
		}

		if (case4)
		{
			float timeEnter = timeDisc.m_min;
// 			Vec2 dirXY = Vec2(direction.x, direction.y).GetNormalized();
// 			float distance2D = distance * Vec2(direction.x, direction.y).GetLength();
// 			RaycastResult2D rResultXY2D = RaycastVsDisc2D(Vec2(start.x, start.y), dirXY, distance2D, center, radius);
			raycastResult3D.m_didImpact = rResultXY2D.m_didImpact;
			raycastResult3D.m_impactDist = timeEnter;
			raycastResult3D.m_impactPos = Vec3(rResultXY2D.m_impactPos.x, rResultXY2D.m_impactPos.y, start.z + timeEnter * direction.z);
			raycastResult3D.m_impactNormal = Vec3(rResultXY2D.m_impactNormal.x, rResultXY2D.m_impactNormal.y, 0.f);
			raycastResult3D.m_rayStartPos = start;
			raycastResult3D.m_rayFwdNormal = direction;
			raycastResult3D.m_rayMaxLength = distance;
			return raycastResult3D;
		}
	}

	return raycastMiss3D;
}

RaycastResult3D RaycastVsPlane3D(const Vec3& startPos, const Vec3& fwdNormal, float maxDist, Plane3D const& plane)
{
	Vec3 endPos = startPos + fwdNormal * maxDist;
	float altitudeStart = DotProduct3D(startPos, plane.m_normal) - plane.m_distFromOrigin;
	float altitudeEnd = DotProduct3D(endPos, plane.m_normal) - plane.m_distFromOrigin;

	if (altitudeStart * altitudeEnd >= 0)
	{
		RaycastResult3D missResult;
		missResult.m_didImpact = false;
		return missResult;
	}

	//Similar triangle
	float fwdNormalOntoPlaneNorm = DotProduct3D(fwdNormal, plane.m_normal);
	float impactDist = (-(altitudeStart / fwdNormalOntoPlaneNorm));
	RaycastResult3D hitResult;
	hitResult.m_didImpact = true;
	hitResult.m_impactDist = impactDist;
	hitResult.m_impactPos = startPos + fwdNormal * impactDist;
	hitResult.m_impactNormal = plane.m_normal;
	if (altitudeStart < 0.f)
	{
		hitResult.m_impactNormal = -plane.m_normal;
	}
	return hitResult;
}

// [Zero check]if the ray intersects the x y - disc at a position between the cylinder min and max z
// if (direction.z == 0.f)
// {
// 	if (start.z >= minZ && start.z <= maxZ)
// 	{
// 		Vec2 dirXY = Vec2(direction.x, direction.y).GetNormalized();
// 		float distance2D = distance * Vec2(direction.x, direction.y).GetLength();
// 		RaycastResult2D rResultXY2D = RaycastVsDisc2D(Vec2(start.x, start.y), dirXY, distance2D, center, radius);
// 		raycastResult3D.m_didImpact = rResultXY2D.m_didImpact;
// 		raycastResult3D.m_impactDist = rResultXY2D.m_impactDist;
// 		raycastResult3D.m_impactPos = Vec3(rResultXY2D.m_impactPos.x, rResultXY2D.m_impactPos.y, start.z);
// 		raycastResult3D.m_impactNormal = Vec3(rResultXY2D.m_impactNormal.x, rResultXY2D.m_impactNormal.y, 0.f);
// 		raycastResult3D.m_rayStartPos = Vec3(rResultXY2D.m_rayStartPos.x, rResultXY2D.m_rayStartPos.y, start.z);
// 		raycastResult3D.m_rayFwdNormal = Vec3(rResultXY2D.m_rayFwdNormal.x, rResultXY2D.m_rayFwdNormal.y, 0.f);
// 		raycastResult3D.m_rayMaxLength = rResultXY2D.m_rayMaxLength;
// 		return raycastResult3D;
// 	}
// 	else
// 	{
// 		return raycastMiss3D;
// 	}
// }
// 
// if( direction.z>0 )
// if the ray intersects the plane at the cylinder max z 
// float tMaxZResult = fabsf(maxZ - start.z) / fabsf(direction.z * distance);
// if (tMaxZResult >= 0.f && tMaxZResult <= 1.f)
// {
// 	RaycastResult3D raycastMaxZResult3D;
// 	raycastMaxZResult3D.m_didImpact = true;
// 	raycastMaxZResult3D.m_impactPos = start + tMaxZResult * direction * distance;
// 	raycastMaxZResult3D.m_impactDist = tMaxZResult * distance;
// 
// 	raycastMaxZResult3D.m_impactNormal = Vec3::Z_AXIS;
// 	raycastMaxZResult3D.m_rayStartPos = start;
// 	raycastMaxZResult3D.m_rayFwdNormal = direction;
// 	raycastMaxZResult3D.m_rayMaxLength = distance;
// 
// 	if (raycastMaxZResult3D.m_didImpact == true && raycastResult3D.m_didImpact == false)
// 	{
// 		raycastResult3D = raycastMaxZResult3D;
// 	}
// 
// 	if (raycastMaxZResult3D.m_didImpact == true && raycastResult3D.m_didImpact == true && raycastMaxZResult3D.m_impactDist < raycastResult3D.m_impactDist)
// 	{
// 		raycastResult3D = raycastMaxZResult3D;
// 	}
// }
// 
// if the ray intersects the plane at the cylinder min z
// float tMinZResult = fabsf(minZ - start.z) / fabsf(direction.z * distance);
// if (tMinZResult >= 0 && tMinZResult <= 1)
// {
// 	RaycastResult3D raycastMinZResult3D;
// 	raycastMinZResult3D.m_didImpact = true;
// 	raycastMinZResult3D.m_impactPos = start + tMinZResult * direction * distance;
// 	raycastMinZResult3D.m_impactDist = tMinZResult * distance;
// 
// 	raycastMinZResult3D.m_impactNormal = -Vec3::Z_AXIS;
// 	raycastMinZResult3D.m_rayStartPos = start;
// 	raycastMinZResult3D.m_rayFwdNormal = direction;
// 	raycastMinZResult3D.m_rayMaxLength = distance;
// 	//return raycastResult3D;
// 
// 	if (raycastMinZResult3D.m_didImpact == true && raycastResult3D.m_didImpact == false)
// 	{
// 		raycastResult3D = raycastMinZResult3D;
// 	}
// 
// 	if (raycastMinZResult3D.m_didImpact == true && raycastResult3D.m_didImpact == true && raycastMinZResult3D.m_impactDist < raycastResult3D.m_impactDist)
// 	{
// 		raycastResult3D = raycastMinZResult3D;
// 	}
// }
// 
// if the ray intersects the x y - disc at a position between the cylinder min and max z
// XY disc Raycast result and t
// Vec2 dirXY = Vec2(direction.x, direction.y).GetNormalized();
// float distance2D = distance * Vec2(direction.x, direction.y).GetLength();
// RaycastResult2D rResultXYIntersec2D = RaycastVsDisc2D(Vec2(start.x, start.y), dirXY, distance2D, center, radius);
// float tBetweenResult = fabsf(rResultXYIntersec2D.m_impactPos.x - start.x) / fabsf(direction.x * distance);
// if (rResultXYIntersec2D.m_didImpact == true && tBetweenResult >= 0 && tBetweenResult <= 1)
// {
// 	RaycastResult3D raycastBetweenResult3D;
// 	raycastBetweenResult3D.m_didImpact = true;
// 	raycastBetweenResult3D.m_impactPos = start + tBetweenResult * direction * distance;
// 	raycastBetweenResult3D.m_impactDist = tBetweenResult * distance;
// 
// 	raycastBetweenResult3D.m_impactNormal = Vec3(rResultXYIntersec2D.m_impactNormal.x, rResultXYIntersec2D.m_impactNormal.y, 0.f);
// 	raycastBetweenResult3D.m_rayStartPos = start;
// 	raycastBetweenResult3D.m_rayFwdNormal = direction;
// 	raycastBetweenResult3D.m_rayMaxLength = distance;
// 
// 	if (raycastBetweenResult3D.m_didImpact == true && raycastResult3D.m_didImpact == false)
// 	{
// 		raycastResult3D = raycastBetweenResult3D;
// 	}
// 
// 	if (raycastBetweenResult3D.m_didImpact == true && raycastResult3D.m_didImpact == true && raycastBetweenResult3D.m_impactDist < raycastResult3D.m_impactDist)
// 	{
// 		raycastResult3D = raycastBetweenResult3D;
// 	}
// }
// 
// return raycastResult3D;