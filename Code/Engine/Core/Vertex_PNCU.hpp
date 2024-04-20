#pragma once
#include"Engine/Math/Vec3.hpp"
#include"Engine/Core/Rgba8.hpp"
#include"Engine/Math/Vec2.hpp"
struct Vertex_PNCU {
public:
	Vertex_PNCU();
	~Vertex_PNCU();
	explicit Vertex_PNCU(Vec3 const& position, Vec3 const& norml, Rgba8 const& tint, Vec2 const& uvTexCoords);
	explicit Vertex_PNCU(Vec3 const& position, Vec3 const& norml, Rgba8 const& tint);
public:
	Vec3 m_position = Vec3::WORLD_ORIGIN;
	Rgba8 m_color;
	Vec2 m_uvTexCoords = Vec2(0.0f, 0.0f);
	Vec3 m_normal = Vec3::WORLD_ORIGIN;
};