#include"Engine/Core/VertexUtils.hpp"
#include"Engine/Math/MathUtils.hpp"
#include "Engine/Math/OBB2.hpp"
#include "Engine/Math/Capsule2.hpp"
#include "Engine/Math/LineSegment2.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Hexagon.hpp"
#define UNUSED(x) (void)(x);

void TransformVertexArrayXY3D(int numVerts, Vertex_PCU* verts, float scaleXY, float rotationDegreesAboutZ, Vec2 const& translationXY){
	for (int vertIndex = 0; vertIndex < numVerts; ++vertIndex) {
		Vec3& pos = verts[vertIndex].m_position;
		TransformPositionXY3D(pos, scaleXY, rotationDegreesAboutZ, translationXY);
	}
}

void TransformVertexArray3D(std::vector<Vertex_PCU>& verts, const Mat44& transform)
{
	for (int index = 0; index < (int)verts.size(); index++) 
	{
		Vec3 vertPosi = verts[index].m_position;
		verts[index].m_position = transform.TransformPosition3D(vertPosi);
	}
}

void TransformVertexArray3D(int numVerts, Vertex_PCU* verts, const Mat44& transform)
{
	for (int vertIndex = 0; vertIndex < numVerts; ++vertIndex) 
	{
		Vec3& pos = verts[vertIndex].m_position;
		pos = transform.TransformPosition3D(pos);
	}
}

void TransformVertexArray3D(std::vector<Vertex_PCUTBN>& verts, const Mat44& transform)
{
	for (int index = 0; index < (int)verts.size(); index++)
	{
		Vec3 vertPosi = verts[index].m_position;
		verts[index].m_position = transform.TransformPosition3D(vertPosi);
		verts[index].m_normal = transform.TransformVectorQuantity3D(verts[index].m_normal);
		verts[index].m_normal.Normalize();
	}
}

void TransformVertexArray3D(std::vector<Vertex_PNCU>& verts, const Mat44& transform)
{
	for (int index = 0; index < (int)verts.size(); index++)
	{
		Vec3 vertPosi = verts[index].m_position;
		verts[index].m_position = transform.TransformPosition3D(vertPosi);
		verts[index].m_normal = transform.TransformVectorQuantity3D(verts[index].m_normal);
		verts[index].m_normal.Normalize();
	}
}


void FillTangentSpaceVectorForMesh(std::vector<Vertex_PCUTBN>& verts, std::vector<unsigned int>const indexes)
{
	int size = (int)indexes.size();
	for (int i = 0; i < size; i++) 
	{
		if (i % 3 == 0) //for each triangle
		{
			int indexP0 = indexes[i];
			int indexP1 = indexes[i + 1];
			int indexP2 = indexes[i + 2];

			Vec2 w0 = verts[indexP0].m_uvTexCoords;
			Vec2 w1 = verts[indexP1].m_uvTexCoords;
			Vec2 w2 = verts[indexP2].m_uvTexCoords;

			Vec3 e1 = verts[indexP1].m_position - verts[indexP0].m_position;//p1-p0
			Vec3 e2 = verts[indexP2].m_position - verts[indexP0].m_position;//p2-p0

			float x1 = w1.x - w0.x;
			float x2 = w2.x - w0.x;
			float y1 = w1.y - w0.y; 
			float y2 = w2.y - w0.y;
			float r = 1.0f / (x1 * y2 - x2 * y1);
			Vec3 t = (e1 * y2 - e2 * y1) * r;
			Vec3 b = (e2 * x1 - e1 * x2) * r;

			verts[indexP0].m_tangent += t;
			verts[indexP1].m_tangent += t;
			verts[indexP2].m_tangent += t;
			verts[indexP0].m_binormal += b;
			verts[indexP1].m_binormal += b;
			verts[indexP2].m_binormal += b;
		}
	}

	for (int vertIndex = 0; vertIndex < verts.size(); vertIndex++) 
	{
		Mat44 tangentSpaceMatrix;
		verts[vertIndex].m_tangent.Normalize();
		verts[vertIndex].m_binormal.Normalize();
		//verts[vertIndex].m_normal.Normalize();
		tangentSpaceMatrix.SetIJK3D(verts[vertIndex].m_tangent, verts[vertIndex].m_binormal, verts[vertIndex].m_normal);
		tangentSpaceMatrix.Orthonormalize_XFwd_YLeft_ZUp();
		verts[vertIndex].m_tangent = tangentSpaceMatrix.GetIBasis3D();
		verts[vertIndex].m_binormal = tangentSpaceMatrix.GetJBasis3D();
		verts[vertIndex].m_normal = tangentSpaceMatrix.GetKBasis3D();
	}

}

void AddVertsForConvexPoly2D(std::vector<Vertex_PCU>& verts, ConvexPoly2D const& convexPoly, Rgba8 const& color /*= Rgba8::WHITE*/)
{
	std::vector<Vec2> const points = convexPoly.GetPoints();
	size_t vertNum = (size_t((int)points.size() - 1 - 1) * 3);
	verts.reserve(vertNum);
	Vec3 startPoint = Vec3(points[0], 0.f);
	for (int i = 1; i < points.size() - 1; i++) 
	{
		Vec3 firstPoint = Vec3(points[i], 0.f);
		Vec3 secondPoint = Vec3(points[i + 1], 0.f);

		verts.emplace_back(Vertex_PCU(startPoint, color));
		verts.emplace_back(Vertex_PCU(firstPoint, color));
		verts.emplace_back(Vertex_PCU(secondPoint, color));
	}
}



void AddVertsForConvexPoly3D(std::vector<Vertex_PCU>& verts, ConvexPoly3D const& convexPoly, Rgba8 const& color /*= Rgba8::WHITE*/)
{
	std::vector<Vec3> const points = convexPoly.GetPoints();
	Vec3 startPoint = points[0];
	for (int i = 1; i < points.size() - 1; i++)
	{
		Vec3 firstPoint = points[i];
		Vec3 secondPoint = points[i + 1];

		verts.emplace_back(Vertex_PCU(startPoint, color));
		verts.emplace_back(Vertex_PCU(firstPoint, color));
		verts.emplace_back(Vertex_PCU(secondPoint, color));
	}
}

AABB2 GetVertexBounds2D(const std::vector<Vertex_PCU>& verts)
{
	AABB2 aabb = AABB2(verts[0].m_position.x, verts[0].m_position.y, verts[0].m_position.x, verts[0].m_position.y);
	//AABB2 aabb = AABB2(verts[0].m_position.x, verts[0].m_position.y, verts[(int)verts.size() - 1].m_position.x, verts[(int)verts.size() - 1].m_position.y);

	for (int vertIndex = 1; vertIndex < (int)verts.size(); vertIndex++) 
	{
		if (aabb.m_mins.x > verts[vertIndex].m_position.x) 
		{
			aabb.m_mins.x = verts[vertIndex].m_position.x;
		}
		if (aabb.m_mins.y > verts[vertIndex].m_position.y)
		{
			aabb.m_mins.y = verts[vertIndex].m_position.y;
		}
		if (aabb.m_maxs.x < verts[vertIndex].m_position.x)
		{
			aabb.m_maxs.x = verts[vertIndex].m_position.x;
		}
		if (aabb.m_maxs.y < verts[vertIndex].m_position.y)
		{
			aabb.m_maxs.y = verts[vertIndex].m_position.y;
		}
	}

	return aabb;
}

void AddVertsForCylinder3D(std::vector<Vertex_PCU>& verts, Vec3 const& start, Vec3 const & end, float radius, Rgba8 const& color /*= Rgba8::WHITE*/, AABB2 const& UVs /*= AABB2::ZERO_TO_ONE*/, int numSlices /*= 8*/)
{
	std::vector<Vertex_PCU> localVerts;
	Vec3 vSE = end - start;
	Vec2 centerXY = Vec2(0.f, 0.f);
	FloatRange minMaxZ = FloatRange(0.f, vSE.GetLength());
	AddVertsForCylinderZ3D(localVerts, centerXY, minMaxZ, radius, (float)numSlices, color, UVs);

	Mat44 mat44;
	Vec3 iBase = vSE.GetNormalized();
	Vec3 jBase = CrossProduct3D(Vec3::Z_AXIS, iBase);
	if (jBase == Vec3::WORLD_ORIGIN) 
	{
		jBase = Vec3::Y_AXIS;
	}
	jBase.Normalize();
	Vec3 kBase = CrossProduct3D(iBase, jBase).GetNormalized();

	mat44.SetIJK3D(kBase, jBase, iBase);
	mat44.SetTranslation3D(start);
	TransformVertexArray3D(localVerts, mat44);

	for (int vertsIndex = 0; vertsIndex < (int)localVerts.size(); vertsIndex++) 
	{
		verts.push_back(localVerts[vertsIndex]);
	}
// 	int NUM_SIDES = (int)numSlices;
// 	int NUM_TRIS = 4 * NUM_SIDES;
// 	int NUM_VERTS = 3 * NUM_TRIS;
// 	float DEFREE_PER_SIDE = 360.f / static_cast<float>(NUM_SIDES);
// 	float deltaXUVs = (UVs.GetDimensions().x / numSlices);
// 	verts.reserve(verts.size() + NUM_VERTS);
// 
// 	for (int sideNum = 0; sideNum < NUM_SIDES; sideNum++) 
// 	{
// 		float startDegrees = DEFREE_PER_SIDE * static_cast<float>(sideNum);
// 		Vec2 startVec2 = Vec2::MakeFromPolarDegrees(startDegrees, radius);
// 		float endDegrees = DEFREE_PER_SIDE * static_cast<float>(sideNum + 1);
// 		Vec2 endVec2 = Vec2::MakeFromPolarDegrees(endDegrees, radius);
// 		//Top
// 		Vec3 startPointTop	= Vec3(startVec2.x, startVec2.y, 0.f) + start;
// 		Vec3 endPointTop	= Vec3(endVec2.x, endVec2.y, 0.f) + start;
// 		Vec3 centerVec3Top	= start;
// 		//Bottom
// 		Vec3 startPointBottom = Vec3(startVec2.x, startVec2.y, 0.f) + end;
// 		Vec3 centerVec3Bottom = Vec3(endVec2.x, endVec2.y, 0.f) + end;
// 		Vec3 endPointBottom = end;
// 
// 		Vec2 centerUVs = UVs.m_mins + UVs.GetDimensions() * 0.5f;
// 		Vec2 startUVs = centerUVs + Vec2::MakeFromPolarDegrees(startDegrees) * centerUVs;
// 		Vec2 endUVs = centerUVs + Vec2::MakeFromPolarDegrees(endDegrees) * centerUVs;
// 
// 		verts.push_back(Vertex_PCU(centerVec3Top, color, centerUVs));
// 		verts.push_back(Vertex_PCU(startPointTop, color, startUVs));
// 		verts.push_back(Vertex_PCU(endPointTop, color, endUVs));
// 
// 		verts.push_back(Vertex_PCU(centerVec3Bottom, color, centerUVs));
// 		verts.push_back(Vertex_PCU(endPointBottom, color, endUVs));
// 		verts.push_back(Vertex_PCU(startPointBottom, color, startUVs));
// 
// 		//Vertical Quad
// 
// 		float quadUVsX = deltaXUVs * (float)sideNum;
// 		float quadUVsXNext = deltaXUVs * (float)(sideNum + 1);
// 
// 		AABB2 quadUVs = AABB2(quadUVsX, UVs.m_mins.y, quadUVsXNext, UVs.m_maxs.y);
// 		AddVertsForQuad3D(verts, startPointBottom, endPointBottom, endPointTop, startPointTop, color, quadUVs);
// 
// 	}

}

void AddVertsForCone3D(std::vector<Vertex_PCU>& verts, Vec3 const& start, const Vec3& end, float radius, Rgba8 const& color /*= Rgba8::WHITE*/, AABB2 const& UVs /*= AABB2::ZERO_TO_ONE*/, int numSlices /*= 8*/)
{
// 	std::vector<Vertex_PCU> localVerts;
// 
// 	int NUM_SIDES = (int)numSlices;
// 	int NUM_TRIS = 2 * NUM_SIDES * 2;
// 	int NUM_VERTS = 3 * NUM_TRIS;
// 	float DEFREE_PER_SIDE = 360.f / static_cast<float>(NUM_SIDES);
// 
// 	verts.reserve(verts.size() + NUM_VERTS);
// 
// 	for (int sideNum = 0; sideNum < NUM_SIDES; sideNum++) 
// 	{
// 		float startDegrees = DEFREE_PER_SIDE * static_cast<float>(sideNum);
// 		Vec2 startVec2 = Vec2::MakeFromPolarDegrees(startDegrees, radius);
// 		float endDegrees = DEFREE_PER_SIDE * static_cast<float>(sideNum + 1);
// 		Vec2 endVec2 = Vec2::MakeFromPolarDegrees(endDegrees, radius);
// 
// 		//Bottom
// 		Vec3 startPointBottom = Vec3(startVec2.x, startVec2.y, 0.f) + end;
// 		Vec3 centerVec3Bottom = Vec3(endVec2.x, endVec2.y, 0.f) + end;
// 		Vec3 endPointBottom = end;
// 
// 		Vec2 centerUVs = UVs.m_mins + UVs.GetDimensions() * 0.5f;
// 		Vec2 startUVs = centerUVs + Vec2::MakeFromPolarDegrees(startDegrees) * centerUVs;
// 		Vec2 endUVs = centerUVs + Vec2::MakeFromPolarDegrees(endDegrees) * centerUVs;
// 
// 		localVerts.push_back(Vertex_PCU(start, color, centerUVs));
// 		localVerts.push_back(Vertex_PCU(centerVec3Bottom, color, endUVs));
// 		localVerts.push_back(Vertex_PCU(startPointBottom, color, startUVs));
// 
// 		localVerts.push_back(Vertex_PCU(centerVec3Bottom, color, centerUVs));
// 		localVerts.push_back(Vertex_PCU(endPointBottom, color, endUVs));
// 		localVerts.push_back(Vertex_PCU(startPointBottom, color, startUVs));
// 	}
// 
// 	Vec3 vSE = end - start;
// 	Mat44 mat44;
// 	Vec3 iBase = vSE.GetNormalized();
// 	Vec3 jBase = CrossProduct3D(Vec3::Z_AXIS, iBase);
// 	if (jBase == Vec3::WORLD_ORIGIN)
// 	{
// 		jBase = Vec3::Y_AXIS;
// 	}
// 	jBase.Normalize();
// 	Vec3 kBase = CrossProduct3D(iBase, jBase);
// 
// 	mat44.SetIJK3D(iBase, jBase, kBase);
// 	mat44.SetTranslation3D(start);
// 	TransformVertexArray3D(localVerts, mat44);
// // 
// 	for (int vertsIndex = 0; vertsIndex < (int)localVerts.size(); vertsIndex++)
// 	{
// 		verts.push_back(localVerts[vertsIndex]);
// 		}



	Vec3 dispSE = (end - start);
	Vec3 iForward = dispSE.GetNormalized();
	Vec3 jLeft = CrossProduct3D(Vec3::Z_AXIS, iForward);
	if (jLeft == Vec3::WORLD_ORIGIN)
	{
		jLeft = Vec3::Y_AXIS;
	}
	else
	{
		jLeft.Normalize();
	}
	Vec3 kUp = CrossProduct3D(iForward, jLeft);
	Mat44 transformationMatrix;
	transformationMatrix.SetIJK3D(iForward, jLeft, kUp);
	transformationMatrix.SetTranslation3D(start);

	Vec3 sectorMinTip = Vec3(0.f, 0.f, 0.f);
	sectorMinTip = transformationMatrix.TransformPosition3D(sectorMinTip);
	float depth = dispSE.GetLength();
	Vec3 sectorMaxTip = Vec3(depth, 0.f, 0.f);
	sectorMaxTip = transformationMatrix.TransformPosition3D(sectorMaxTip);
	Vec2 sectorTipUV = Vec2(0.5f, 0.5f);
	float currentYawDegrees = 0.f;
	float degreesPerYawSlice = 360.f / numSlices;
	for (int sliceNum = 0; sliceNum < numSlices; ++sliceNum)
	{
		Vec2 currentSectorVert1Vec2 = Vec2::MakeFromPolarDegrees(currentYawDegrees, radius);
		Vec3 currentSectorVert1 = Vec3(0.f, currentSectorVert1Vec2.y, currentSectorVert1Vec2.x);
		Vec2 currentSectorVert2Vec2 = Vec2::MakeFromPolarDegrees(currentYawDegrees + degreesPerYawSlice, radius);
		Vec3 currentSectorVert2 = Vec3(0.f, currentSectorVert2Vec2.y, currentSectorVert2Vec2.x);

		float currentSectorVert1U = RangeMap(currentSectorVert1.z, -1.f, 1.f, UVs.m_mins.x, UVs.m_maxs.x);
		float currentSectorVert1V = RangeMap(currentSectorVert1.y, -1.f, 1.f, UVs.m_mins.y, UVs.m_maxs.y);
		Vec2 currentSectorVert1UV = Vec2(currentSectorVert1U, currentSectorVert1V);
		float currentSectorVert2U = RangeMap(currentSectorVert2.z, -1.f, 1.f, UVs.m_mins.x, UVs.m_maxs.x);
		float currentSectorVert2V = RangeMap(currentSectorVert2.y, -1.f, 1.f, UVs.m_mins.y, UVs.m_maxs.y);
		Vec2 currentSectorVert2UV = Vec2(currentSectorVert2U, currentSectorVert2V);
		Vec3 currentSectorMinVert1 = currentSectorVert1;
		Vec3 currentSectorMinVert2 = currentSectorVert2;
		currentSectorMinVert1 = transformationMatrix.TransformPosition3D(currentSectorMinVert1);
		currentSectorMinVert2 = transformationMatrix.TransformPosition3D(currentSectorMinVert2);
		verts.push_back(Vertex_PCU(currentSectorMinVert1, color, Vec2(currentSectorVert2U, 1.f - currentSectorVert2V)));
		verts.push_back(Vertex_PCU(currentSectorMinVert2, color, Vec2(currentSectorVert2U, 1.f - currentSectorVert2V)));
		verts.push_back(Vertex_PCU(sectorMinTip, color, sectorTipUV));
		verts.push_back(Vertex_PCU(sectorMaxTip, color, sectorTipUV));
		verts.push_back(Vertex_PCU(currentSectorMinVert2, color, currentSectorVert1UV));
		verts.push_back(Vertex_PCU(currentSectorMinVert1, color, currentSectorVert2UV));
		currentYawDegrees += degreesPerYawSlice;
	}
}

void AddVertsForWorldArrow3D(std::vector<Vertex_PCU>& verts, Vec3 const& start, const Vec3& end, float radius, Rgba8 const& color /*= Rgba8::WHITE*/, AABB2 const& UVs /*= AABB2::ZERO_TO_ONE*/, int numSlices /*= 8*/)
{
	UNUSED(UVs);
	UNUSED(numSlices);
	float coneRatio = 0.4f;
	Vec3 vSE = end - start;
	Vec3 vSENormal = vSE.GetNormalized();
	float vSELength = vSE.GetLength();

	Vec3 coneStart = end;
	Vec3 coneEnd = coneStart + (-vSENormal) * coneRatio * vSELength;
	float coneRadius = radius;
	Vec3 cylinederStart = coneEnd;
	Vec3 cylinederEnd = start;
	float cylinederRadius = 0.4f * radius;

	AddVertsForCone3D(verts, coneEnd, coneStart, coneRadius, color);
	AddVertsForCylinder3D(verts, cylinederStart, cylinederEnd, cylinederRadius, color);

}

void AddVertsForRing2D(std::vector<Vertex_PCU>& verts, Vec2 const& center, float radius, float thickness, Rgba8 const& color)
{
	float halfThickness = 0.5f * thickness;
	float innerRadius = radius - halfThickness;
	float outerRadius = radius + halfThickness;
	constexpr int NUM_SIDES = 200;
	constexpr int NUM_TRIS = 2 * NUM_SIDES;          //each side is a trapezoid
	constexpr int NUM_VERTS = 3 * NUM_TRIS;          //3 vertexes per triangle

	verts.reserve(NUM_VERTS);
	constexpr float DEFREE_PER_SIDE = 360.f / static_cast<float>(NUM_SIDES);
	for (int sideNum = 0; sideNum < NUM_SIDES; sideNum++)
	{

		//Compute angle-related terms
		float startDegrees = DEFREE_PER_SIDE * static_cast<float>(sideNum);
		float endDegrees = DEFREE_PER_SIDE * static_cast<float>(sideNum + 1);
		float cosStart = CosDegrees(startDegrees);
		float sinStart = SinDegrees(startDegrees);
		float cosEnd = CosDegrees(endDegrees);
		float sinEnd = SinDegrees(endDegrees);

		//Compute inner & outer positions
		Vec3 innerStartPos(center.x + innerRadius * cosStart, center.y + innerRadius * sinStart, 0.f);
		Vec3 outerStartPos(center.x + outerRadius * cosStart, center.y + outerRadius * sinStart, 0.f);
		Vec3 outerEndPos(center.x + outerRadius * cosEnd, center.y + outerRadius * sinEnd, 0.f);
		Vec3 innerEndPos(center.x + innerRadius * cosEnd, center.y + innerRadius * sinEnd, 0.f);

		verts.push_back(Vertex_PCU(innerEndPos, color));
		verts.push_back(Vertex_PCU(innerStartPos, color));
		verts.push_back(Vertex_PCU(outerStartPos, color));

		verts.push_back(Vertex_PCU(innerEndPos, color));
		verts.push_back(Vertex_PCU(outerStartPos, color));
		verts.push_back(Vertex_PCU(outerEndPos, color));

	}

}

void AddVertsForSector2D(std::vector<Vertex_PCU>& verts, Vec2 const& center, float const& radius, float const& startDegrees, float const& endDegrees, Rgba8 const& color)
{
	constexpr int NUM_SIDES = 20;
	constexpr int NUM_TRIS = 2 * NUM_SIDES;
	constexpr int NUM_VERTS = 3 * NUM_TRIS;
	float deltaDegree = endDegrees - startDegrees;
	if (startDegrees> endDegrees)
	{
		deltaDegree += 360;
	}
	float DEFREE_PER_SIDE = deltaDegree / static_cast<float>(NUM_SIDES);
	verts.reserve(verts.size() + NUM_VERTS);
	for (int sideNum = 0; sideNum < NUM_SIDES; sideNum++)
	{
		float starterDegrees = DEFREE_PER_SIDE * static_cast<float>(sideNum)+ startDegrees;
		float enderDegrees = DEFREE_PER_SIDE * static_cast<float>(sideNum + 1)+ startDegrees;
		//Compute angle-related terms
		Vec2 startPoint = Vec2::MakeFromPolarDegrees(starterDegrees, radius) + center;
		Vec2 endPoint = Vec2::MakeFromPolarDegrees(enderDegrees, radius) + center;

		//Compute point positions

		Vec3 centerVec3 = Vec3(center.x, center.y, 0.f);
		Vec3 StartPosVec3 = Vec3(startPoint.x, startPoint.y, 0.f);
		Vec3 EndPosVec3 = Vec3(endPoint.x, endPoint.y, 0.f);

		verts.push_back(Vertex_PCU(centerVec3, color, Vec2(0.f, 0.f)));
		verts.push_back(Vertex_PCU(StartPosVec3, color, Vec2(1.f, 0.f)));
		verts.push_back(Vertex_PCU(EndPosVec3, color, Vec2(1.f, 1.f)));
	}
}

void AddVertsForCapsule2D(std::vector<Vertex_PCU>& verts, Capsule2 const& capsule, Rgba8 const& color)
{
	//calculate displacement

	Vec2 startToEnd = capsule.m_end - capsule.m_start;
	Vec2 startToEndNormalized = startToEnd.GetNormalized();
	Vec2 bonePerpendicular = startToEndNormalized.GetRotated90Degrees() * capsule.m_radius;
	Vec2 radiusVertiOfStart = bonePerpendicular;

	//calculate and draw half circle + another circle
	float starterDegree = radiusVertiOfStart.GetOrientationDegrees();
	float starterDegreeForAnotherCircle = (-radiusVertiOfStart).GetOrientationDegrees();


	AddVertsForSector2D(verts, capsule.m_start, capsule.m_radius, starterDegree, starterDegree + 180.f, color);
	AddVertsForSector2D(verts, capsule.m_end, capsule.m_radius, starterDegreeForAnotherCircle, starterDegreeForAnotherCircle + 180.f, color); 
	//Draw the Square

	verts.reserve(verts.size() + 6);
	OBB2 obb = OBB2();
	obb.m_center = 0.5f * (startToEnd)+capsule.m_start;
	obb.m_iBasisNormal = (startToEnd).GetNormalized();
	obb.m_halfDimensions = Vec2(startToEnd.GetLength() * 0.5f, capsule.m_radius);

	AddVertsForOBB2D(verts, obb, color);

}

void AddVertsForCapsule2D(std::vector<Vertex_PCU>& verts, Vec2 const& boneStart, Vec2 const& boneEnd, float radius, Rgba8 const& color)
{
	//calculate displacement
	Vec2 startToEnd = boneEnd - boneStart;
	Vec2 startToEndNormalized = startToEnd.GetNormalized();
	Vec2 bonePerpendicular = startToEndNormalized.GetRotated90Degrees() * radius;
	Vec2 radiusVertiOfStart = bonePerpendicular + boneStart;

	//calculate and draw half circle + another circle
	float starterDegree = radiusVertiOfStart.GetOrientationDegrees();
	float starterDegreeForAnotherCircle = (-radiusVertiOfStart).GetOrientationDegrees();

	constexpr int NUM_SIDES = 50;
	constexpr int NUM_TRIS = 2 * NUM_SIDES;
	constexpr int NUM_VERTS = 3 * NUM_TRIS;
	constexpr float DEFREE_PER_SIDE = 180.f / static_cast<float>(NUM_SIDES);

	//verts.reserve(verts.size() + NUM_VERTS);
	for (int sideNum = 0; sideNum < NUM_SIDES; sideNum++)
	{
		float startDegrees = DEFREE_PER_SIDE * static_cast<float>(sideNum) + starterDegree;
		float endDegrees = DEFREE_PER_SIDE * static_cast<float>(sideNum + 1) + starterDegree;

		AddVertsForSector2D(verts, boneStart, radius, startDegrees, endDegrees, color);
	}

	//calculate and draw another half circle
	for (int sideNum = 0; sideNum < NUM_SIDES; sideNum++)
	{
		float startDegrees = DEFREE_PER_SIDE * static_cast<float>(sideNum) + starterDegreeForAnotherCircle;
		float endDegrees = DEFREE_PER_SIDE * static_cast<float>(sideNum + 1) + starterDegreeForAnotherCircle;

		AddVertsForSector2D(verts, boneStart, radius, startDegrees, endDegrees, color);
	}
	//Draw the Square

	verts.reserve(verts.size() + 6);
	Vec2 startLeft = radiusVertiOfStart;
	Vec2 startRight = -radiusVertiOfStart;
	Vec2 endLeft = boneEnd + bonePerpendicular;
	Vec2 endRight = boneEnd - bonePerpendicular;

	Vec3 startLeftVec3 = Vec3(startLeft.x, startLeft.y, 0.f);
	Vec3 startRightVec3 = Vec3(startRight.x, startRight.y, 0.f);
	Vec3 endLeftVec3 = Vec3(endLeft.x, endLeft.y, 0.f);
	Vec3 endRightVec3 = Vec3(endRight.x, endRight.y, 0.f);

	verts.push_back(Vertex_PCU(startRightVec3, color, Vec2(0.f, 0.f)));
	verts.push_back(Vertex_PCU(endRightVec3, color, Vec2(1.f, 0.f)));
	verts.push_back(Vertex_PCU(endLeftVec3, color, Vec2(1.f, 1.f)));
	verts.push_back(Vertex_PCU(endLeftVec3, color, Vec2(0.f, 0.f)));
	verts.push_back(Vertex_PCU(startLeftVec3, color, Vec2(1.f, 0.f)));
	verts.push_back(Vertex_PCU(startRightVec3, color, Vec2(1.f, 1.f)));
}

void AddVertsForDisc2D(std::vector<Vertex_PCU>& verts, Vec2 const& center, float const& radius, Rgba8 const& color)
{
	constexpr int NUM_SIDES = 60;
	constexpr int NUM_TRIS = 2 * NUM_SIDES;
	constexpr int NUM_VERTS = 3 * NUM_TRIS;
	constexpr float DEFREE_PER_SIDE = 360.f / static_cast<float>(NUM_SIDES);
	verts.reserve(NUM_VERTS);

	for (int sideNum = 0; sideNum < NUM_SIDES; sideNum++) {
		//Compute angle-related terms
		float startDegrees = DEFREE_PER_SIDE * static_cast<float>(sideNum);
		float endDegrees = DEFREE_PER_SIDE * static_cast<float>(sideNum + 1);
		Vec2 startPoint = Vec2::MakeFromPolarDegrees(startDegrees, radius) + center;
		Vec2 endPoint = Vec2::MakeFromPolarDegrees(endDegrees, radius) + center;

		Vec3 centerVec3 = Vec3(center.x, center.y, 0.f);
		Vec3 startPosVec3 = Vec3(startPoint.x, startPoint.y, 0.f);
		Vec3 endPosVec3 = Vec3(endPoint.x, endPoint.y, 0.f);

		verts.push_back(Vertex_PCU(centerVec3, color, Vec2(0.f, 0.f)));
		verts.push_back(Vertex_PCU(startPosVec3, color, Vec2(0.f, 0.f)));
		verts.push_back(Vertex_PCU(endPosVec3, color, Vec2(0.f, 0.f)));
	}
}

void AddVertsForAABB2D(std::vector<Vertex_PCU>& verts, AABB2 const& bounds, Rgba8 const& color)
{
	constexpr int NUM_TRIS = 2;
	constexpr int NUM_VERTS = 3 * NUM_TRIS;
	verts.reserve(verts.size() + NUM_VERTS);
	Vec3 BL = Vec3(bounds.m_mins.x, bounds.m_mins.y, 0.f);
	Vec3 BR = Vec3(bounds.m_maxs.x, bounds.m_mins.y, 0.f);
	Vec3 TR = Vec3(bounds.m_maxs.x, bounds.m_maxs.y, 0.f);
	Vec3 TL = Vec3(bounds.m_mins.x, bounds.m_maxs.y, 0.f);

	verts.push_back(Vertex_PCU(BL, color, Vec2(0.f, 0.f)));
	verts.push_back(Vertex_PCU(BR, color, Vec2(1.f, 0.f)));
	verts.push_back(Vertex_PCU(TR, color, Vec2(1.f, 1.f)));

	verts.push_back(Vertex_PCU(BL, color, Vec2(0.f, 0.f)));
	verts.push_back(Vertex_PCU(TR, color, Vec2(1.f, 1.f)));
	verts.push_back(Vertex_PCU(TL, color, Vec2(0.f, 1.f)));

}
void AddVertsForAABB2D(std::vector<Vertex_PCU>& verts, AABB2 const& spriteBounds, Rgba8 const& spriteTint, Vec2 const& uvAtMins, Vec2 const& uvAtMaxs)
{
	constexpr int NUM_TRIS = 2;
	constexpr int NUM_VERTS = 3 * NUM_TRIS;
	verts.reserve(verts.size() + NUM_VERTS);
	Vec3 BL = Vec3(spriteBounds.m_mins.x, spriteBounds.m_mins.y, 0.f);
	Vec3 BR = Vec3(spriteBounds.m_maxs.x, spriteBounds.m_mins.y, 0.f);
	Vec3 TL = Vec3(spriteBounds.m_mins.x, spriteBounds.m_maxs.y, 0.f);
	Vec3 TR = Vec3(spriteBounds.m_maxs.x, spriteBounds.m_maxs.y, 0.f);

	verts.push_back(Vertex_PCU(BL, spriteTint, uvAtMins));
	verts.push_back(Vertex_PCU(BR, spriteTint, Vec2(uvAtMaxs.x, uvAtMins.y)));
	verts.push_back(Vertex_PCU(TR, spriteTint, uvAtMaxs));

	verts.push_back(Vertex_PCU(TR, spriteTint, uvAtMaxs));
	verts.push_back(Vertex_PCU(TL, spriteTint, Vec2(uvAtMins.x, uvAtMaxs.y)));
	verts.push_back(Vertex_PCU(BL, spriteTint, uvAtMins));
}
void AddVertsForQuad3D(std::vector<Vertex_PCU>& verts, const Vec3& bottomLeft, const Vec3& bottomRight, const Vec3& topRight, const Vec3& topLeft, const Rgba8& color /*= Rgba8::WHITE*/, const AABB2& UVs /*= AABB2::ZERO_TO_ONE*/)
{
	int NUM_TRIS = 2;
	int NUM_VERTS = 3 * NUM_TRIS;
	verts.reserve(verts.size() + NUM_VERTS);
	verts.push_back(Vertex_PCU(bottomLeft, color, UVs.m_mins));
	verts.push_back(Vertex_PCU(bottomRight, color, Vec2(UVs.m_maxs.x, UVs.m_mins.y)));
	verts.push_back(Vertex_PCU(topRight, color, UVs.m_maxs));

	verts.push_back(Vertex_PCU(topRight, color, UVs.m_maxs));
	verts.push_back(Vertex_PCU(topLeft, color, Vec2(UVs.m_mins.x, UVs.m_maxs.y)));
	verts.push_back(Vertex_PCU(bottomLeft, color, UVs.m_mins));
}	

void AddVertsForQuad3D(std::vector<Vertex_PCU>& verts, std::vector<unsigned int>& indexes, const Vec3& bottomLeft, const Vec3& bottomRight, const Vec3& topRight, const Vec3& topLeft, const Rgba8& color, const AABB2& UVs /*= AABB2::ZERO_TO_ONE*/)
{
	int NUM_TRIS = 2;
	int NUM_VERTS = 3 * NUM_TRIS;
	verts.reserve(verts.size() + 4);
	indexes.reserve(indexes.size() + NUM_VERTS);
	int startIndex = (int)verts.size();
	verts.push_back(Vertex_PCU(bottomLeft, color, UVs.m_mins));
	verts.push_back(Vertex_PCU(bottomRight, color, Vec2(UVs.m_maxs.x, UVs.m_mins.y)));
	verts.push_back(Vertex_PCU(topRight, color, UVs.m_maxs));
	verts.push_back(Vertex_PCU(topLeft, color, Vec2(UVs.m_mins.x, UVs.m_maxs.y)));

	indexes.push_back(startIndex);
	indexes.push_back(startIndex + 1);
	indexes.push_back(startIndex + 2);
	indexes.push_back(startIndex + 2);
	indexes.push_back(startIndex + 3);
	indexes.push_back(startIndex);
}

void AddVertsForQuad3D(std::vector<Vertex_PCUTBN>& verts, std::vector<unsigned int>& indexes, const Vec3& bottomLeft, const Vec3& bottomRight, const Vec3& topRight, const Vec3& topLeft, const Rgba8& color, const AABB2& UVs /*= AABB2::ZERO_TO_ONE*/)
{
	int NUM_TRIS = 2;
	int NUM_VERTS = 3 * NUM_TRIS;
	verts.reserve(verts.size() + 4);
	indexes.reserve(indexes.size() + NUM_VERTS);
	int startIndex = (int)verts.size();
	Vec3 aBBLToBR = bottomRight - bottomLeft;
// 	Vec3 bCBRToTR = topRight - bottomRight;
// 	Vec3 cDTRToTL = topLeft - topRight;
	Vec3 dATLToBL = bottomLeft - topLeft;

	Vec3 normalBL = CrossProduct3D(aBBLToBR, -dATLToBL).GetNormalized();
// 	Vec3 normalBR = CrossProduct3D(bCBRToTR, -aBBLToBR).GetNormalized();
// 	Vec3 normalTR = CrossProduct3D(cDTRToTL, -bCBRToTR).GetNormalized();
// 	Vec3 normalTL = CrossProduct3D(dATLToBL, -cDTRToTL).GetNormalized();
	
	verts.push_back(Vertex_PCUTBN(Vertex_PNCU(bottomLeft, normalBL, color, UVs.m_mins)));
	verts.push_back(Vertex_PCUTBN(Vertex_PNCU(bottomRight, normalBL, color, Vec2(UVs.m_maxs.x, UVs.m_mins.y))));
	verts.push_back(Vertex_PCUTBN(Vertex_PNCU(topRight, normalBL, color, UVs.m_maxs)));
	verts.push_back(Vertex_PCUTBN(Vertex_PNCU(topLeft, normalBL, color, Vec2(UVs.m_mins.x, UVs.m_maxs.y))));

	indexes.push_back(startIndex);
	indexes.push_back(startIndex + 1);
	indexes.push_back(startIndex + 2);
	indexes.push_back(startIndex + 2);
	indexes.push_back(startIndex + 3);
	indexes.push_back(startIndex);
}

void AddVertsForQuad3D(std::vector<Vertex_PCUTBN>& verts, const Vec3& bottomLeft, const Vec3& bottomRight, const Vec3& topRight, const Vec3& topLeft, const Rgba8& color, const AABB2& UVs /*= AABB2::ZERO_TO_ONE*/)
{
	int NUM_TRIS = 2;
	int NUM_VERTS = 3 * NUM_TRIS;
	verts.reserve(verts.size() + NUM_VERTS);

	Vec3 aBBLToBR = bottomRight - bottomLeft;
	Vec3 dATLToBL = bottomLeft - topLeft;

	Vec3 normalBL = CrossProduct3D(aBBLToBR, -dATLToBL).GetNormalized();
	verts.push_back(Vertex_PCUTBN(Vertex_PNCU(bottomLeft, normalBL, color, UVs.m_mins)));
	verts.push_back(Vertex_PCUTBN(Vertex_PNCU(bottomRight, normalBL, color, Vec2(UVs.m_maxs.x, UVs.m_mins.y))));
	verts.push_back(Vertex_PCUTBN(Vertex_PNCU(topRight, normalBL, color, UVs.m_maxs)));

	verts.push_back(Vertex_PCUTBN(Vertex_PNCU(topRight, normalBL, color, UVs.m_maxs)));
	verts.push_back(Vertex_PCUTBN(Vertex_PNCU(topLeft, normalBL, color, Vec2(UVs.m_mins.x, UVs.m_maxs.y))));
	verts.push_back(Vertex_PCUTBN(Vertex_PNCU(bottomLeft, normalBL, color, UVs.m_mins)));
}

void AddVertsForQuadNormalOnly3D(std::vector<Vertex_PNCU>& verts, std::vector<unsigned int>& indexes, const Vec3& bottomLeft, const Vec3& bottomRight, const Vec3& topRight, const Vec3& topLeft, const Rgba8& color /*= Rgba8::WHITE*/, const AABB2& UVs /*= AABB2::ZERO_TO_ONE*/)
{
	int NUM_TRIS = 2;
	int NUM_VERTS = 3 * NUM_TRIS;
	verts.reserve(verts.size() + 4);
	indexes.reserve(indexes.size() + NUM_VERTS);
	int startIndex = (int)verts.size();
	Vec3 aBBLToBR = bottomRight - bottomLeft;

	Vec3 dATLToBL = bottomLeft - topLeft;

	Vec3 normalBL = CrossProduct3D(aBBLToBR, -dATLToBL).GetNormalized();

	verts.push_back(Vertex_PNCU(bottomLeft, normalBL, color, UVs.m_mins));
	verts.push_back(Vertex_PNCU(bottomRight, normalBL, color, Vec2(UVs.m_maxs.x, UVs.m_mins.y)));
	verts.push_back(Vertex_PNCU(topRight, normalBL, color, UVs.m_maxs));
	verts.push_back(Vertex_PNCU(topLeft, normalBL, color, Vec2(UVs.m_mins.x, UVs.m_maxs.y)));

	indexes.push_back(startIndex);
	indexes.push_back(startIndex + 1);
	indexes.push_back(startIndex + 2);
	indexes.push_back(startIndex + 2);
	indexes.push_back(startIndex + 3);
	indexes.push_back(startIndex);
}

void AddVertsForQuadNormalOnly3D(std::vector<Vertex_PNCU>& verts, const Vec3& bottomLeft, const Vec3& bottomRight, const Vec3& topRight, const Vec3& topLeft, const Rgba8& color, const AABB2& UVs /*= AABB2::ZERO_TO_ONE*/)
{
	int NUM_TRIS = 2;
	int NUM_VERTS = 3 * NUM_TRIS;
	verts.reserve(verts.size() + NUM_VERTS);

	Vec3 aBBLToBR = bottomRight - bottomLeft;
	Vec3 dATLToBL = bottomLeft - topLeft;

	Vec3 normalBL = CrossProduct3D(aBBLToBR, -dATLToBL).GetNormalized();
	verts.push_back(Vertex_PNCU(bottomLeft, normalBL, color, UVs.m_mins));
	verts.push_back(Vertex_PNCU(bottomRight, normalBL, color, Vec2(UVs.m_maxs.x, UVs.m_mins.y)));
	verts.push_back(Vertex_PNCU(topRight, normalBL, color, UVs.m_maxs));

	verts.push_back(Vertex_PNCU(topRight, normalBL, color, UVs.m_maxs));
	verts.push_back(Vertex_PNCU(topLeft, normalBL, color, Vec2(UVs.m_mins.x, UVs.m_maxs.y)));
	verts.push_back(Vertex_PNCU(bottomLeft, normalBL, color, UVs.m_mins));
}

void AddVertsForQuadNormalOnly3DPCUTBN(std::vector<Vertex_PCUTBN>& verts, const Vec3& bottomLeft, const Vec3& bottomRight, const Vec3& topRight, const Vec3& topLeft, const Rgba8& color, const AABB2& UVs /*= AABB2::ZERO_TO_ONE*/)
{
	int NUM_TRIS = 2;
	int NUM_VERTS = 3 * NUM_TRIS;
	verts.reserve(verts.size() + NUM_VERTS);

	Vec3 aBBLToBR = bottomRight - bottomLeft;
	Vec3 dATLToBL = bottomLeft - topLeft;

	Vec3 normalBL = CrossProduct3D(aBBLToBR, -dATLToBL).GetNormalized();
	verts.push_back(Vertex_PCUTBN(Vertex_PNCU(bottomLeft, normalBL, color, UVs.m_mins)));
	verts.push_back(Vertex_PCUTBN(Vertex_PNCU(bottomRight, normalBL, color, Vec2(UVs.m_maxs.x, UVs.m_mins.y))));
	verts.push_back(Vertex_PCUTBN(Vertex_PNCU(topRight, normalBL, color, UVs.m_maxs)));

	verts.push_back(Vertex_PCUTBN(Vertex_PNCU(topRight, normalBL, color, UVs.m_maxs)));
	verts.push_back(Vertex_PCUTBN(Vertex_PNCU(topLeft, normalBL, color, Vec2(UVs.m_mins.x, UVs.m_maxs.y))));
	verts.push_back(Vertex_PCUTBN(Vertex_PNCU(bottomLeft, normalBL, color, UVs.m_mins)));
}

void AddVertsForTriangle3D(std::vector<Vertex_PCUTBN>& verts, const Vec3& pointA, const Vec3& pointB, const Vec3& pointC, const Rgba8& color, bool isInXYPlane /*= false*/)
{
	int NUM_VERTS = 3;
	verts.reserve(verts.size() + NUM_VERTS);

	Vec3 normal = Vec3(0.f, 0.f, 1.f);
	if (!isInXYPlane) 
	{
		Vec3 aToB = pointB - pointA;
		Vec3 aToC = pointC - pointA;

		normal = CrossProduct3D(aToB, aToC).GetNormalized();
	}


	verts.push_back(Vertex_PCUTBN(Vertex_PNCU(pointA, normal, color)));
	verts.push_back(Vertex_PCUTBN(Vertex_PNCU(pointB, normal, color)));
	verts.push_back(Vertex_PCUTBN(Vertex_PNCU(pointC, normal, color)));

}


void AddVertsForCube3D(std::vector<Vertex_PCUTBN>& verts, std::vector<unsigned int>& indexes, const AABB3& bounds, const Rgba8& color /*= Rgba8::WHITE*/, const AABB2& UVs /*= AABB2::ZERO_TO_ONE*/)
{
	//float maxX = bounds.m_maxs.x;
	//float maxY = bounds.m_maxs.y;
	//float maxZ = bounds.m_maxs.z;
	//float minX = bounds.m_mins.x;
	//float minY = bounds.m_mins.y;
	//float minZ = bounds.m_mins.z;

	//AddVertsForQuad3D(verts, indexes, Vec3(maxX, minY, minZ), Vec3(maxX, maxY, minZ), Vec3(maxX, maxY, maxZ), Vec3(maxX, minY, maxZ), color, UVs);//right
	//AddVertsForQuad3D(verts, indexes, Vec3(minX, maxY, minZ), Vec3(minX, minY, minZ), Vec3(minX, minY, maxZ), Vec3(minX, maxY, maxZ), color, UVs);//left
	//
	//AddVertsForQuad3D(verts, indexes, Vec3(maxX, maxY, minZ), Vec3(minX, maxY, minZ), Vec3(minX, maxY, maxZ), Vec3(maxX, maxY, maxZ), color, UVs);//front
	//AddVertsForQuad3D(verts, indexes, Vec3(minX, minY, minZ), Vec3(maxX, minY, minZ), Vec3(maxX, minY, maxZ), Vec3(minX, minY, maxZ), color, UVs);//back
	//
	//AddVertsForQuad3D(verts, indexes, Vec3(minX, minY, maxZ), Vec3(maxX, minY, maxZ), Vec3(maxX, maxY, maxZ), Vec3(minX, maxY, maxZ), color, UVs);//up
	//AddVertsForQuad3D(verts, indexes, Vec3(minX, maxY, minZ), Vec3(maxX, maxY, minZ), Vec3(maxX, minY, minZ), Vec3(minX, minY, minZ), color, UVs);//down


	Vec3 ESB = Vec3(bounds.m_maxs.x, bounds.m_mins.y, bounds.m_mins.z);
	Vec3 ENB = Vec3(bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_mins.z);
	Vec3 ENT = Vec3(bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_maxs.z);
	Vec3 EST = Vec3(bounds.m_maxs.x, bounds.m_mins.y, bounds.m_maxs.z);
	Vec3 WNB = Vec3(bounds.m_mins.x, bounds.m_maxs.y, bounds.m_mins.z);
	Vec3 WSB = Vec3(bounds.m_mins.x, bounds.m_mins.y, bounds.m_mins.z);
	Vec3 WST = Vec3(bounds.m_mins.x, bounds.m_mins.y, bounds.m_maxs.z);
	Vec3 WNT = Vec3(bounds.m_mins.x, bounds.m_maxs.y, bounds.m_maxs.z);
	AddVertsForQuad3D(verts, indexes, ESB, ENB, ENT, EST, color, UVs); // EAST FACE +X
	AddVertsForQuad3D(verts, indexes, WNB, WSB, WST, WNT, color, UVs); // WEST FACE -X
	AddVertsForQuad3D(verts, indexes, ENB, WNB, WNT, ENT, color, UVs); // NORTH FACE +Y
	AddVertsForQuad3D(verts, indexes, WSB, ESB, EST, WST, color, UVs); // SOUTH FACE -Y
	AddVertsForQuad3D(verts, indexes, WST, EST, ENT, WNT, color, UVs); // TOP FACE +Z
	AddVertsForQuad3D(verts, indexes, WNB, ENB, ESB, WSB, color, UVs); // BOTTOM FACE -Z
}



void AddVertsForLineWithLinePrimitive(std::vector<Vertex_PCU>& verts, Vec3 const& start, Vec3 const& end, const Rgba8& color /*= Rgba8::WHITE*/)
{
	verts.push_back(Vertex_PCU(start, Rgba8::WHITE));
	verts.push_back(Vertex_PCU(end, color));
}

void AddVertsForRoundedAABB3D(std::vector<Vertex_PCUTBN>& verts, const AABB3& bounds, const Rgba8& color /*= Rgba8::WHITE*/, const AABB2& UVs /*= AABB2::ZERO_TO_ONE*/)
{
	float maxX = bounds.m_maxs.x;
	float maxY = bounds.m_maxs.y;
	float maxZ = bounds.m_maxs.z;
	float minX = bounds.m_mins.x;
	float minY = bounds.m_mins.y;
	float minZ = bounds.m_mins.z;

	AddVertsForRoundedQuad3D(verts, Vec3(maxX, minY, minZ), Vec3(maxX, maxY, minZ), Vec3(maxX, maxY, maxZ), Vec3(maxX, minY, maxZ), color, UVs);//right
	AddVertsForRoundedQuad3D(verts, Vec3(minX, maxY, minZ), Vec3(minX, minY, minZ), Vec3(minX, minY, maxZ), Vec3(minX, maxY, maxZ), color, UVs);//left

	AddVertsForRoundedQuad3D(verts, Vec3(maxX, maxY, minZ), Vec3(minX, maxY, minZ), Vec3(minX, maxY, maxZ), Vec3(maxX, maxY, maxZ), color, UVs);//front
	AddVertsForRoundedQuad3D(verts, Vec3(minX, minY, minZ), Vec3(maxX, minY, minZ), Vec3(maxX, minY, maxZ), Vec3(minX, minY, maxZ), color, UVs);//back

	AddVertsForRoundedQuad3D(verts, Vec3(minX, minY, maxZ), Vec3(maxX, minY, maxZ), Vec3(maxX, maxY, maxZ), Vec3(minX, maxY, maxZ), color, UVs);//up
	AddVertsForRoundedQuad3D(verts, Vec3(minX, maxY, minZ), Vec3(maxX, maxY, minZ), Vec3(maxX, minY, minZ), Vec3(minX, minY, minZ), color, UVs);//down
}

// void AddVertsForRoundedAABB3D(std::vector<Vertex_PNCU>& verts, const AABB3& bounds, const Rgba8& color /*= Rgba8::WHITE*/, const AABB2& UVs /*= AABB2::ZERO_TO_ONE*/)
// {
// 	float maxX = bounds.m_maxs.x;
// 	float maxY = bounds.m_maxs.y;
// 	float maxZ = bounds.m_maxs.z;
// 	float minX = bounds.m_mins.x;
// 	float minY = bounds.m_mins.y;
// 	float minZ = bounds.m_mins.z;
// 
// 	AddVertsForRoundedQuad3D(verts, Vec3(maxX, minY, minZ), Vec3(maxX, maxY, minZ), Vec3(maxX, maxY, maxZ), Vec3(maxX, minY, maxZ), color, UVs);//right
// 	AddVertsForRoundedQuad3D(verts, Vec3(minX, maxY, minZ), Vec3(minX, minY, minZ), Vec3(minX, minY, maxZ), Vec3(minX, maxY, maxZ), color, UVs);//left
// 
// 	AddVertsForRoundedQuad3D(verts, Vec3(maxX, maxY, minZ), Vec3(minX, maxY, minZ), Vec3(minX, maxY, maxZ), Vec3(maxX, maxY, maxZ), color, UVs);//front
// 	AddVertsForRoundedQuad3D(verts, Vec3(minX, minY, minZ), Vec3(maxX, minY, minZ), Vec3(maxX, minY, maxZ), Vec3(minX, minY, maxZ), color, UVs);//back
// 
// 	AddVertsForRoundedQuad3D(verts, Vec3(minX, minY, maxZ), Vec3(maxX, minY, maxZ), Vec3(maxX, maxY, maxZ), Vec3(minX, maxY, maxZ), color, UVs);//up
// 	AddVertsForRoundedQuad3D(verts, Vec3(minX, maxY, minZ), Vec3(maxX, maxY, minZ), Vec3(maxX, minY, minZ), Vec3(minX, minY, minZ), color, UVs);//down
// }

void AddVertsForRoundedQuad3D(std::vector<Vertex_PCUTBN>& verts, const Vec3& bottomLeft, const Vec3& bottomRight, const Vec3& topRight, const Vec3& topLeft,const Rgba8& color /*= Rgba8::WHITE*/, const AABB2& UVs /*= AABB2::ZERO_TO_ONE*/)
{
	int NUM_TRIS = 2;
	int NUM_VERTS = 3 * NUM_TRIS;
	verts.reserve(verts.size() + NUM_VERTS);

	Vec3 topMiddle = 0.5f * (topLeft + topRight);
	Vec3 bottomMiddle = 0.5f * (bottomLeft + bottomRight);
	Vec3 middleUp = topMiddle - bottomMiddle;

	//Normals
	Vec3 halfLeftToRight = topRight - topMiddle;
	Vec3 halfRightToLeft = topLeft - topMiddle;
	Vec3 middleForward = CrossProduct3D(halfLeftToRight, middleUp);
	
	//UVs
	Vec2 middleBottomUVs = UVs.m_mins + Vec2(0.5f * (UVs.m_maxs - UVs.m_mins).x, 0.f);
	Vec2 middleTopUVs = UVs.m_mins + Vec2(0.5f * (UVs.m_maxs - UVs.m_mins).x, (UVs.m_maxs - UVs.m_mins).y);


	verts.push_back(Vertex_PCUTBN(Vertex_PNCU(bottomLeft, halfRightToLeft, color, UVs.m_mins)));
	verts.push_back(Vertex_PCUTBN(Vertex_PNCU(bottomMiddle, middleForward, color, middleBottomUVs)));
	verts.push_back(Vertex_PCUTBN(Vertex_PNCU(topMiddle, middleForward, color, middleTopUVs)));

	verts.push_back(Vertex_PCUTBN(Vertex_PNCU(topMiddle, middleForward, color, middleTopUVs)));
	verts.push_back(Vertex_PCUTBN(Vertex_PNCU(topLeft, middleForward, color, Vec2(UVs.m_mins.x, UVs.m_maxs.y))));
	verts.push_back(Vertex_PCUTBN(Vertex_PNCU(bottomLeft, halfRightToLeft, color, UVs.m_mins)));

	verts.push_back(Vertex_PCUTBN(Vertex_PNCU(bottomMiddle, middleForward, color, middleBottomUVs)));
	verts.push_back(Vertex_PCUTBN(Vertex_PNCU(bottomRight, halfLeftToRight, color, Vec2(UVs.m_maxs.x, UVs.m_mins.y))));//
	verts.push_back(Vertex_PCUTBN(Vertex_PNCU(topRight, halfLeftToRight, color, UVs.m_maxs)));

	verts.push_back(Vertex_PCUTBN(Vertex_PNCU(topRight, halfLeftToRight, color, UVs.m_maxs)));
	verts.push_back(Vertex_PCUTBN(Vertex_PNCU(topMiddle, middleForward, color, middleTopUVs)));
	verts.push_back(Vertex_PCUTBN(Vertex_PNCU(bottomMiddle, middleForward, color, middleBottomUVs)));

}

// void AddVertsForRoundedQuad3D(std::vector<Vertex_PNCU>& verts, const Vec3& bottomLeft, const Vec3& bottomRight, const Vec3& topRight, const Vec3& topLeft, const Rgba8& color /*= Rgba8::WHITE*/, const AABB2& UVs /*= AABB2::ZERO_TO_ONE*/)
// {
// 
// }

void AddVertsForAABBWireframe3D(std::vector<Vertex_PCU>& verts, AABB3 const& bounds, float lineThickness, Rgba8 const& tint /*= Rgba8::WHITE*/)
{
	float minX = bounds.m_mins.x;
	float maxX = bounds.m_maxs.x;
	float minY = bounds.m_mins.y;
	float maxY = bounds.m_maxs.y;
	float minZ = bounds.m_mins.z;
	float maxZ = bounds.m_maxs.z;

	// South west point 3 lines
	AddVertsForLineSegment3D(verts, Vec3(minX, minY, minZ), Vec3(maxX, minY, minZ), lineThickness, tint);
	AddVertsForLineSegment3D(verts, Vec3(minX, minY, minZ), Vec3(minX, maxY, minZ), lineThickness, tint);
	AddVertsForLineSegment3D(verts, Vec3(minX, minY, minZ), Vec3(minX, minY, maxZ), lineThickness, tint);

	AddVertsForLineSegment3D(verts, Vec3(maxX, maxY, minZ), Vec3(maxX, maxY, maxZ), lineThickness, tint);
	AddVertsForLineSegment3D(verts, Vec3(maxX, maxY, minZ), Vec3(maxX, minY, minZ), lineThickness, tint);
	AddVertsForLineSegment3D(verts, Vec3(maxX, maxY, minZ), Vec3(minX, maxY, minZ), lineThickness, tint);

	AddVertsForLineSegment3D(verts, Vec3(minX, maxY, maxZ), Vec3(minX, maxY, minZ), lineThickness, tint);
	AddVertsForLineSegment3D(verts, Vec3(minX, maxY, maxZ), Vec3(minX, minY, maxZ), lineThickness, tint);
	AddVertsForLineSegment3D(verts, Vec3(minX, maxY, maxZ), Vec3(maxX, maxY, maxZ), lineThickness, tint);

	AddVertsForLineSegment3D(verts, Vec3(maxX, minY, maxZ), Vec3(maxX, minY, minZ), lineThickness, tint);
	AddVertsForLineSegment3D(verts, Vec3(maxX, minY, maxZ), Vec3(minX, minY, maxZ), lineThickness, tint);
	AddVertsForLineSegment3D(verts, Vec3(maxX, minY, maxZ), Vec3(maxX, maxY, maxZ), lineThickness, tint);
}

void AddVertsForCylinderZ3D(std::vector<Vertex_PCU>& verts, Vec2 const& centerXY, FloatRange const& minMaxZ, float radius, float numSlices, Rgba8 const& tint /*= Rgba8::WHITE*/, AABB2 const& UVs /*= AABB2::ZERO_TO_ONE*/)
{
	int NUM_SIDES = (int)numSlices;
	int NUM_TRIS = 4 * NUM_SIDES;
	int NUM_VERTS = 3 * NUM_TRIS;
	float DEFREE_PER_SIDE = 360.f / static_cast<float>(NUM_SIDES);
	float deltaXUVs = (UVs.GetDimensions().x / numSlices);

	verts.reserve(verts.size() + NUM_VERTS);
	
	for (int sideNum = 0; sideNum < NUM_SIDES; sideNum++) {
		
		float startDegrees = DEFREE_PER_SIDE * static_cast<float>(sideNum);
		Vec2 startVec2 = Vec2::MakeFromPolarDegrees(startDegrees, radius)+ centerXY;
		float endDegrees = DEFREE_PER_SIDE * static_cast<float>(sideNum + 1);
		Vec2 endVec2 = Vec2::MakeFromPolarDegrees(endDegrees, radius)+ centerXY;

		//Top
		Vec3 startPointTop = Vec3(startVec2.x, startVec2.y, minMaxZ.m_max); 
		Vec3 endPointTop = Vec3(endVec2.x, endVec2.y, minMaxZ.m_max);
		Vec3 centerVec3Top = Vec3(centerXY.x, centerXY.y, minMaxZ.m_max);

		//Bottom
		Vec3 startPointBottom = Vec3(startVec2.x, startVec2.y, minMaxZ.m_min);
		Vec3 centerVec3Bottom = Vec3(centerXY.x, centerXY.y, minMaxZ.m_min);
		Vec3 endPointBottom = Vec3(endVec2.x, endVec2.y, minMaxZ.m_min);
		
		Vec2 centerUVs = UVs.m_mins + UVs.GetDimensions() * 0.5f;
		Vec2 startUVs = centerUVs + Vec2::MakeFromPolarDegrees(startDegrees) * centerUVs;
		Vec2 endUVs = centerUVs + Vec2::MakeFromPolarDegrees(endDegrees) * centerUVs;

		verts.push_back(Vertex_PCU(centerVec3Top, tint, centerUVs));
		verts.push_back(Vertex_PCU(startPointTop, tint, startUVs));
		verts.push_back(Vertex_PCU(endPointTop, tint, endUVs));

		verts.push_back(Vertex_PCU(centerVec3Bottom, tint, centerUVs));
		verts.push_back(Vertex_PCU(endPointBottom, tint, endUVs));
		verts.push_back(Vertex_PCU(startPointBottom, tint, startUVs));

		//Vertical Quad

		float quadUVsX = deltaXUVs * (float)sideNum;
		float quadUVsXNext = deltaXUVs * (float)(sideNum + 1);

		AABB2 quadUVs = AABB2(quadUVsX, UVs.m_mins.y, quadUVsXNext, UVs.m_maxs.y);
		AddVertsForQuad3D(verts, startPointBottom, endPointBottom, endPointTop, startPointTop, tint, quadUVs);
	}

}

void AddVertsForCylinderZWireframe3D(std::vector<Vertex_PCU>& verts, Vec2 const& centerXY, FloatRange const& minMaxZ, float radius, float numSlices, float lineThickness, Rgba8 const& tint /*= Rgba8::WHITE*/)
{
	int NUM_SIDES = (int)numSlices;
	int NUM_TRIS = 4 * NUM_SIDES;
	int NUM_VERTS = 3 * NUM_TRIS;
	float DEFREE_PER_SIDE = 360.f / static_cast<float>(NUM_SIDES);

	verts.reserve(NUM_VERTS);

	for (int sideNum = 0; sideNum < NUM_SIDES; sideNum++) {

		float startDegrees = DEFREE_PER_SIDE * static_cast<float>(sideNum);
		Vec2 startVec2 = Vec2::MakeFromPolarDegrees(startDegrees, radius) + centerXY;
		float endDegrees = DEFREE_PER_SIDE * static_cast<float>(sideNum + 1);
		Vec2 endVec2 = Vec2::MakeFromPolarDegrees(endDegrees, radius) + centerXY;

		//Top
		Vec3 startPointTop = Vec3(startVec2.x, startVec2.y, minMaxZ.m_max);
		Vec3 endPointTop = Vec3(endVec2.x, endVec2.y, minMaxZ.m_max);
		Vec3 centerVec3Top = Vec3(centerXY.x, centerXY.y, minMaxZ.m_max);

		//Bottom
		Vec3 startPointBottom = Vec3(startVec2.x, startVec2.y, minMaxZ.m_min);
		Vec3 centerVec3Bottom = Vec3(centerXY.x, centerXY.y, minMaxZ.m_min);
		Vec3 endPointBottom = Vec3(endVec2.x, endVec2.y, minMaxZ.m_min);

		AddVertsForLineSegment3D(verts, centerVec3Top, startPointTop, lineThickness, tint);
		AddVertsForLineSegment3D(verts, startPointTop, endPointTop, lineThickness, tint);

		AddVertsForLineSegment3D(verts, centerVec3Bottom, startPointBottom, lineThickness, tint);
		AddVertsForLineSegment3D(verts, startPointBottom, endPointBottom, lineThickness, tint);

		AddVertsForLineSegment3D(verts, startPointBottom, startPointTop, lineThickness, tint);
		AddVertsForLineSegment3D(verts, startPointBottom, endPointTop, lineThickness, tint);

	}

}

void AddVertsForAABB3D(std::vector<Vertex_PCU>& verts, const AABB3& bounds, const Rgba8& color /*= Rgba8::WHITE*/, const AABB2& UVs /*= AABB2::ZERO_TO_ONE*/)
{
	float maxX = bounds.m_maxs.x;
	float maxY = bounds.m_maxs.y;
	float maxZ = bounds.m_maxs.z;
	float minX = bounds.m_mins.x;
	float minY = bounds.m_mins.y;
	float minZ = bounds.m_mins.z;

	AddVertsForQuad3D(verts, Vec3(maxX, minY, minZ), Vec3(maxX, maxY, minZ), Vec3(maxX, maxY, maxZ), Vec3(maxX, minY, maxZ), color, UVs);//right
	AddVertsForQuad3D(verts, Vec3(minX, maxY, minZ), Vec3(minX, minY, minZ), Vec3(minX, minY, maxZ), Vec3(minX, maxY, maxZ), color, UVs);//left

	AddVertsForQuad3D(verts, Vec3(maxX, maxY, minZ), Vec3(minX, maxY, minZ), Vec3(minX, maxY, maxZ), Vec3(maxX, maxY, maxZ), color, UVs);//front
	AddVertsForQuad3D(verts, Vec3(minX, minY, minZ), Vec3(maxX, minY, minZ), Vec3(maxX, minY, maxZ), Vec3(minX, minY, maxZ), color, UVs);//back

	AddVertsForQuad3D(verts, Vec3(minX, minY, maxZ), Vec3(maxX, minY, maxZ), Vec3(maxX, maxY, maxZ), Vec3(minX, maxY, maxZ), color, UVs);//up
	AddVertsForQuad3D(verts, Vec3(minX, maxY, minZ), Vec3(maxX, maxY, minZ), Vec3(maxX, minY, minZ), Vec3(minX, minY, minZ), color, UVs);//down
}

void AddVertsForSphere3D(std::vector<Vertex_PCU>& verts, const Vec3& center, float radius, const Rgba8& color /*= Rgba8::WHITE*/, 
	const AABB2& UVs /*= AABB2::ZERO_TO_ONE*/, int numLatitudeSlice /*= 8*/)
{
	int NUM_LAYER = numLatitudeSlice;
	int NUM_SLICE = 16;

	float deltaLatitudeDegree = 180.f / NUM_LAYER;
	float deltaLongtitudeDegree = 360.f / NUM_SLICE;

	for (int layerIndex = 0; layerIndex < NUM_LAYER; layerIndex++)
	{
		for (int sliceIndex = 0; sliceIndex < NUM_SLICE; sliceIndex++) 
		{
			float currentLatitudeDegree = -90.f + layerIndex * deltaLatitudeDegree;
			float currentLongtitudeDegree = sliceIndex * deltaLongtitudeDegree;
			Vec3 BL = center + Vec3::MakeFromPolarDegrees(currentLatitudeDegree, currentLongtitudeDegree, radius);
			Vec3 BR = center + Vec3::MakeFromPolarDegrees(currentLatitudeDegree, currentLongtitudeDegree + deltaLongtitudeDegree, radius);
			Vec3 TL = center + Vec3::MakeFromPolarDegrees(currentLatitudeDegree + deltaLatitudeDegree, currentLongtitudeDegree, radius);
			Vec3 TR = center + Vec3::MakeFromPolarDegrees(currentLatitudeDegree + deltaLatitudeDegree, 
				currentLongtitudeDegree + deltaLongtitudeDegree, radius);
 			//calculate UV
			float minV = layerIndex * (UVs.GetDimensions().y / (float)NUM_LAYER);
			float maxV = minV + (UVs.GetDimensions().y / (float)NUM_LAYER);
			float minU = 0.f + sliceIndex * (UVs.GetDimensions().x / (float)NUM_SLICE);
			float maxU = minU + (UVs.GetDimensions().x / (float)NUM_SLICE);
			AABB2 aabb = AABB2(Vec2(minU, minV), Vec2(maxU, maxV));
			AddVertsForQuad3D(verts, BL, BR, TR, TL, color, aabb);
		}
	}

}

void AddVertsForUVSphereZ3D(std::vector<Vertex_PCU>& verts, const Vec3& center, float radius, float numSlices, float numStacks, const Rgba8& tint /*= Rgba8::WHITE*/, AABB2 const& UVs /*= AABB2::ZERO_TO_ONE*/)
{
	int NUM_LAYER = (int)numStacks;
	int NUM_SLICE = (int)numSlices;

	float deltaLatitudeDegree = 180.f / NUM_LAYER;
	float deltaLongtitudeDegree = 360.f / NUM_SLICE;

	for (int layerIndex = 0; layerIndex < NUM_LAYER; layerIndex++)
	{
		for (int sliceIndex = 0; sliceIndex < NUM_SLICE; sliceIndex++)
		{
			float currentLatitudeDegree = -90.f + layerIndex * deltaLatitudeDegree;
			float currentLongtitudeDegree = sliceIndex * deltaLongtitudeDegree;
			Vec3 BL = center + Vec3::MakeFromPolarDegrees(currentLatitudeDegree, currentLongtitudeDegree, radius);
			Vec3 BR = center + Vec3::MakeFromPolarDegrees(currentLatitudeDegree, currentLongtitudeDegree + deltaLongtitudeDegree, radius);
			Vec3 TL = center + Vec3::MakeFromPolarDegrees(currentLatitudeDegree + deltaLatitudeDegree, currentLongtitudeDegree, radius);
			Vec3 TR = center + Vec3::MakeFromPolarDegrees(currentLatitudeDegree + deltaLatitudeDegree,
				currentLongtitudeDegree + deltaLongtitudeDegree, radius);
			//calculate UV
			float minV = layerIndex * (UVs.GetDimensions().y / (float)NUM_LAYER);
			float maxV = minV + (UVs.GetDimensions().y / (float)NUM_LAYER);
			float minU = 0.f + sliceIndex * (UVs.GetDimensions().x / (float)NUM_SLICE);
			float maxU = minU + (UVs.GetDimensions().x / (float)NUM_SLICE);
			AABB2 aabb = AABB2(Vec2(minU, minV), Vec2(maxU, maxV));
			AddVertsForQuad3D(verts, BL, BR, TR, TL, tint, aabb);
		}
	}
}

void AddVertsForUVSphereZ3D(std::vector<Vertex_PCUTBN>& verts, std::vector<unsigned int>& indexes, const Vec3& center, float radius, float numSlices, float numStacks, const Rgba8& tint /*= Rgba8::WHITE*/, AABB2 const& UVs /*= AABB2::ZERO_TO_ONE*/)
{
	int NUM_LAYER = (int)numStacks;
	int NUM_SLICE = (int)numSlices;
	
	int vertsNum = 2 + (NUM_LAYER - 1) * NUM_SLICE;
	int indexNum = 3 * (((NUM_LAYER - 2) * NUM_SLICE * 2) + (2 * NUM_SLICE));
	verts.reserve(verts.size() + vertsNum);
	indexes.reserve(indexes.size() + indexNum);

	int vertStartIndex = (int)verts.size();
	int NUM_VERTS_PER_LAYER = NUM_SLICE;

	float pitchPerStack = 180.f / NUM_LAYER;//Pitch
	float yawPerSlice = 360.f / NUM_SLICE;//Yaw

	for (int layerIndex = 0; layerIndex < 1; layerIndex++)
	{
		for (int sliceIndex = 0; sliceIndex < NUM_SLICE - 1; sliceIndex++)
		{
			float currentPitch = -90.f + layerIndex * pitchPerStack;
			float currentYaw = sliceIndex * yawPerSlice;
			
			Vec3 TR = center + Vec3::MakeFromPolarDegrees(currentPitch + pitchPerStack, currentYaw + yawPerSlice, radius);
			//calculate UV
			float minV = layerIndex * (UVs.GetDimensions().y / (float)NUM_LAYER);
			float maxV = minV + (UVs.GetDimensions().y / (float)NUM_LAYER);
			float minU = 0.f + sliceIndex * (UVs.GetDimensions().x / (float)NUM_SLICE);
			float maxU = minU + (UVs.GetDimensions().x / (float)NUM_SLICE);

			//Verts
			if (sliceIndex == 0) 
			{
				Vec3 BL = center + Vec3::MakeFromPolarDegrees(currentPitch, currentYaw, radius);
				Vec3 normalBL = Vec3::MakeFromPolarDegrees(currentPitch, currentYaw, radius);
				verts.push_back(Vertex_PCUTBN(BL, tint, Vec2(minU, minV), Vec3::WORLD_ORIGIN, Vec3::WORLD_ORIGIN, normalBL));

				Vec3 TL = center + Vec3::MakeFromPolarDegrees(currentPitch + pitchPerStack, currentYaw, radius);
				Vec3 normalTL = Vec3::MakeFromPolarDegrees(currentPitch + pitchPerStack, currentYaw, radius);
				verts.push_back(Vertex_PCUTBN(TL, tint, Vec2(minU, maxV), Vec3::WORLD_ORIGIN, Vec3::WORLD_ORIGIN, normalTL));
			}
			Vec3 normalTR = Vec3::MakeFromPolarDegrees(currentPitch + pitchPerStack, currentYaw + yawPerSlice, radius);
			verts.push_back(Vertex_PCUTBN(TR, tint, Vec2(maxU, maxV), Vec3::WORLD_ORIGIN, Vec3::WORLD_ORIGIN, normalTR));
		}

		for (int sliceIndex = 0; sliceIndex < NUM_SLICE - 1; sliceIndex++) 
		{
			//Calculate Verts No.
			int TRIndex, TLIndex = 0;
			TRIndex = 1 + (sliceIndex + 1);
			TLIndex = 1 + (sliceIndex);

			indexes.push_back(vertStartIndex);
			indexes.push_back(TRIndex+ vertStartIndex);
			indexes.push_back(TLIndex+ vertStartIndex);

			if (sliceIndex == NUM_SLICE - 1 - 1) 
			{
				int TR2Index, TL2Index = 0;
				TR2Index = 1;
				TL2Index = TRIndex;

				indexes.push_back(vertStartIndex);
				indexes.push_back(TR2Index + vertStartIndex);
				indexes.push_back(TL2Index + vertStartIndex);
			}

		}
	}
	for (int layerIndex = 1; layerIndex < NUM_LAYER - 1; layerIndex++) 
	{
		for (int sliceIndex = 0; sliceIndex < NUM_SLICE - 1; sliceIndex++)
		{
			float currentLatitudeDegree = -90.f + layerIndex * pitchPerStack;
			float currentLongtitudeDegree = sliceIndex * yawPerSlice;
			Vec3 BL = center + Vec3::MakeFromPolarDegrees(currentLatitudeDegree, currentLongtitudeDegree, radius);
			Vec3 BR = center + Vec3::MakeFromPolarDegrees(currentLatitudeDegree, currentLongtitudeDegree + yawPerSlice, radius);
			Vec3 TL = center + Vec3::MakeFromPolarDegrees(currentLatitudeDegree + pitchPerStack, currentLongtitudeDegree, radius);
			Vec3 TR = center + Vec3::MakeFromPolarDegrees(currentLatitudeDegree + pitchPerStack, currentLongtitudeDegree + yawPerSlice, radius);
			//calculate UV
			float minV = layerIndex * (UVs.GetDimensions().y / (float)NUM_LAYER);
			float maxV = minV + (UVs.GetDimensions().y / (float)NUM_LAYER);
			float minU = 0.f + sliceIndex * (UVs.GetDimensions().x / (float)NUM_SLICE);
			float maxU = minU + (UVs.GetDimensions().x / (float)NUM_SLICE);

			//TL and TR
			
			if (sliceIndex == 0) 
			{
				Vec3 normalTL = Vec3::MakeFromPolarDegrees(currentLatitudeDegree + pitchPerStack, currentLongtitudeDegree, radius);
				verts.push_back(Vertex_PCUTBN(TL, tint, Vec2(minU, maxV), Vec3::WORLD_ORIGIN, Vec3::WORLD_ORIGIN, normalTL));
			}

			Vec3 normalTR = Vec3::MakeFromPolarDegrees(currentLatitudeDegree + pitchPerStack, currentLongtitudeDegree + yawPerSlice, radius);
			verts.push_back(Vertex_PCUTBN(TR, tint, Vec2(maxU, maxV), Vec3::WORLD_ORIGIN, Vec3::WORLD_ORIGIN, normalTR));

		}

		for (int sliceIndex = 0; sliceIndex < NUM_SLICE - 1; sliceIndex++) 
		{
			//Calculate Verts No.
			int BLIndex, BRIndex, TRIndex, TLIndex = 0;

			BLIndex = 1 + ((layerIndex - 1) * NUM_VERTS_PER_LAYER) + (sliceIndex);//1+(1-1)*5+(0+1)=2
			TLIndex = 1 + ((layerIndex)*NUM_VERTS_PER_LAYER) + (sliceIndex);//1+(1)*5+(0+1)=8
			BRIndex = 1 + ((layerIndex - 1) * NUM_VERTS_PER_LAYER) + (sliceIndex + 1);//1+(1)*5+(0+1)=8
			TRIndex = 1 + ((layerIndex)*NUM_VERTS_PER_LAYER) + (sliceIndex + 1);//1+(1)*5+(0+1)=8

			indexes.push_back(BLIndex + vertStartIndex);
			indexes.push_back(BRIndex + vertStartIndex);
			indexes.push_back(TRIndex + vertStartIndex);

			indexes.push_back(TRIndex + vertStartIndex);
			indexes.push_back(TLIndex + vertStartIndex);
			indexes.push_back(BLIndex + vertStartIndex);

			if (sliceIndex == NUM_SLICE - 1 - 1) 
			{
				//The last TR is first TL

				int firstTLIndex = 1 + ((layerIndex)*NUM_VERTS_PER_LAYER) + (0);
				int lastTRIndex = firstTLIndex;
				int lastTLIndex = 1 + ((layerIndex)*NUM_VERTS_PER_LAYER) + (sliceIndex + 1);

				int lastBLIndex = 1 + ((layerIndex - 1) * NUM_VERTS_PER_LAYER) + (sliceIndex + 1);
				//The last BR is first TL of previous layer
				int lastBRIndex = 1 + ((layerIndex - 1) * NUM_VERTS_PER_LAYER) + (0);

				indexes.push_back(lastBLIndex + vertStartIndex);
				indexes.push_back(lastBRIndex + vertStartIndex);
				indexes.push_back(lastTRIndex + vertStartIndex);

				indexes.push_back(lastTRIndex + vertStartIndex);
				indexes.push_back(lastTLIndex + vertStartIndex);
				indexes.push_back(lastBLIndex + vertStartIndex);
			}
		}

	}

	for (int layerIndex = NUM_LAYER-1; layerIndex < NUM_LAYER; layerIndex++)
	{

		int EndsliceIndex = 0;
		float currentPitch = -90.f + layerIndex * pitchPerStack;
		float currentYaw = EndsliceIndex * yawPerSlice;

		Vec3 TR = center + Vec3::MakeFromPolarDegrees(currentPitch + pitchPerStack, currentYaw + yawPerSlice, radius);
		//calculate UV
		float minV = layerIndex * (UVs.GetDimensions().y / (float)NUM_LAYER);
		float maxV = minV + (UVs.GetDimensions().y / (float)NUM_LAYER);
		float minU = 0.f + EndsliceIndex * (UVs.GetDimensions().x / (float)NUM_SLICE);
		float maxU = minU + (UVs.GetDimensions().x / (float)NUM_SLICE);

		Vec3 normalTR = Vec3::MakeFromPolarDegrees(currentPitch + pitchPerStack, currentYaw + yawPerSlice, radius);
		verts.push_back(Vertex_PCUTBN(TR, tint, Vec2(maxU, maxV), Vec3::WORLD_ORIGIN, Vec3::WORLD_ORIGIN, normalTR));

		int vertsEndIndex = (int)verts.size() - 1;//21
		for (int sliceIndex = 0; sliceIndex < NUM_SLICE - 1; sliceIndex++)
		{
			//Calculate Verts No.
			int BLIndex, BRIndex = 0;

			BLIndex = 1 + ((layerIndex - 1) * NUM_VERTS_PER_LAYER) + (sliceIndex);//1+(1-1)*5+(0+1)=2
			BRIndex = 1 + ((layerIndex - 1) * NUM_VERTS_PER_LAYER) + (sliceIndex + 1);//1+(1)*5+(0+1)=8

			indexes.push_back(BLIndex + vertStartIndex);
			indexes.push_back(BRIndex + vertStartIndex);
			indexes.push_back(vertsEndIndex);

			if (sliceIndex == NUM_SLICE - 1 - 1) 
			{
				//First BL is last BR
				int firstBLIndex = 1 + ((layerIndex - 1) * NUM_VERTS_PER_LAYER) + (0);
				int lastBRIndex = firstBLIndex;
				int lastBLIndex = 1 + ((layerIndex - 1) * NUM_VERTS_PER_LAYER) + (sliceIndex + 1);
				indexes.push_back(lastBLIndex + vertStartIndex);
				indexes.push_back(lastBRIndex + vertStartIndex);
				indexes.push_back(vertsEndIndex);
			}

		}
	}
}

void AddVertsForUVSphereZ3DWithPNCU(std::vector<Vertex_PNCU>& verts, std::vector<unsigned int>& indexes, const Vec3& center, float radius, float numSlices, float numStacks, const Rgba8& tint /*= Rgba8::WHITE*/, AABB2 const& UVs /*= AABB2::ZERO_TO_ONE*/)
{
	int NUM_LAYER = (int)numStacks;
	int NUM_SLICE = (int)numSlices;

	int vertsNum = 2 + (NUM_LAYER - 1) * NUM_SLICE;
	int indexNum = 3 * (((NUM_LAYER - 2) * NUM_SLICE * 2) + (2 * NUM_SLICE));
	verts.reserve(verts.size() + vertsNum);
	indexes.reserve(indexes.size() + indexNum);

	int vertStartIndex = (int)verts.size();
	int NUM_VERTS_PER_LAYER = NUM_SLICE;

	float pitchPerStack = 180.f / NUM_LAYER;//Pitch
	float yawPerSlice = 360.f / NUM_SLICE;//Yaw

	for (int layerIndex = 0; layerIndex < 1; layerIndex++)
	{
		for (int sliceIndex = 0; sliceIndex < NUM_SLICE - 1; sliceIndex++)
		{
			float currentPitch = -90.f + layerIndex * pitchPerStack;
			float currentYaw = sliceIndex * yawPerSlice;

			Vec3 TR = center + Vec3::MakeFromPolarDegrees(currentPitch + pitchPerStack, currentYaw + yawPerSlice, radius);
			//calculate UV
			float minV = layerIndex * (UVs.GetDimensions().y / (float)NUM_LAYER);
			float maxV = minV + (UVs.GetDimensions().y / (float)NUM_LAYER);
			float minU = 0.f + sliceIndex * (UVs.GetDimensions().x / (float)NUM_SLICE);
			float maxU = minU + (UVs.GetDimensions().x / (float)NUM_SLICE);

			//Verts
			if (sliceIndex == 0)
			{
				Vec3 BL = center + Vec3::MakeFromPolarDegrees(currentPitch, currentYaw, radius);
				Vec3 normalBL = Vec3::MakeFromPolarDegrees(currentPitch, currentYaw, radius);
				verts.push_back(Vertex_PNCU(BL, normalBL, tint, Vec2(minU, minV)));
				
				Vec3 TL = center + Vec3::MakeFromPolarDegrees(currentPitch + pitchPerStack, currentYaw, radius);
				Vec3 normalTL = Vec3::MakeFromPolarDegrees(currentPitch + pitchPerStack, currentYaw, radius);
				verts.push_back(Vertex_PNCU(TL, normalTL, tint, Vec2(minU, maxV)));
				
			}
			Vec3 normalTR = Vec3::MakeFromPolarDegrees(currentPitch + pitchPerStack, currentYaw + yawPerSlice, radius);
			verts.push_back(Vertex_PNCU(TR, normalTR, tint, Vec2(maxU, maxV)));
			
		}

		for (int sliceIndex = 0; sliceIndex < NUM_SLICE - 1; sliceIndex++)
		{
			//Calculate Verts No.
			int TRIndex, TLIndex = 0;
			TRIndex = 1 + (sliceIndex + 1);
			TLIndex = 1 + (sliceIndex);

			indexes.push_back(vertStartIndex);
			indexes.push_back(TRIndex + vertStartIndex);
			indexes.push_back(TLIndex + vertStartIndex);

			if (sliceIndex == NUM_SLICE - 1 - 1)
			{
				int TR2Index, TL2Index = 0;
				TR2Index = 1;
				TL2Index = TRIndex;

				indexes.push_back(vertStartIndex);
				indexes.push_back(TR2Index + vertStartIndex);
				indexes.push_back(TL2Index + vertStartIndex);
			}
		}
	}

	for (int layerIndex = 1; layerIndex < NUM_LAYER - 1; layerIndex++)
	{
		for (int sliceIndex = 0; sliceIndex < NUM_SLICE - 1; sliceIndex++)
		{
			float currentLatitudeDegree = -90.f + layerIndex * pitchPerStack;
			float currentLongtitudeDegree = sliceIndex * yawPerSlice;
			Vec3 BL = center + Vec3::MakeFromPolarDegrees(currentLatitudeDegree, currentLongtitudeDegree, radius);
			Vec3 BR = center + Vec3::MakeFromPolarDegrees(currentLatitudeDegree, currentLongtitudeDegree + yawPerSlice, radius);
			Vec3 TL = center + Vec3::MakeFromPolarDegrees(currentLatitudeDegree + pitchPerStack, currentLongtitudeDegree, radius);
			Vec3 TR = center + Vec3::MakeFromPolarDegrees(currentLatitudeDegree + pitchPerStack, currentLongtitudeDegree + yawPerSlice, radius);
			//calculate UV
			float minV = layerIndex * (UVs.GetDimensions().y / (float)NUM_LAYER);
			float maxV = minV + (UVs.GetDimensions().y / (float)NUM_LAYER);
			float minU = 0.f + sliceIndex * (UVs.GetDimensions().x / (float)NUM_SLICE);
			float maxU = minU + (UVs.GetDimensions().x / (float)NUM_SLICE);

			//TL and TR

			if (sliceIndex == 0)
			{
				Vec3 normalTL = Vec3::MakeFromPolarDegrees(currentLatitudeDegree + pitchPerStack, currentLongtitudeDegree, radius);
				verts.push_back(Vertex_PNCU(TL, normalTL, tint, Vec2(minU, maxV)));
				
			}

			Vec3 normalTR = Vec3::MakeFromPolarDegrees(currentLatitudeDegree + pitchPerStack, currentLongtitudeDegree + yawPerSlice, radius);
			verts.push_back(Vertex_PNCU(TR, normalTR, tint, Vec2(maxU, maxV)));
			
		}

		for (int sliceIndex = 0; sliceIndex < NUM_SLICE - 1; sliceIndex++)
		{
			//Calculate Verts No.
			int BLIndex, BRIndex, TRIndex, TLIndex = 0;

			BLIndex = 1 + ((layerIndex - 1) * NUM_VERTS_PER_LAYER) + (sliceIndex);//1+(1-1)*5+(0+1)=2
			TLIndex = 1 + ((layerIndex)*NUM_VERTS_PER_LAYER) + (sliceIndex);//1+(1)*5+(0+1)=8
			BRIndex = 1 + ((layerIndex - 1) * NUM_VERTS_PER_LAYER) + (sliceIndex + 1);//1+(1)*5+(0+1)=8
			TRIndex = 1 + ((layerIndex)*NUM_VERTS_PER_LAYER) + (sliceIndex + 1);//1+(1)*5+(0+1)=8

			indexes.push_back(BLIndex + vertStartIndex);
			indexes.push_back(BRIndex + vertStartIndex);
			indexes.push_back(TRIndex + vertStartIndex);

			indexes.push_back(TRIndex + vertStartIndex);
			indexes.push_back(TLIndex + vertStartIndex);
			indexes.push_back(BLIndex + vertStartIndex);

			if (sliceIndex == NUM_SLICE - 1 - 1)
			{
				//The last TR is first TL

				int firstTLIndex = 1 + ((layerIndex)*NUM_VERTS_PER_LAYER) + (0);
				int lastTRIndex = firstTLIndex;
				int lastTLIndex = 1 + ((layerIndex)*NUM_VERTS_PER_LAYER) + (sliceIndex + 1);

				int lastBLIndex = 1 + ((layerIndex - 1) * NUM_VERTS_PER_LAYER) + (sliceIndex + 1);
				//The last BR is first TL of previous layer
				int lastBRIndex = 1 + ((layerIndex - 1) * NUM_VERTS_PER_LAYER) + (0);

				indexes.push_back(lastBLIndex + vertStartIndex);
				indexes.push_back(lastBRIndex + vertStartIndex);
				indexes.push_back(lastTRIndex + vertStartIndex);

				indexes.push_back(lastTRIndex + vertStartIndex);
				indexes.push_back(lastTLIndex + vertStartIndex);
				indexes.push_back(lastBLIndex + vertStartIndex);
			}
		}

	}

	for (int layerIndex = NUM_LAYER - 1; layerIndex < NUM_LAYER; layerIndex++)
	{

		int EndsliceIndex = 0;
		float currentPitch = -90.f + layerIndex * pitchPerStack;
		float currentYaw = EndsliceIndex * yawPerSlice;

		Vec3 TR = center + Vec3::MakeFromPolarDegrees(currentPitch + pitchPerStack, currentYaw + yawPerSlice, radius);
		//calculate UV
		float minV = layerIndex * (UVs.GetDimensions().y / (float)NUM_LAYER);
		float maxV = minV + (UVs.GetDimensions().y / (float)NUM_LAYER);
		float minU = 0.f + EndsliceIndex * (UVs.GetDimensions().x / (float)NUM_SLICE);
		float maxU = minU + (UVs.GetDimensions().x / (float)NUM_SLICE);

		Vec3 normalTR = Vec3::MakeFromPolarDegrees(currentPitch + pitchPerStack, currentYaw + yawPerSlice, radius);
		verts.push_back(Vertex_PNCU(TR, normalTR, tint, Vec2(maxU, maxV)));

		int vertsEndIndex = (int)verts.size() - 1;//21
		for (int sliceIndex = 0; sliceIndex < NUM_SLICE - 1; sliceIndex++)
		{
			//Calculate Verts No.
			int BLIndex, BRIndex = 0;

			BLIndex = 1 + ((layerIndex - 1) * NUM_VERTS_PER_LAYER) + (sliceIndex);//1+(1-1)*5+(0+1)=2
			BRIndex = 1 + ((layerIndex - 1) * NUM_VERTS_PER_LAYER) + (sliceIndex + 1);//1+(1)*5+(0+1)=8

			indexes.push_back(BLIndex + vertStartIndex);
			indexes.push_back(BRIndex + vertStartIndex);
			indexes.push_back(vertsEndIndex);


			if (sliceIndex == NUM_SLICE - 1 - 1)
			{
				//First BL is last BR
				int firstBLIndex = 1 + ((layerIndex - 1) * NUM_VERTS_PER_LAYER) + (0);
				int lastBRIndex = firstBLIndex;
				int lastBLIndex = 1 + ((layerIndex - 1) * NUM_VERTS_PER_LAYER) + (sliceIndex + 1);
				indexes.push_back(lastBLIndex + vertStartIndex);
				indexes.push_back(lastBRIndex + vertStartIndex);
				indexes.push_back(vertsEndIndex);
			}
		}
	}
}

void AddVertsForUVSphereZWireframe3D(std::vector<Vertex_PCU>& verts, const Vec3& center, float radius, float numSlices, float numStacks, float lineThickness, const Rgba8& tint /*= Rgba8::WHITE*/)
{
	int NUM_LAYER = (int)numStacks;
	int NUM_SLICE = (int)numSlices;

	float deltaLatitudeDegree = 180.f / NUM_LAYER;
	float deltaLongtitudeDegree = 360.f / NUM_SLICE;

	for (int layerIndex = 0; layerIndex < NUM_LAYER; layerIndex++)
	{
		for (int sliceIndex = 0; sliceIndex < NUM_SLICE; sliceIndex++)
		{
			float currentLatitudeDegree = -90.f + layerIndex * deltaLatitudeDegree;
			float currentLongtitudeDegree = sliceIndex * deltaLongtitudeDegree;
			Vec3 BL = center + Vec3::MakeFromPolarDegrees(currentLatitudeDegree, currentLongtitudeDegree, radius);
			Vec3 BR = center + Vec3::MakeFromPolarDegrees(currentLatitudeDegree, currentLongtitudeDegree + deltaLongtitudeDegree, radius);
			Vec3 TL = center + Vec3::MakeFromPolarDegrees(currentLatitudeDegree + deltaLatitudeDegree, currentLongtitudeDegree, radius);
			Vec3 TR = center + Vec3::MakeFromPolarDegrees(currentLatitudeDegree + deltaLatitudeDegree,
				currentLongtitudeDegree + deltaLongtitudeDegree, radius);
			AddVertsForLineSegment3D(verts, TL, TR, lineThickness, tint);
			AddVertsForLineSegment3D(verts, TL, BL, lineThickness, tint);
		}
	}
}

void AddVertsForOBB2D(std::vector<Vertex_PCU>& verts, OBB2 const& box, Rgba8 const& color)
{
	constexpr int NUM_TRIS = 2;
	constexpr int NUM_VERTS = 3 * NUM_TRIS;
	verts.reserve(verts.size() + NUM_VERTS);
	Vec2 jBaseNormal = Vec2(-box.m_iBasisNormal.y, box.m_iBasisNormal.x);
	Vec2 rightStepInBox = box.m_iBasisNormal * box.m_halfDimensions.x;
	Vec2 upStepInBox = jBaseNormal * box.m_halfDimensions.y;
	//Corner point
	Vec2 TR = box.m_center + rightStepInBox + upStepInBox;
	Vec2 TL = box.m_center - rightStepInBox + upStepInBox;
	Vec2 BL = box.m_center - rightStepInBox - upStepInBox;
	Vec2 BR = box.m_center + rightStepInBox - upStepInBox;
	//convert to vector 3
	Vec3 TRVec3 = Vec3(TR.x, TR.y, 0.f);
	Vec3 TLVec3 = Vec3(TL.x, TL.y, 0.f);
	Vec3 BLVec3 = Vec3(BL.x, BL.y, 0.f);
	Vec3 BRVec3 = Vec3(BR.x, BR.y, 0.f);

	verts.push_back(Vertex_PCU(BLVec3, color, Vec2(0.f, 0.f)));
	verts.push_back(Vertex_PCU(BRVec3, color, Vec2(1.f, 0.f)));
	verts.push_back(Vertex_PCU(TRVec3, color, Vec2(1.f, 1.f)));

	verts.push_back(Vertex_PCU(TRVec3, color, Vec2(0.f, 0.f)));
	verts.push_back(Vertex_PCU(TLVec3, color, Vec2(1.f, 1.f)));
	verts.push_back(Vertex_PCU(BLVec3, color, Vec2(0.f, 1.f)));
}

void AddVertsForLineSegment2D(std::vector<Vertex_PCU>& verts, Vec2 const& start, Vec2 const& end, float const& thickness, Rgba8 const& color)
{
	float halfThickness = 0.5f * thickness;
	constexpr int NUM_TRIS = 6;
	constexpr int NUM_VERTS = 3 * NUM_TRIS;
	verts.reserve(verts.size() + NUM_VERTS);

	//Line between start and End
	Vec2 startToEnd = end - start;
	Vec2 startToEndNormalize = startToEnd.GetNormalized();
	Vec2 SLNormalized = startToEndNormalize.GetRotated90Degrees();
	Vec2 SLDisplacement = SLNormalized * halfThickness;
	
	Vec2 SL = start + SLDisplacement;
	Vec2 SR = start - SLDisplacement;
	Vec2 EL = end + SLDisplacement;
	Vec2 ER = end - SLDisplacement;

	Vec3 SLVec3 = Vec3(SL.x, SL.y, 0.f);
	Vec3 SRVec3 = Vec3(SR.x, SR.y, 0.f);
	Vec3 ELVec3 = Vec3(EL.x, EL.y, 0.f);
	Vec3 ERVec3 = Vec3(ER.x, ER.y, 0.f);

	verts.push_back(Vertex_PCU(SRVec3, color, Vec2(0.f, 0.f)));
	verts.push_back(Vertex_PCU(ERVec3, color, Vec2(1.f, 0.f)));
	verts.push_back(Vertex_PCU(ELVec3, color, Vec2(1.f, 1.f)));

	verts.push_back(Vertex_PCU(ELVec3, color, Vec2(0.f, 0.f)));
	verts.push_back(Vertex_PCU(SLVec3, color, Vec2(1.f, 1.f)));
	verts.push_back(Vertex_PCU(SRVec3, color, Vec2(0.f, 1.f)));

}

void AddVertsForLineSegment2D(std::vector<Vertex_PCU>& verts, LineSegment2 const& lineSegment, float thickness, Rgba8 const& color)
{
	float halfThickness = 0.5f * thickness;
	constexpr int NUM_TRIS = 6;
	constexpr int NUM_VERTS = 3 * NUM_TRIS;
	verts.reserve(verts.size() + NUM_VERTS);
	 
	//Line between start and End
	Vec2 startToEnd = lineSegment.m_end - lineSegment.m_start;
	Vec2 startToEndNormalize = startToEnd.GetNormalized();
	Vec2 SLNormalized = startToEndNormalize.GetRotated90Degrees();
	Vec2 SLDisplacement = SLNormalized * halfThickness;

	Vec2 SL = lineSegment.m_start + SLDisplacement;
	Vec2 SR = lineSegment.m_start - SLDisplacement;
	Vec2 EL = lineSegment.m_end + SLDisplacement;
	Vec2 ER = lineSegment.m_end - SLDisplacement;

	Vec3 SLVec3 = Vec3(SL.x, SL.y, 0.f);
	Vec3 SRVec3 = Vec3(SR.x, SR.y, 0.f);
	Vec3 ELVec3 = Vec3(EL.x, EL.y, 0.f);
	Vec3 ERVec3 = Vec3(ER.x, ER.y, 0.f);

	verts.push_back(Vertex_PCU(SRVec3, color, Vec2(0.f, 0.f)));
	verts.push_back(Vertex_PCU(ERVec3, color, Vec2(1.f, 0.f)));
	verts.push_back(Vertex_PCU(ELVec3, color, Vec2(1.f, 1.f)));

	verts.push_back(Vertex_PCU(ELVec3, color, Vec2(0.f, 0.f)));
	verts.push_back(Vertex_PCU(SLVec3, color, Vec2(1.f, 1.f)));
	verts.push_back(Vertex_PCU(SRVec3, color, Vec2(0.f, 1.f)));
}

void AddVertsForLineSegment2D(std::vector<Vertex_PCU>& verts, Vec3 const& start, Vec3 const& end, float const& thickness, Rgba8 const& color)
{
	float halfThickness = 0.5f * thickness;
	constexpr int NUM_TRIS = 6;
	constexpr int NUM_VERTS = 3 * NUM_TRIS;
	verts.reserve(verts.size() + NUM_VERTS);

	//Line between start and End
	Vec3 startToEnd = end - start;
	Vec3 startToEndNormalize = startToEnd.GetNormalized();
	Vec3 SLNormalized = startToEndNormalize.GetRotatedAboutZDegrees(90.f);
	Vec3 SLDisplacement = SLNormalized * halfThickness;

	Vec3 SL = start + SLDisplacement;
	Vec3 SR = start - SLDisplacement;
	Vec3 EL = end + SLDisplacement;
	Vec3 ER = end - SLDisplacement;


	verts.push_back(Vertex_PCU(SR, color, Vec2(0.f, 0.f)));
	verts.push_back(Vertex_PCU(ER, color, Vec2(1.f, 0.f)));
	verts.push_back(Vertex_PCU(EL, color, Vec2(1.f, 1.f)));

	verts.push_back(Vertex_PCU(EL, color, Vec2(0.f, 0.f)));
	verts.push_back(Vertex_PCU(SL, color, Vec2(1.f, 1.f)));
	verts.push_back(Vertex_PCU(SR, color, Vec2(0.f, 1.f)));
}


void AddVertsForLineSegment2D(std::vector<Vertex_PCUTBN>& verts, std::vector<int>& indexes, Vec2 const& start, Vec2 const& end, float const& thickness, Rgba8 const& color)
{
	float halfThickness = 0.5f * thickness;
	constexpr int NUM_TRIS = 6;
	constexpr int NUM_INDEXES = 3 * NUM_TRIS;
	verts.reserve(verts.size() + 4);
	indexes.reserve(indexes.size() + NUM_INDEXES);
	//Line between start and End

	Vec2 startToEnd = end - start;
	Vec2 startToEndNormalize = startToEnd.GetNormalized();
	Vec2 SLNormalized = startToEndNormalize.GetRotated90Degrees();
	Vec2 SLDisplacement = SLNormalized * halfThickness;
	Vec2 SEDisplacement = startToEndNormalize * halfThickness;

	Vec2 SL = start + SLDisplacement - SEDisplacement;
	Vec2 SR = start - SLDisplacement - SEDisplacement;
	Vec2 EL = end + SLDisplacement + SEDisplacement;
	Vec2 ER = end - SLDisplacement + SEDisplacement;

	Vec3 SLVec3 = Vec3(SL.x, SL.y, 0.f);
	Vec3 SRVec3 = Vec3(SR.x, SR.y, 0.f);
	Vec3 ELVec3 = Vec3(EL.x, EL.y, 0.f);
	Vec3 ERVec3 = Vec3(ER.x, ER.y, 0.f);

	Vec3 normal = Vec3::Z_AXIS;
	int startIndex = (int)verts.size();

	verts.push_back(Vertex_PCUTBN(Vertex_PNCU(SRVec3, normal, color)));
	verts.push_back(Vertex_PCUTBN(Vertex_PNCU(ELVec3, normal, color)));
	verts.push_back(Vertex_PCUTBN(Vertex_PNCU(ERVec3, normal, color)));
	verts.push_back(Vertex_PCUTBN(Vertex_PNCU(SLVec3, normal, color)));


	indexes.push_back(startIndex);
	indexes.push_back(startIndex + 2);
	indexes.push_back(startIndex + 1);

	indexes.push_back(startIndex + 1);
	indexes.push_back(startIndex + 3);
	indexes.push_back(startIndex);
}

void AddVertsForLineSegment2D(std::vector<Vertex_PCUTBN>& verts, Vec2 const& start, Vec2 const& end, float const& thickness, Rgba8 const& color, float dragDownOffset /*= 0.f*/)
{
	float halfThickness = 0.5f * thickness;
	constexpr int NUM_TRIS = 6;
	constexpr int NUM_VERTS = 3 * NUM_TRIS;
	verts.reserve(verts.size() + NUM_VERTS);

	//Line between start and End

	Vec2 startToEnd = end - start;
	Vec2 startToEndNormalize = startToEnd.GetNormalized();
	Vec2 SLNormalized = startToEndNormalize.GetRotated90Degrees();
	Vec2 SLDisplacement = SLNormalized * halfThickness;
	Vec2 SEDisplacement = startToEndNormalize * halfThickness;

	Vec2 SL = start + SLDisplacement - SEDisplacement;
	Vec2 SR = start - SLDisplacement - SEDisplacement;
	Vec2 EL = end + SLDisplacement + SEDisplacement;
	Vec2 ER = end - SLDisplacement + SEDisplacement;

	Vec3 SLVec3 = Vec3(SL.x, SL.y, 0.f - dragDownOffset);
	Vec3 SRVec3 = Vec3(SR.x, SR.y, 0.f - dragDownOffset);
	Vec3 ELVec3 = Vec3(EL.x, EL.y, 0.f - dragDownOffset);
	Vec3 ERVec3 = Vec3(ER.x, ER.y, 0.f - dragDownOffset);

	Vec3 normal = Vec3::Z_AXIS;


	verts.push_back(Vertex_PCUTBN(Vertex_PNCU(SRVec3, normal, color)));
	verts.push_back(Vertex_PCUTBN(Vertex_PNCU(ERVec3, normal, color)));
	verts.push_back(Vertex_PCUTBN(Vertex_PNCU(ELVec3, normal, color)));

	verts.push_back(Vertex_PCUTBN(Vertex_PNCU(ELVec3, normal, color)));
	verts.push_back(Vertex_PCUTBN(Vertex_PNCU(SLVec3, normal, color)));
	verts.push_back(Vertex_PCUTBN(Vertex_PNCU(SRVec3, normal, color)));
}

void AddVertsForLineSegmentWithOffset2D(std::vector<Vertex_PCU>& verts, Vec2 const& start, Vec2 const& end, float const& thickness, Rgba8 const& color)
{
	float halfThickness = 0.5f * thickness;
	constexpr int NUM_TRIS = 6;
	constexpr int NUM_VERTS = 3 * NUM_TRIS;
	verts.reserve(verts.size() + NUM_VERTS);

	//Line between start and End
	Vec2 startToEnd = end - start;
	Vec2 startToEndNormalize = startToEnd.GetNormalized();
	Vec2 SLNormalized = startToEndNormalize.GetRotated90Degrees();
	Vec2 SLDisplacement = SLNormalized * halfThickness;
	Vec2 SEDisplacement = startToEndNormalize * halfThickness;

	Vec2 SL = start + SLDisplacement - SEDisplacement;
	Vec2 SR = start - SLDisplacement - SEDisplacement;
	Vec2 EL = end + SLDisplacement + SEDisplacement;
	Vec2 ER = end - SLDisplacement + SEDisplacement;

	Vec3 SLVec3 = Vec3(SL.x, SL.y, 0.f);
	Vec3 SRVec3 = Vec3(SR.x, SR.y, 0.f);
	Vec3 ELVec3 = Vec3(EL.x, EL.y, 0.f);
	Vec3 ERVec3 = Vec3(ER.x, ER.y, 0.f);

	verts.push_back(Vertex_PCU(SRVec3, color, Vec2(0.f, 0.f)));
	verts.push_back(Vertex_PCU(ERVec3, color, Vec2(1.f, 0.f)));
	verts.push_back(Vertex_PCU(ELVec3, color, Vec2(1.f, 1.f)));

	verts.push_back(Vertex_PCU(ELVec3, color, Vec2(0.f, 0.f)));
	verts.push_back(Vertex_PCU(SLVec3, color, Vec2(1.f, 1.f)));
	verts.push_back(Vertex_PCU(SRVec3, color, Vec2(0.f, 1.f)));
}

void AddVertsForLineSegment3D(std::vector<Vertex_PCU>& verts, Vec3 const& start, Vec3 const& end, float const& thickness, Rgba8 const& color)
{
	float halfThickness = 0.5f * thickness;

	Vec3 iForward = (end - start).GetNormalized();
	Vec3 jLeft = CrossProduct3D(Vec3::Z_AXIS, iForward);
	if (jLeft == Vec3::WORLD_ORIGIN) 
	{
		jLeft = Vec3::Y_AXIS;
	}
	jLeft.Normalize();
	Vec3 kUp = CrossProduct3D(iForward, jLeft).GetNormalized();

	Vec3 iStep = iForward * halfThickness;
	Vec3 jStep = jLeft * halfThickness;
	Vec3 kStep = kUp * halfThickness;

// 	Vec3 SBL = start - iStep + jStep - kStep;
// 	Vec3 SBR = start - iStep - jStep - kStep;
// 	Vec3 STR = start - iStep - jStep + kStep;
// 	Vec3 STL = start - iStep + jStep + kStep;
// 
// 	Vec3 EBL = end + iStep - jStep - kStep;
// 	Vec3 EBR = end + iStep + jStep - kStep;
// 	Vec3 ETR = end + iStep + jStep + kStep;
// 	Vec3 ETL = end + iStep - jStep + kStep;

	Vec3 SBL = start + jStep - kStep;
	Vec3 SBR = start - jStep - kStep;
	Vec3 STR = start - jStep + kStep;
	Vec3 STL = start + jStep + kStep;

	Vec3 EBL = end - jStep - kStep;
	Vec3 EBR = end + jStep - kStep;
	Vec3 ETR = end + jStep + kStep;
	Vec3 ETL = end - jStep + kStep;
	AddVertsForQuad3D(verts, SBL, SBR, STR, STL, color);//right
	AddVertsForQuad3D(verts, EBL, EBR, ETR, ETL, color);//left

	AddVertsForQuad3D(verts, SBR, EBL, ETL, STR, color);//front
	AddVertsForQuad3D(verts, EBR, SBL, STL, ETR, color);//back

	AddVertsForQuad3D(verts, STR, ETL, ETR, STL, color);//up
	AddVertsForQuad3D(verts, EBR, SBL, SBR, EBL, color);//down
}

void AddVertsForLineSegment3D(std::vector<Vertex_PCUTBN>& verts, Vec3 const& start, Vec3 const& end, float const& thickness, Rgba8 const& color)
{
	float halfThickness = 0.5f * thickness;

	Vec3 iForward = (end - start).GetNormalized();
	Vec3 jLeft = CrossProduct3D(Vec3::Z_AXIS, iForward);
	if (jLeft == Vec3::WORLD_ORIGIN)
	{
		jLeft = Vec3::Y_AXIS;
	}
	jLeft.Normalize();
	Vec3 kUp = CrossProduct3D(iForward, jLeft).GetNormalized();

	Vec3 iStep = iForward * halfThickness;
	Vec3 jStep = jLeft * halfThickness;
	Vec3 kStep = kUp * halfThickness;

	Vec3 SBL = start + jStep - kStep;
	Vec3 SBR = start - jStep - kStep;
	Vec3 STR = start - jStep + kStep;
	Vec3 STL = start + jStep + kStep;

	Vec3 EBL = end - jStep - kStep;
	Vec3 EBR = end + jStep - kStep;
	Vec3 ETR = end + jStep + kStep;
	Vec3 ETL = end - jStep + kStep;
	AddVertsForQuad3D(verts, SBL, SBR, STR, STL, color);//right
	AddVertsForQuad3D(verts, EBL, EBR, ETR, ETL, color);//left

	AddVertsForQuad3D(verts, SBR, EBL, ETL, STR, color);//front
	AddVertsForQuad3D(verts, EBR, SBL, STL, ETR, color);//back

	AddVertsForQuad3D(verts, STR, ETL, ETR, STL, color);//up
	AddVertsForQuad3D(verts, EBR, SBL, SBR, EBL, color);//down
}

void AddVertsForInfiniteLine2D(std::vector<Vertex_PCU>& verts, Vec2 const& start, Vec2 const& end, float const& thickness, Rgba8 const& color)
{
	float infiniteLength = 600.f;
	Vec2 hugeForward = end - start;
	hugeForward.SetLength(infiniteLength);
	Vec2 drawEnd = end + hugeForward;
	Vec2 drawStart = start - hugeForward;
	AddVertsForLineSegment2D(verts, drawStart, drawEnd, thickness, color);
}

void AddVertsForInfiniteLine2D(std::vector<Vertex_PCU>& verts, LineSegment2 const& lineSegment, float thickness, Rgba8 const& color)
{
	float infiniteLength = 600.f;
	Vec2 hugeForward = lineSegment.m_end - lineSegment.m_start;
	hugeForward.SetLength(infiniteLength);
	Vec2 drawEnd = lineSegment.m_end + hugeForward;
	Vec2 drawStart = lineSegment.m_start - hugeForward;
	AddVertsForLineSegment2D(verts, drawEnd, drawStart, thickness, color);
}


void AddVertsForHollowHexagon(std::vector<Vertex_PCUTBN>& verts, Hexagon2D const& hexag, float thickness, Rgba8 const& color /*= Rgba8::WHITE*/)
{
	Vec2 center = hexag.GetCenter();
	float halfThickness = 0.5f * thickness;
	constexpr int NUM_STRIP = 6;
	constexpr int NUM_TRIS = 2 * NUM_STRIP;
	constexpr int NUM_INDEXES = 3 * NUM_TRIS;

	verts.reserve(verts.size() + NUM_INDEXES);

	for (int i = 0; i < 6; ++i)
	{
		int index = i % 6;
		int nextIndex = (i + 1) % 6;
		Vec2 start = Vec2(hexag.m_pointArray[index].x, hexag.m_pointArray[index].y);
		Vec2 end = Vec2(hexag.m_pointArray[nextIndex].x, hexag.m_pointArray[nextIndex].y);

		Vec2 dispCSNormalized = Vec2(start - center).GetNormalized();
		Vec2 dispCSBack = (-dispCSNormalized) * halfThickness;
		Vec2 dispCSForward = -dispCSBack;

		Vec2 dispCENormalized = Vec2(end - center).GetNormalized();
		Vec2 dispCEBack = (-dispCENormalized) * halfThickness;
		Vec2 dispCEForward = -dispCEBack;


		Vec2 SL = start + dispCSBack;
		Vec2 SR = start + dispCSForward;

		Vec2 EL = end + dispCEBack;
		Vec2 ER = end + dispCEForward;

		Vec3 SL3D(SL, 0.f);
		Vec3 SR3D(SR, 0.f);
		Vec3 EL3D(EL, 0.f);
		Vec3 ER3D(ER, 0.f);


		Vec3 normal = Vec3::Z_AXIS;
		//verts 
		verts.push_back(Vertex_PCUTBN(Vertex_PNCU(SL3D,normal, color)));
		verts.push_back(Vertex_PCUTBN(Vertex_PNCU(SR3D,normal, color)));
		verts.push_back(Vertex_PCUTBN(Vertex_PNCU(ER3D,normal, color)));

		verts.push_back(Vertex_PCUTBN(Vertex_PNCU(ER3D,normal, color)));
		verts.push_back(Vertex_PCUTBN(Vertex_PNCU(EL3D,normal, color)));
		verts.push_back(Vertex_PCUTBN(Vertex_PNCU(SL3D,normal, color)));

	}
}

void AddVertsForHollowHexagon(std::vector<Vertex_PCUTBN>& verts, std::vector<int>& indexes, Hexagon2D const& hexag, float thickness, Rgba8 const& color /*= Rgba8::WHITE*/)
{
	Vec2 center = hexag.GetCenter();
	float halfThickness = 0.5f * thickness;
	constexpr int NUM_STRIP = 6;
	constexpr int NUM_VERTEXES = 4 * NUM_STRIP;
	constexpr int NUM_TRIS = 2 * NUM_STRIP;
	constexpr int NUM_INDEXES = 3 * NUM_TRIS;

	verts.reserve(verts.size() + NUM_VERTEXES);
	indexes.reserve(indexes.size() + NUM_INDEXES);

	Vec3 normal = Vec3::Z_AXIS;

	for (int i = 0; i < 6; ++i)
	{
		int index = i % 6;
		int nextIndex = (i + 1) % 6;
		Vec2 start = Vec2(hexag.m_pointArray[index].x, hexag.m_pointArray[index].y);
		Vec2 end = Vec2(hexag.m_pointArray[nextIndex].x, hexag.m_pointArray[nextIndex].y);

		Vec2 dispCSNormalized = Vec2(start - center).GetNormalized();
		Vec2 dispCSBack = (-dispCSNormalized)* halfThickness;
		Vec2 dispCSForward = -dispCSBack;

		Vec2 dispCENormalized = Vec2(end - center).GetNormalized();
		Vec2 dispCEBack = (-dispCENormalized) * halfThickness;
		Vec2 dispCEForward = -dispCEBack;


		Vec2 SL = start + dispCSBack;
		Vec2 SR = start + dispCSForward;

		Vec2 EL = end + dispCEBack;
		Vec2 ER = end + dispCEForward;

		Vec3 SL3D(SL, 0.f);
		Vec3 SR3D(SR, 0.f);
		Vec3 EL3D(EL, 0.f);
		Vec3 ER3D(ER, 0.f);

		int startIndex = (int)verts.size();

		//verts 
		verts.push_back(Vertex_PCUTBN(Vertex_PNCU(SL3D, normal, color)));
		verts.push_back(Vertex_PCUTBN(Vertex_PNCU(SR3D, normal, color)));
		verts.push_back(Vertex_PCUTBN(Vertex_PNCU(EL3D, normal, color)));
		verts.push_back(Vertex_PCUTBN(Vertex_PNCU(ER3D, normal, color)));

		//index

		indexes.push_back(startIndex);
		indexes.push_back(startIndex + 1);
		indexes.push_back(startIndex + 3);

		indexes.push_back(startIndex + 3);
		indexes.push_back(startIndex + 2);
		indexes.push_back(startIndex);

		//AddVertsForLineSegment2D(verts, indexes, start, end, thickness, color);
	}
}

void AddVertsForHollowHexagon(std::vector<Vertex_PCU>& verts, Hexagon2D const& hexag, float thickness, Rgba8 const& color /*= Rgba8::WHITE*/)
{
	Vec2 center = hexag.GetCenter();
	float halfThickness = 0.5f * thickness;
	constexpr int NUM_STRIP = 6;
	constexpr int NUM_TRIS = 2 * NUM_STRIP;
	constexpr int NUM_INDEXES = 3 * NUM_TRIS;

	verts.reserve(verts.size() + NUM_INDEXES);

	for (int i = 0; i < 6; ++i)
	{
		int index = i % 6;
		int nextIndex = (i + 1) % 6;
		Vec2 start = Vec2(hexag.m_pointArray[index].x, hexag.m_pointArray[index].y);
		Vec2 end = Vec2(hexag.m_pointArray[nextIndex].x, hexag.m_pointArray[nextIndex].y);

		Vec2 dispCSNormalized = Vec2(start - center).GetNormalized();
		Vec2 dispCSBack = (-dispCSNormalized) * halfThickness;
		Vec2 dispCSForward = -dispCSBack;

		Vec2 dispCENormalized = Vec2(end - center).GetNormalized();
		Vec2 dispCEBack = (-dispCENormalized) * halfThickness;
		Vec2 dispCEForward = -dispCEBack;


		Vec2 SL = start + dispCSBack;
		Vec2 SR = start + dispCSForward;

		Vec2 EL = end + dispCEBack;
		Vec2 ER = end + dispCEForward;

		Vec3 SL3D(SL, 0.f);
		Vec3 SR3D(SR, 0.f);
		Vec3 EL3D(EL, 0.f);
		Vec3 ER3D(ER, 0.f);

		//verts 
		verts.push_back(Vertex_PCU(SL3D, color));
		verts.push_back(Vertex_PCU(SR3D, color));
		verts.push_back(Vertex_PCU(ER3D, color));

		verts.push_back(Vertex_PCU(ER3D, color));
		verts.push_back(Vertex_PCU(EL3D, color));
		verts.push_back(Vertex_PCU(SL3D, color));

	}
}

void AddVertsForFullHexagon(std::vector<Vertex_PCUTBN>& verts, Hexagon2D const& hexag, Rgba8 const& color /*= Rgba8::WHITE*/)
{
	Vec2 centerXY = hexag.GetCenter();
	Vec3 center = Vec3(centerXY.x, centerXY.y, 0.f);
	for (int i = 0; i < 6; ++i)
	{
		int index = i % 6;
		int nextIndex = (i + 1) % 6;
		Vec3 start3D = Vec3(hexag.m_pointArray[index].x, hexag.m_pointArray[index].y, 0.f);
		Vec3 end3D = Vec3(hexag.m_pointArray[nextIndex].x, hexag.m_pointArray[nextIndex].y, 0.f);
		AddVertsForTriangle3D(verts, start3D, end3D, center, color, true);
	}
}

void AddVertsForFullHexagon(std::vector<Vertex_PCUTBN>& verts, std::vector<int>& indexes, Hexagon2D const& hexag, Rgba8 const& color /*= Rgba8::WHITE*/)
{
	Vec2 centerXY = hexag.GetCenter();
	Vec3 center = Vec3(centerXY.x, centerXY.y, 0.f);

	constexpr int NUM_TRIS = 6;
	constexpr int NUM_INDEXES = 3 * NUM_TRIS;
	verts.reserve(verts.size() + 7);
	indexes.reserve(indexes.size() + NUM_INDEXES);
	
	Vec3 normal = Vec3(0.f, 0.f, 1.f);
	int startIndex = (int)verts.size();
	//Vertex 

	verts.push_back(Vertex_PCUTBN(Vertex_PNCU(hexag.m_pointArray3D[0], normal, color)));
	verts.push_back(Vertex_PCUTBN(Vertex_PNCU(hexag.m_pointArray3D[1], normal, color)));
	verts.push_back(Vertex_PCUTBN(Vertex_PNCU(hexag.m_pointArray3D[2], normal, color)));
	verts.push_back(Vertex_PCUTBN(Vertex_PNCU(hexag.m_pointArray3D[3], normal, color)));
	verts.push_back(Vertex_PCUTBN(Vertex_PNCU(hexag.m_pointArray3D[4], normal, color)));
	verts.push_back(Vertex_PCUTBN(Vertex_PNCU(hexag.m_pointArray3D[5], normal, color)));
	verts.push_back(Vertex_PCUTBN(Vertex_PNCU(center, normal, color)));


	int centerIndex = (int)verts.size() - 1;
	//Index
	for (int i = 0; i < 6; ++i)
	{
		int index = i % 6 + startIndex;
		int nextIndex = (i + 1) % 6 + startIndex;
		
		indexes.push_back(index);
		indexes.push_back(nextIndex);
		indexes.push_back(centerIndex);
		//Vec3 start3D = Vec3(hexag.m_pointArray[index].x, hexag.m_pointArray[index].y, 0.f);
		//Vec3 end3D = Vec3(hexag.m_pointArray[nextIndex].x, hexag.m_pointArray[nextIndex].y, 0.f);
	}
}

void AddVertsForArrow2D(std::vector<Vertex_PCU>& verts, Vec2 tailPos, Vec2 tipPos, float arrowSize, float lineThickness, Rgba8 color)
{
	//Triangle
	Vec2 tipToTail = tailPos - tipPos;
	Vec2 iBase = tipToTail.GetNormalized();
	Vec2 jBase = iBase.GetRotated90Degrees();

	Vec2 tipBR = tipPos + arrowSize * iBase + arrowSize * jBase;
	Vec2 tipUR = tipPos + arrowSize * iBase - arrowSize * jBase;

	constexpr int NUM_VERTS = 3;
	verts.reserve(verts.size() + NUM_VERTS);

	Vec3 tipDown = Vec3(tipBR.x, tipBR.y, 0.f);
	Vec3 tiplLeft = Vec3(tipUR.x, tipUR.y, 0.f);
	Vec3 tipSelf = Vec3(tipPos.x, tipPos.y, 0.f);
	
	TransformPositionXY3D(tipDown, 1.f, 0.f, Vec2(0.f, 0.f));
	TransformPositionXY3D(tiplLeft, 1.f, 0.f, Vec2(0.f, 0.f));
	TransformPositionXY3D(tipSelf, 1.f, 0.f, Vec2(0.f, 0.f));

	verts.push_back(Vertex_PCU(tipDown, color, Vec2(0.f, 0.f)));
	verts.push_back(Vertex_PCU(tipSelf, color, Vec2(1.f, 1.f)));
	verts.push_back(Vertex_PCU(tiplLeft, color, Vec2(1.f, 0.f)));

	//Tail

	Vec2 tailStart = 0.5f * Vec2(tiplLeft.x + tipDown.x, tiplLeft.y + tipDown.y);
	Vec2 tailEnd = tailPos;

	AddVertsForLineSegment2D(verts, tailStart, tailEnd, lineThickness, color);
}

void AddVertsForSlimArrow2D(std::vector<Vertex_PCU>& verts, Vec2 const& start, Vec2 const& end, float thickness, float arrowRadius, Rgba8 color)
{
	Vec2 tailPos = start;
	Vec2 tipPos = end;
	AddVertsForLineSegment2D(verts, tailPos, tipPos, thickness, color);

	Vec2 tipToTail = tailPos - tipPos;
	Vec2 tipToTailNormal = tipToTail.GetNormalized();
	Vec2 vertDown = tipPos + arrowRadius * tipToTailNormal.GetRotatedDegrees(40.f);
	Vec2 vertUp = tipPos + arrowRadius * tipToTailNormal.GetRotatedDegrees(-40.f);

	AddVertsForLineSegment2D(verts, tipPos, vertUp, thickness, color);
	AddVertsForLineSegment2D(verts, tipPos, vertDown, thickness, color);
}


