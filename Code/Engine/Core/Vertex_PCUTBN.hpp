#pragma once
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/Rgba8.hpp"

#include <Engine/Core/Vertex_PNCU.hpp>

//------------------------------------------------------------------------------------------------
struct Vertex_PCUTBN
{
	Vertex_PCUTBN() = default;
	Vertex_PCUTBN(Vec3 const& position, Rgba8 const& color, Vec2 const& uvTexCoords,Vec3 const& binormal, Vec3 const& tangent, Vec3 const& normal)
		: m_position(position)
		, m_normal(normal)
		, m_tangent(tangent)
		, m_binormal(binormal)
		, m_color(color)
		, m_uvTexCoords(uvTexCoords)
	{}
	Vertex_PCUTBN(Vertex_PNCU const& pncu) 
		: m_position(pncu.m_position)
		, m_normal(pncu.m_normal)
		, m_color(pncu.m_color)
		, m_uvTexCoords(pncu.m_uvTexCoords)
	{

	}
	Vertex_PCUTBN(Vec3 const& position, Vec3 const& normal, Rgba8 const& tint, Vec2 const& uvTexCoords)
		: m_position(position)
		, m_normal(normal)
		, m_color(tint)
		, m_uvTexCoords(uvTexCoords)
	{

	}
public:
	Vec3	m_position;
	Rgba8	m_color = Rgba8::WHITE;
	Vec2	m_uvTexCoords;
	Vec3	m_tangent;
	Vec3	m_binormal;
	Vec3	m_normal;
};