#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/IntVec2.hpp"
constexpr float sqrt3 = 1.73205f;
constexpr float oneOverSqrt3 = 0.57735f;
struct Hexagon2D
{
public:
	Hexagon2D() = default;
	//explicit Hexagon2D(Vec3 center, float circumRadius);
	explicit Hexagon2D(Vec3 center, float inRadius);
	//explicit Hexagon2D(Vec2 center, float circumRadius);
	~Hexagon2D() = default;
public:
	Vec2 GetCenter()const { return m_center; }
	Vec2 m_pointArray[6];
	Vec3 m_pointArray3D[6];
	void SetWalkablity(bool b) { m_isEmpty = b; }
	bool IsWalkable()const { return m_isEmpty; }
	void SetCoords(IntVec2 i) { m_coord = i; }
	IntVec2 GetCoords()const { return m_coord; }
	float GetInRadius()const { return m_inRadius; }
private:
	void FillPointArray2D();
	Vec2 m_center{ 0.f,0.f };
	IntVec2 m_coord{ 0,0 };
	float m_inRadius = 0.f;
	float m_circumRadius = 0.f;
	float m_sideLength = 0.f;
	bool m_isEmpty = true;
};