#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Renderer/SpriteDefinition.hpp"
#include "Engine/Math/Vec3.hpp"


//#include "Engine/Renderer/Renderer.hpp"
BitmapFont::BitmapFont(char const* fontFilePathNameWithNoExtension, Texture& fontTexture)
	:m_fontFilePathNameWithNoExtension(fontFilePathNameWithNoExtension),
	m_fontGlyphsSpriteSheet(fontTexture, IntVec2(16, 16))
{
	
}

Texture& BitmapFont::GetTexture()
{
	return m_fontGlyphsSpriteSheet.GetTexture();
}

void BitmapFont::AddVertsForText2D(std::vector<Vertex_PCU>& vertexArray, Vec2 const& textMins, float cellHeight, std::string const& text, Rgba8 const& tint, float cellAspect)
{
	float cellLength = cellHeight * cellAspect;
	Vec2 bitMapFontMins, bitMapFontMaxs;
	for (int index = 0; index < text.length(); index++) 
	{
		//Calculate character min and max
		bitMapFontMins = Vec2(textMins.x + index * cellLength, textMins.y);
		bitMapFontMaxs = Vec2(bitMapFontMins.x + cellLength, bitMapFontMins.y + cellHeight);
		AABB2 bitMapBounds = AABB2(bitMapFontMins.x, bitMapFontMins.y, bitMapFontMaxs.x, bitMapFontMaxs.y);

		Vec2 uvMin = Vec2::ZERO;
		Vec2 uvMax = Vec2::ZERO;
		m_fontGlyphsSpriteSheet.GetSpriteUVs(uvMin, uvMax, (int)text[index]);
		AddVertsForAABB2D(vertexArray, bitMapBounds, tint, uvMin, uvMax);

	}
}

void BitmapFont::AddVertsForFreeText3D(std::vector<Vertex_PCU>& verts, Vec2 const& textMins, float cellHeight, std::string const& text, Rgba8 const& tint /*= Rgba8::WHITE*/, float cellAspect /*= 1.f*/, Vec2 const& alignment /*= Vec2(0.5f, 0.5f)*/, int maxGlyphsToDraw /*= 999999999*/)
{
	UNUSED(maxGlyphsToDraw);
	UNUSED(alignment);
	std::vector<Vertex_PCU> tempVerts;
	AddVertsForText2D(tempVerts, textMins, cellHeight, text, tint, cellAspect);

	for (int vertsIndex = 0; vertsIndex < (int)tempVerts.size(); vertsIndex++) 
	{
		Vertex_PCU tempV3 = tempVerts[vertsIndex];
		tempV3.m_position = Vec3(tempVerts[vertsIndex].m_position.x, tempVerts[vertsIndex].m_position.y, 0.f);
		tempVerts[vertsIndex].m_position = tempV3.m_position;
		verts.push_back(tempV3);
	}
}


void BitmapFont::AddVertsForText3D(std::vector<Vertex_PCU>& verts, Vec2 const& textMins, float cellHeight, std::string const& text, Rgba8 const& tint /*= Rgba8::WHITE*/, float cellAspect /*= 1.f*/, Vec2 const& alignment /*= Vec2(0.5f, 0.5f)*/, int maxGlyphsToDraw /*= 999999999*/)
{
	UNUSED(maxGlyphsToDraw);
	std::vector<Vertex_PCU> tempVerts;
	AddVertsForText2D(tempVerts, textMins, cellHeight, text, tint, cellAspect);

	AABB2 textBounds = GetVertexBounds2D(tempVerts);
	Vec2 translation;
	translation.x = abs(textBounds.m_maxs.x - textBounds.m_mins.x);
	translation.y = abs(textBounds.m_maxs.y - textBounds.m_maxs.y);
	translation.x *= alignment.x;
	translation.y *= alignment.y;
	Mat44 originAligned;
	originAligned.SetIJK3D(Vec3::Y_AXIS, Vec3::Z_AXIS, Vec3::X_AXIS);
	originAligned.AppendTranslation3D((Vec3(-translation.x, -translation.y, 0.f)));
	TransformVertexArray3D(tempVerts, originAligned);
	verts.insert(verts.end(), tempVerts.begin(), tempVerts.end());
}

void BitmapFont::AddVertsForText3D(std::vector<Vertex_PCU>& verts, Vec3 const& textOrigins, Vec3 const& iBasis, Vec3 const& jBasis, float cellHeight, std::string const& text, Rgba8 const& tint /*= Rgba8::WHITE*/, float cellAspect /*= 1.f*/, int maxGlyphsToDraw /*= 999999999*/)
{
	UNUSED(maxGlyphsToDraw);
	std::vector<Vertex_PCU> tempVerts;
	AddVertsForText2D(tempVerts, Vec2::ZERO, cellHeight, text, tint, cellAspect);
	Vec3 kBasis = CrossProduct3D(iBasis, jBasis).GetNormalized();
	Mat44 transform(iBasis, jBasis, kBasis, textOrigins);
	TransformVertexArray3D(tempVerts, transform);
	verts.insert(verts.end(), tempVerts.begin(), tempVerts.end());
}

// 
// void BitmapFont::AddVertsForTextInBox2D(std::vector<Vertex_PCU>& vertexArray, Vec2 const& textMins, 
// 	float cellHeight, std::vector<DevConsoleLine> devConsoleTextLines, float cellAspect, float const& lineInBox)
// {
// 	Vec2 startMins = textMins;
// 	int lineNum = (int)lineInBox-1;									//39 lines for commands
// 	for (int lineIndex = devConsoleTextLines.size() - 1; lineIndex >= 0 && lineNum <= 40; lineIndex-- && lineNum--)
// 	{
// 		Vec2 currentMins = startMins + Vec2(0.f, (devConsoleTextLines.size() - lineIndex - 1) * cellHeight);
// 		AddVertsForText2D(vertexArray, currentMins, cellHeight, devConsoleTextLines[lineIndex].m_text, devConsoleTextLines[lineIndex].m_color, cellAspect);
// 	}
// }

void BitmapFont::AddVertsForTextInBox2D(std::vector<Vertex_PCU>& vertexArray, AABB2 const& box, float cellHeight, std::string const& text, Rgba8 const& tint /*= Rgba8(255, 255, 255)*/,
	float cellAspect /*= 1.f*/, Vec2 const& alignment /*= Vec2(0.5f, 0.5f)*/, TextDrawMode mode /*= TextDrawMode::SHRINK*/, int maxGlyphsToDraw/* = 99999999*/)
{
	UNUSED(maxGlyphsToDraw);
	vertexArray.reserve(3000);
	Strings dividedText = SplitStringOnDelimiter(text, '\n');
	float cellWidth = cellHeight / cellAspect;

	if (mode == TextDrawMode::OVERRUN)
	{
		//Calculate min for each Texts Box
		int maxWidthOfLines = 0;
		for (int lineIndex = 0; lineIndex < (int)dividedText.size(); lineIndex++)
		{
			if (lineIndex == 0)
			{
				maxWidthOfLines = (int)dividedText[lineIndex].length();
			}
			else
			{
				if ((int)dividedText[lineIndex].length() > maxWidthOfLines)
				{
					maxWidthOfLines = (int)dividedText[lineIndex].length();
				}
			}
		}

		float textBlockHeight = (float)dividedText.size() * cellHeight;
		float textBlockWidth = (float)maxWidthOfLines * cellWidth;
		Vec2 starterMins = box.m_mins + (box.GetDimensions() - Vec2(textBlockWidth, textBlockHeight)) * alignment;
		for (int printLineIndex = 0; printLineIndex < (int)dividedText.size(); printLineIndex++)
		{
			AddVertsForText2D(vertexArray, starterMins, cellHeight, dividedText[printLineIndex], tint, cellAspect);
		}
	}
	else //mode == TextDrawMode::SHRINK
	{
		//Calculate min for each Texts Box
		int maxUnitOfLines = 0;
		for (int lineIndex = 0; lineIndex < (int)dividedText.size(); lineIndex++)
		{
			if (lineIndex == 0)
			{
				maxUnitOfLines = (int)dividedText[lineIndex].length();
			}
			else
			{
				if ((int)dividedText[lineIndex].length() > maxUnitOfLines)
				{
					maxUnitOfLines = (int)dividedText[lineIndex].length();
				}
			}
		}

		if (maxUnitOfLines * cellWidth > box.GetDimensions().x)
		{
			if (cellHeight * (float)dividedText.size() >= box.GetDimensions().y) 
			{
				//choose one between cellWidth and cellHeight which are relatively smaller
				if (cellWidth * cellAspect > cellHeight) //CellHeight is smaller
				{
					cellHeight = box.GetDimensions().y / (float)dividedText.size();
					cellWidth = cellHeight / cellAspect;										//re-write new cellWidth based on new height
				}
				else
				{
					cellWidth = box.GetDimensions().x / maxUnitOfLines;
					cellHeight = cellHeight * cellAspect;										//re-write new height based on new cellWidth
				}
			}
			else 
			{
				cellWidth = box.GetDimensions().x / maxUnitOfLines;					//cellWidth + cellWidth
			}

		}
		
		float textBlockHeight = (float)dividedText.size() * cellHeight;
		float textBlockWidth = (float)maxUnitOfLines * cellWidth;
		Vec2 starterMins = box.m_mins + (box.GetDimensions() - Vec2(textBlockWidth, textBlockHeight)) * alignment;
		for (int printLineIndex = 0; printLineIndex < (int)dividedText.size(); printLineIndex++)
		{
			AddVertsForText2D(vertexArray, starterMins, cellHeight, dividedText[printLineIndex], tint, cellAspect);
		}

	}
}

float BitmapFont::GetTextWidth(float cellHeight, std::string const& text, float cellAspect /*= 1.f*/)
{
	float cellWidth = cellHeight * cellAspect;
	return text.length() * cellWidth;
}

float BitmapFont::GetGlyphAspect(int glyphUnicode) const
{
	return (float)(glyphUnicode/ glyphUnicode);
}
