#pragma once
#include"Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/Vertex_PNCU.hpp"
#include "Engine/Core/Vertex_PCUTBN.hpp"
#include"Engine/Math/AABB2.hpp"
#include "Engine/Math/Capsule2.hpp"
#include "Engine/Math/OBB2.hpp"
#include "Engine/Math/LineSegment2.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Math/Hexagon.hpp"
#include "Engine/Math/ConvexPoly2D.hpp"
#include "Engine/Math/ConvexPoly3D.hpp"
#include <vector>


void TransformVertexArrayXY3D(int numVerts, Vertex_PCU* verts, float scaleXY, float rotationDegreesAboutZ, Vec2 const& translationXY);
void TransformVertexArray3D(int numVerts, Vertex_PCU* verts, const Mat44& transform);
void TransformVertexArray3D(std::vector<Vertex_PCU>& verts, const Mat44& transform);
void TransformVertexArray3D(std::vector<Vertex_PNCU>& verts, const Mat44& transform);
void TransformVertexArray3D(std::vector<Vertex_PCUTBN>& verts, const Mat44& transform);

void FillTangentSpaceVectorForMesh(std::vector<Vertex_PCUTBN>& verts, std::vector<unsigned int>const indexes);

void AddVertsForConvexPoly2D(std::vector<Vertex_PCU>& verts, ConvexPoly2D const& convexPoly, Rgba8 const& color = Rgba8::WHITE);
void AddVertsForConvexPoly3D(std::vector<Vertex_PCU>& verts, ConvexPoly3D const& convexPoly, Rgba8 const& color = Rgba8::WHITE);

AABB2 GetVertexBounds2D(const std::vector<Vertex_PCU>& verts);
void AddVertsForCylinder3D(std::vector<Vertex_PCU>& verts, Vec3 const& start, Vec3 const & end, float radius, Rgba8 const& color = Rgba8::WHITE, AABB2 const& UVs = AABB2::ZERO_TO_ONE, int numSlices = 8);
void AddVertsForCone3D(std::vector<Vertex_PCU>& verts, Vec3 const& start, const Vec3& end, float radius, Rgba8 const& color = Rgba8::WHITE, AABB2 const& UVs = AABB2::ZERO_TO_ONE, int numSlices = 8);
void AddVertsForWorldArrow3D(std::vector<Vertex_PCU>& verts, Vec3 const& start, const Vec3& end, float radius, Rgba8 const& color = Rgba8::WHITE, AABB2 const& UVs = AABB2::ZERO_TO_ONE, int numSlices = 8);

void AddVertsForDisc2D(std::vector<Vertex_PCU>& verts, Vec2 const& center, float const& radius, Rgba8 const& color);
void AddVertsForRing2D(std::vector<Vertex_PCU>& verts, Vec2 const& center, float radius, float thickness, Rgba8 const& color);

void AddVertsForLineSegment2D(std::vector<Vertex_PCU>& verts, LineSegment2 const& lineSegment,float thickness, Rgba8 const& color);
void AddVertsForLineSegment2D(std::vector<Vertex_PCU>& verts, Vec2 const& start, Vec2 const& end, float const& thickness, Rgba8 const& color);
void AddVertsForLineSegment2D(std::vector<Vertex_PCUTBN>& verts, Vec2 const& start, Vec2 const& end, float const& thickness, Rgba8 const& color, float dragDownOffset = 0.f);
void AddVertsForLineSegment2D(std::vector<Vertex_PCUTBN>& verts, std::vector<int>& indexes, Vec2 const& start, Vec2 const& end, float const& thickness, Rgba8 const& color);

void AddVertsForLineSegmentWithOffset2D(std::vector<Vertex_PCU>& verts, Vec2 const& start, Vec2 const& end, float const& thickness, Rgba8 const& color);

void AddVertsForLineSegment3D(std::vector<Vertex_PCU>& verts, Vec3 const& start, Vec3 const& end, float const& thickness, Rgba8 const& color);

void AddVertsForArrow2D(std::vector<Vertex_PCU>& verts, Vec2 tailPos, Vec2 tipPos, float arrowSize, float lineThickness, Rgba8 color);
void AddVertsForSlimArrow2D(std::vector<Vertex_PCU>& verts, Vec2 const& start, Vec2 const& end, float thickness, float arrowRadius, Rgba8 color);
void AddVertsForAABB2D(std::vector<Vertex_PCU>& verts, AABB2 const& spriteBounds, Rgba8 const& spriteTint, Vec2 const& uvAtMins, Vec2 const& uvAtMaxs);
void AddVertsForAABB2D(std::vector<Vertex_PCU>& verts, AABB2 const& bounds, Rgba8 const& color);
void AddVertsForAABB3D(std::vector<Vertex_PCU>& verts, const AABB3& bounds, const Rgba8& color = Rgba8::WHITE, const AABB2& UVs = AABB2::ZERO_TO_ONE);
void AddVertsForQuad3D(std::vector<Vertex_PCU>& verts, const Vec3& bottomLeft, const Vec3& bottomRight, const Vec3& topRight, const Vec3& topLeft, const Rgba8& color, const AABB2& UVs = AABB2::ZERO_TO_ONE);
void AddVertsForQuad3D(std::vector<Vertex_PCU>& verts, std::vector<unsigned int>& indexes, const Vec3& bottomLeft, const Vec3& bottomRight, const Vec3& topRight, const Vec3& topLeft, const Rgba8& color, const AABB2& UVs = AABB2::ZERO_TO_ONE);
void AddVertsForRoundedAABB3D(std::vector<Vertex_PCUTBN>& verts, const AABB3& bounds, const Rgba8& color = Rgba8::WHITE, const AABB2& UVs = AABB2::ZERO_TO_ONE);

// void AddVertsForRoundedQuad3D(std::vector<Vertex_PNCU>& verts, const Vec3& bottomLeft, const Vec3& bottomRight, const Vec3& topRight, const Vec3& topLeft, const Rgba8& color = Rgba8::WHITE, const AABB2& UVs = AABB2::ZERO_TO_ONE);
// void AddVertsForRoundedAABB3D(std::vector<Vertex_PNCU>& verts, const AABB3& bounds, const Rgba8& color = Rgba8::WHITE, const AABB2& UVs = AABB2::ZERO_TO_ONE);


void AddVertsForRoundedQuad3D(std::vector<Vertex_PCUTBN>& verts, const Vec3& bottomLeft, const Vec3& bottomRight, const Vec3& topRight, const Vec3& topLeft, const Rgba8& color = Rgba8::WHITE, const AABB2& UVs = AABB2::ZERO_TO_ONE);
void AddVertsForQuad3D(std::vector<Vertex_PCUTBN>& verts, std::vector<unsigned int>& indexes, const Vec3& bottomLeft, const Vec3& bottomRight, const Vec3& topRight, const Vec3& topLeft, const Rgba8& color = Rgba8::WHITE, const AABB2& UVs = AABB2::ZERO_TO_ONE);
void AddVertsForQuad3D(std::vector<Vertex_PCUTBN>& verts, const Vec3& bottomLeft, const Vec3& bottomRight, const Vec3& topRight, const Vec3& topLeft, const Rgba8& color, const AABB2& UVs = AABB2::ZERO_TO_ONE);
void AddVertsForQuadNormalOnly3D(std::vector<Vertex_PNCU>& verts, std::vector<unsigned int>& indexes, const Vec3& bottomLeft, const Vec3& bottomRight, const Vec3& topRight, const Vec3& topLeft, const Rgba8& color = Rgba8::WHITE, const AABB2& UVs = AABB2::ZERO_TO_ONE);
void AddVertsForQuadNormalOnly3D(std::vector<Vertex_PNCU>& verts, const Vec3& bottomLeft, const Vec3& bottomRight, const Vec3& topRight, const Vec3& topLeft, const Rgba8& color, const AABB2& UVs = AABB2::ZERO_TO_ONE);
void AddVertsForQuadNormalOnly3DPCUTBN(std::vector<Vertex_PCUTBN>& verts, const Vec3& bottomLeft, const Vec3& bottomRight, const Vec3& topRight, const Vec3& topLeft, const Rgba8& color, const AABB2& UVs = AABB2::ZERO_TO_ONE);


void AddVertsForTriangle3D(std::vector<Vertex_PCUTBN>& verts, const Vec3& pointA, const Vec3& pointB, const Vec3& pointC, const Rgba8& color, bool isInXYPlane = false);
void AddVertsForCube3D(std::vector<Vertex_PCUTBN>& verts, std::vector<unsigned int>& indexes, const AABB3& bounds, const Rgba8& color = Rgba8::WHITE, const AABB2& UVs = AABB2::ZERO_TO_ONE);


void AddVertsForLineWithLinePrimitive(std::vector<Vertex_PCU>& verts, Vec3 const& start, Vec3 const& end, const Rgba8& color = Rgba8::WHITE);

void AddVertsForAABBWireframe3D(std::vector<Vertex_PCU>& verts, AABB3 const& bounds, float lineThickness, Rgba8 const& tint = Rgba8::WHITE);
void AddVertsForCylinderZ3D(std::vector<Vertex_PCU>& verts, Vec2 const& centerXY, FloatRange const& minMaxZ, float radius, float numSlices, Rgba8 const& tint = Rgba8::WHITE, AABB2 const& UVs = AABB2::ZERO_TO_ONE);
void AddVertsForCylinderZWireframe3D(std::vector<Vertex_PCU>& verts, Vec2 const& centerXY, FloatRange const& minMaxZ, float radius, float numSlices, float lineThickness, Rgba8 const& tint = Rgba8::WHITE);
void AddVertsForSphere3D(std::vector<Vertex_PCU>& verts, const Vec3& center, float radius, const Rgba8& color = Rgba8::WHITE, const AABB2& UVs = AABB2::ZERO_TO_ONE, int numLatitudeSlice = 8);
void AddVertsForUVSphereZ3D(std::vector<Vertex_PCU>& verts, const Vec3& center, float radius, float numSlices, float numStacks, const Rgba8& tint = Rgba8::WHITE, AABB2 const& UVs = AABB2::ZERO_TO_ONE);
void AddVertsForUVSphereZWireframe3D(std::vector<Vertex_PCU>& verts, const Vec3& center, float radius, float numSlices, float numStacks, float lineThickness, const Rgba8& tint = Rgba8::WHITE);
void AddVertsForUVSphereZ3D(std::vector<Vertex_PCUTBN>& verts, std::vector<unsigned int>& indexes, const Vec3& center, float radius, float numSlices, float numStacks, const Rgba8& tint = Rgba8::WHITE, AABB2 const& UVs = AABB2::ZERO_TO_ONE);
void AddVertsForUVSphereZ3DWithPNCU(std::vector<Vertex_PNCU>& verts, std::vector<unsigned int>& indexes, const Vec3& center, float radius, float numSlices, float numStacks, const Rgba8& tint = Rgba8::WHITE, AABB2 const& UVs = AABB2::ZERO_TO_ONE);



void AddVertsForSector2D(std::vector<Vertex_PCU>& verts, Vec2 const& center, float const& radius, float const& startDegrees, float const& endDegrees, Rgba8 const& color);
void AddVertsForCapsule2D(std::vector<Vertex_PCU>& verts, Capsule2 const& capsule, Rgba8 const& color);
void AddVertsForCapsule2D(std::vector<Vertex_PCU>& verts, Vec2 const& boneStart, Vec2 const& boneEnd, float radius, Rgba8 const& color);
void AddVertsForOBB2D(std::vector<Vertex_PCU>& verts, OBB2 const& box, Rgba8 const& color);
void AddVertsForInfiniteLine2D(std::vector<Vertex_PCU>& verts, Vec2 const& start, Vec2 const& end, float const& thickness, Rgba8 const& color);
void AddVertsForInfiniteLine2D(std::vector<Vertex_PCU>& verts, LineSegment2 const& lineSegment,float thickness, Rgba8 const& color);


//void AddVertsForHollowHexagon(std::vector<Vertex_PNCU>& verts, Hexagon2D const& hexag, float thickness, Rgba8 const& color = Rgba8::WHITE);
void AddVertsForHollowHexagon(std::vector<Vertex_PCU>& verts, Hexagon2D const& hexag, float thickness, Rgba8 const& color = Rgba8::WHITE);
void AddVertsForHollowHexagon(std::vector<Vertex_PCUTBN>& verts, Hexagon2D const& hexag, float thickness, Rgba8 const& color = Rgba8::WHITE);
void AddVertsForHollowHexagon(std::vector<Vertex_PCUTBN>& verts, std::vector<int>& indexes, Hexagon2D const& hexag, float thickness, Rgba8 const& color = Rgba8::WHITE);
void AddVertsForFullHexagon(std::vector<Vertex_PCUTBN>& verts, Hexagon2D const& hexag, Rgba8 const& color = Rgba8::WHITE);
void AddVertsForFullHexagon(std::vector<Vertex_PCUTBN>& verts, std::vector<int>& indexes, Hexagon2D const& hexag, Rgba8 const& color = Rgba8::WHITE);