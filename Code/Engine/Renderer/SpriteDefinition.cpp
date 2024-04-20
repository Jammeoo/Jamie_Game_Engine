#include "Engine/Renderer/SpriteDefinition.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
SpriteDefinition::SpriteDefinition(SpriteSheet const& SpriteSheet, int spriteIndex, Vec2 const& uvAtMins, Vec2 const& uvAtMaxs)
	:m_spriteSheet(SpriteSheet),
	m_spriteIndex(spriteIndex),
	m_uvAtMins(uvAtMins),
	m_uvAtMaxs(uvAtMaxs)
{

}

void SpriteDefinition::GetUVs(Vec2& out_uvAtMins, Vec2& out_uvAtMaxs) const
{
	out_uvAtMins = m_uvAtMins;
	out_uvAtMaxs = m_uvAtMaxs;
}

AABB2 SpriteDefinition::GetUVs() const
{
	AABB2 aabb = AABB2(m_uvAtMins, m_uvAtMaxs);
	return aabb;
}

SpriteSheet const& SpriteDefinition::GetSpritesheet() const
{
	return m_spriteSheet;
}

Texture& SpriteDefinition::GetTexture() const
{
	return m_spriteSheet.GetTexture();
}

float SpriteDefinition::GetAspect() const
{
	return (m_uvAtMaxs.x - m_uvAtMins.x) / (m_uvAtMaxs.y - m_uvAtMins.y);
}
