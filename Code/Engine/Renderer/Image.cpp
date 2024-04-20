#define STB_IMAGE_IMPLEMENTATOIN
#include "ThirdParty/stb/stb_image.h"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Renderer/Image.hpp"

Image::Image()
{

}

Image::~Image()
{

}

Image::Image(const char* imageFilePath)
{
	int bytesPerTexel = 4;
	int numComponentsRequested = 0;
	m_imageFilePath = imageFilePath;
	stbi_set_flip_vertically_on_load(1); // We prefer uvTexCoords has origin (0,0) at BOTTOM LEFT
	unsigned char* texelData = stbi_load(imageFilePath, 
		&m_dimensions.x, &m_dimensions.y, &bytesPerTexel, numComponentsRequested);
	GUARANTEE_OR_DIE(texelData, Stringf("Failed to load image \"%s\"", imageFilePath));
	for (int imageY = 0; imageY < m_dimensions.y; imageY++) 
	{
		for (int imageX = 0; imageX < m_dimensions.x; imageX++)
		{
			int imageIndex = imageY * m_dimensions.x + imageX;
			Rgba8 rgba8OfImageIndex = Rgba8(0, 0, 0, 255);
			if (bytesPerTexel == 4)
			{
				rgba8OfImageIndex.r = texelData[bytesPerTexel * imageIndex];
				rgba8OfImageIndex.g = texelData[bytesPerTexel * imageIndex + 1];
				rgba8OfImageIndex.b = texelData[bytesPerTexel * imageIndex + 2];
				rgba8OfImageIndex.a = texelData[bytesPerTexel * imageIndex + 3];
			}else if(bytesPerTexel == 3)
			{
				rgba8OfImageIndex.r = texelData[bytesPerTexel * imageIndex];
				rgba8OfImageIndex.g = texelData[bytesPerTexel * imageIndex + 1];
				rgba8OfImageIndex.b = texelData[bytesPerTexel * imageIndex + 2];
			}
			else if (bytesPerTexel == 1) 
			{
				rgba8OfImageIndex.r = texelData[bytesPerTexel * imageIndex];
				rgba8OfImageIndex.g = texelData[bytesPerTexel * imageIndex];
				rgba8OfImageIndex.b = texelData[bytesPerTexel * imageIndex];
				
			}
			m_texelRgba8Data.push_back(rgba8OfImageIndex);
		}
	}


}

Image::Image(IntVec2 size, Rgba8 color)
{
	m_dimensions = size;
	m_imageFilePath = "";
	for (int imageY = 0; imageY < size.y; imageY++)
	{
		for (int imageX = 0; imageX < size.x; imageX++)
		{
			m_texelRgba8Data.push_back(color);
		}
	}
}

IntVec2 Image::GetDimensions() const
{
	return m_dimensions;
}

const std::string& Image::GetImageFilePath() const
{
	return m_imageFilePath;
}

const void* Image::GetRawData() const
{
	return (void*)m_texelRgba8Data.data();
}

const Rgba8 Image::GetTexelColor(IntVec2 const& texelCoords) const
{
	int texelIndex = texelCoords.y * m_dimensions.x + texelCoords.x;
	return m_texelRgba8Data[texelIndex];
}

