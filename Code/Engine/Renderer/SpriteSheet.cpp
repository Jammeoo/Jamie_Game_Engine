#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Math/AABB2.hpp"

#include "Engine/Renderer/SpriteDefinition.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Renderer/Texture.hpp"
SpriteSheet::SpriteSheet(Texture& texture, IntVec2 const& simpleGridLayout)
	:m_texture(texture), m_dimension(simpleGridLayout)
{
	float uPerSpriteX = 1.f / static_cast<float>(simpleGridLayout.x);
	float vPerSpriteY = 1.f / static_cast<float>(simpleGridLayout.y);
	float offsetX = 1.f / (128.f* (float)simpleGridLayout.x);
	float offsetY = 1.f / (128.f * (float)simpleGridLayout.y);
	int totalNum = simpleGridLayout.x * simpleGridLayout.y;
	m_spriteDefs.reserve(totalNum);
	int spriteIndex = 0;
	for (int spriteY = simpleGridLayout.y - 1; spriteY >= 0; spriteY--) 
	{
		for (int spriteX = 0; spriteX < simpleGridLayout.x; spriteX++) 
		{
			float minU = static_cast<float>(spriteX) * uPerSpriteX;
			float maxU = minU + uPerSpriteX;
			float minV = static_cast<float>(vPerSpriteY) * spriteY;
			float maxV = minV + vPerSpriteY;
			SpriteDefinition spriteDef = SpriteDefinition(*this, spriteIndex, Vec2(minU + offsetX, minV + offsetY), Vec2(maxU - offsetX, maxV - offsetY));
			m_spriteDefs.push_back(spriteDef);
			spriteIndex++;
		}
	}

}



Texture& SpriteSheet::GetTexture() const
{
	return m_texture;
}

int SpriteSheet::GetNumSprites() const
{
	return (int)m_spriteDefs.size();
}

int SpriteSheet::GetSpriteIndexFor8x8(Vec2 uvMin) const
{
	return (int)(uvMin.x + (7 - uvMin.y) * 8);
}

SpriteDefinition const& SpriteSheet::GetSpriteDef(int spriteIndex) const
{
	return m_spriteDefs[spriteIndex];
}

void SpriteSheet::GetSpriteUVs(Vec2& out_uvAtMins, Vec2& out_uvAtMaxs, int spriteIndex) const
{
	out_uvAtMins = GetSpriteDef(spriteIndex).GetUVs().m_mins;
	out_uvAtMaxs = GetSpriteDef(spriteIndex).GetUVs().m_maxs;
}

AABB2 SpriteSheet::GetSpriteUVs(int spriteIndex) const
{
	AABB2 aabb = AABB2();
	GetSpriteUVs(aabb.m_mins, aabb.m_maxs, spriteIndex);
	return aabb;
}

AABB2 SpriteSheet::GetSpriteUVs(IntVec2 spriteCroods) const
{
	int spriteIndex = spriteCroods.x + spriteCroods.y * m_dimension.x;
	return GetSpriteUVs(spriteIndex);
}
