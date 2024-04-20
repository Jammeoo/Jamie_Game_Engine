#include"Vertex_PCU.hpp"
Vertex_PCU::Vertex_PCU() {

}

Vertex_PCU::Vertex_PCU(float posiX, float posiY, float posiZ, Rgba8 const& tint, float uvX, float uvY)
{
	m_position = Vec3(posiX,posiY,posiZ);
	m_color = tint;
	m_uvTexCoords = Vec2(uvX, uvY);
}

Vertex_PCU::~Vertex_PCU() {

}

Vertex_PCU::Vertex_PCU(Vec3 const& position, Rgba8 const& tint, Vec2 const& uvTexCoords) {
	m_position = position;
	m_color = tint;
	m_uvTexCoords = uvTexCoords;
}
Vertex_PCU::Vertex_PCU(Vec3 const& position, Rgba8 const& tint) {
	m_position = position;
	m_color = tint;
}