#pragma once
#include "Engine/Math/Vec3.hpp"



struct AABB3
{
public:
	Vec3 m_mins = Vec3::WORLD_ORIGIN;
	Vec3 m_maxs = Vec3::WORLD_ORIGIN;

public:
	~AABB3();
	AABB3(){}
	AABB3(AABB3 const& copyFrom);
	explicit AABB3(float minX, float minY, float minZ, float maxX, float maxY, float maxZ);
	explicit AABB3(Vec3 const& mins, Vec3 const& maxs);
	Vec3 const GetDimensions() const;
	Vec3 GetCenter()const;
};