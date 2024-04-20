#pragma once
#include <string>
#include <vector>
#include "Engine/Math/IntVec2.hpp"
struct Rgba8;

class Image
{
	friend class Renderer;
#ifdef ENABLE_D3D12
	friend class RendererD12;
#endif // ENABLE_D3D12
public:
	Image();
	~Image();
	Image(const char* imageFilePath);
	Image(IntVec2 size, Rgba8 color);

	IntVec2 GetDimensions()const;
	const std::string& GetImageFilePath() const;
	const void* GetRawData() const;
	const Rgba8 GetTexelColor(IntVec2 const& texelCoords)const;
	std::vector<Rgba8> GeTexelData()const { return m_texelRgba8Data; }
private:
	std::string m_imageFilePath;
	IntVec2 m_dimensions = IntVec2::ZERO;
	std::vector<Rgba8> m_texelRgba8Data;
};