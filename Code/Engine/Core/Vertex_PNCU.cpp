#include "Engine/Core/Vertex_PNCU.hpp"

Vertex_PNCU::Vertex_PNCU()
{

}

Vertex_PNCU::Vertex_PNCU(Vec3 const& position, Vec3 const& normal, Rgba8 const& tint, Vec2 const& uvTexCoords)
{
	m_position = position;
	m_normal = normal;
	m_color = tint;
	m_uvTexCoords = uvTexCoords;
}

Vertex_PNCU::Vertex_PNCU(Vec3 const& position, Vec3 const& normal, Rgba8 const& tint)
{
	m_position = position;
	m_normal = normal;
	m_color = tint;
}

Vertex_PNCU::~Vertex_PNCU()
{

}
