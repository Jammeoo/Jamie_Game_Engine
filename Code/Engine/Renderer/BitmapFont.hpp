#pragma once
#include <vector>
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/DevConsole.hpp"
//------------------------------------------------------------------------------------------------
	enum class TextDrawMode
	{
		SHRINK,
		OVERRUN,
	};

class BitmapFont
{
	friend class Renderer; // Only the Renderer can create new BitmapFont objects!


private:
	BitmapFont(char const* fontFilePathNameWithNoExtension, Texture& fontTexture);

public:
	Texture& GetTexture();

	void AddVertsForText2D(std::vector<Vertex_PCU>& vertexArray, Vec2 const& textMins,
		float cellHeight, std::string const& text, Rgba8 const& tint/* = Rgba8::WHITE*/, float cellAspect/* = 1.f*/);
	void AddVertsForFreeText3D(std::vector<Vertex_PCU>& verts, Vec2 const& textMins, float cellHeight, std::string const& text, Rgba8 const& tint = Rgba8::WHITE, float cellAspect = 1.f, Vec2 const& alignment = Vec2(0.5f, 0.5f), int maxGlyphsToDraw = 999999999);
	void AddVertsForText3D(std::vector<Vertex_PCU>& verts, Vec2 const& textMins, float cellHeight, std::string const& text, Rgba8 const& tint = Rgba8::WHITE, float cellAspect = 1.f, Vec2 const& alignment = Vec2(0.5f, 0.5f), int maxGlyphsToDraw = 999999999);
	void AddVertsForText3D(std::vector<Vertex_PCU>& verts, Vec3 const& textOrigins, Vec3 const& textRightDir, Vec3 const& textUpDir, float cellHeight, std::string const& text, Rgba8 const& tint = Rgba8::WHITE, float cellAspect = 1.f, int maxGlyphsToDraw = 999999999);

	//void AddVertsForTextInBox2D(std::vector<Vertex_PCU>& vertexArray, Vec2 const& textMins, float cellHeight, std::vector<DevConsoleLine> devConsoleTextLines, float cellAspect, float const& lineInBox);
	void AddVertsForTextInBox2D(std::vector<Vertex_PCU>& vertexArray, AABB2 const& box, float cellHeight, std::string const& text, Rgba8 const& tint/* = Rgba8(255, 255, 255)*/, float cellAspect/* = 1.f*/, Vec2 const& alignment /*= Vec2(0.5f, 0.5f)*/, TextDrawMode mode/* = TextDrawMode::SHRINK*/, int maxGlyphsToDraw/* = 99999999*/);
	float GetTextWidth(float cellHeight, std::string const& text, float cellAspect = 1.f);

protected:
	float GetGlyphAspect(int glyphUnicode) const; // For now this will always return 1.0f!!!

protected:
	std::string	m_fontFilePathNameWithNoExtension;
	SpriteSheet	m_fontGlyphsSpriteSheet;
};
