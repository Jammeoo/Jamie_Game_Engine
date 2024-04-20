#pragma once
#include <vector>
#include "Engine/Renderer/SpriteDefinition.hpp"
#include "Engine/Math/IntVec2.hpp"
class Texture;
struct Vec2;
struct AABB2;

class SpriteSheet
{
public:
	SpriteSheet();
	explicit SpriteSheet(Texture& texture, IntVec2 const& simpleGridLayout);//simpleGridLayout 
	Texture& GetTexture() const;
	int GetNumSprites() const;
	int GetSpriteIndexFor8x8(Vec2 uvMin)const;
	SpriteDefinition const& GetSpriteDef(int spriteIndex) const;
	void GetSpriteUVs(Vec2& out_uvAtMins, Vec2& out_uvAtMaxs, int spriteIndex) const;
	AABB2 GetSpriteUVs(int spriteIndex) const;
	AABB2 GetSpriteUVs(IntVec2 spriteCroods)const;
protected:
	Texture& m_texture;//reference members must be set in constructor's initializer list
	std::vector<SpriteDefinition> m_spriteDefs;
	IntVec2 m_dimension = IntVec2::ZERO;
};