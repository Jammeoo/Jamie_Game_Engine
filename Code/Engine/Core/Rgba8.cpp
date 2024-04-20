#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
const Rgba8 Rgba8::WHITE =			Rgba8(255, 255, 255, 255);
const Rgba8 Rgba8::RED =			Rgba8(255, 0, 0, 255);
const Rgba8 Rgba8::GREEN =			Rgba8(0, 255, 0, 255);
const Rgba8 Rgba8::LIGHTGREEN =		Rgba8(102, 255, 102, 255);
const Rgba8 Rgba8::BLUE =			Rgba8(0, 0, 255, 255);
const Rgba8 Rgba8::GREEN_BLUE =		Rgba8(0, 255, 255, 255);
const Rgba8 Rgba8::PINK =			Rgba8(255, 0, 102, 255);
const Rgba8 Rgba8::YELLOW =			Rgba8(255, 255, 153, 255);
const Rgba8 Rgba8::PURPLE =			Rgba8(128, 0, 128, 255);
const Rgba8 Rgba8::DEEP_BLUE =		Rgba8(128, 0, 128, 255);
const Rgba8 Rgba8::BLACK =			Rgba8(0, 0, 0, 255);
const Rgba8 Rgba8::BLUE_PURPLE =	Rgba8(159, 159, 213, 255);
const Rgba8 Rgba8::LIGHT_RED =		Rgba8(213, 159, 159, 255);
const Rgba8 Rgba8::DARK_RED =		Rgba8(176, 33, 33, 255);
const Rgba8 Rgba8::GRAY =			Rgba8(127, 127, 127, 255);
const Rgba8 Rgba8::LIGHT_GRAY = Rgba8(209, 209, 224, 255);

Rgba8::Rgba8(){
	r = g = b = a = 255;
};
Rgba8::Rgba8(unsigned char initialR, unsigned char initialG, unsigned char initialB, unsigned char initialA) {
	r = initialR;
	g = initialG;
	b = initialB;
	a = initialA;
}

void Rgba8::SetFromText(char const* text)
{
	Strings originalText = SplitStringOnDelimiter(text, ',');

	if (originalText.size() == 3) 
	{
		r = (unsigned char)atoi(originalText[0].c_str());
		g = (unsigned char)atoi(originalText[1].c_str());
		b = (unsigned char)atoi(originalText[2].c_str());
	}else if (originalText.size() == 4)
	{
		r = (unsigned char)atoi(originalText[0].c_str());
		g = (unsigned char)atoi(originalText[1].c_str());
		b = (unsigned char)atoi(originalText[2].c_str());
		a = (unsigned char)atoi(originalText[3].c_str());
	}
}


Rgba8 Rgba8::GetLighterColor()
{
	int rgbaR = 0, rgbaG = 0, rgbaB = 0;
	rgbaR = (int)r + 100;
	rgbaG = (int)g + 100;
	rgbaB = (int)b + 100;
	if (rgbaR > 255) rgbaR = 255;
	if (rgbaG > 255) rgbaG = 255;
	if (rgbaB > 255) rgbaB = 255;
	Rgba8 rgba8 = Rgba8((unsigned char)rgbaR, (unsigned char)rgbaG, (unsigned char)rgbaB, a);
	return rgba8;
}

void Rgba8::GetAsFloats(float* colorAsFloats)	const		//This method is used to get the color as an array of floats
{
	colorAsFloats[0] = NormalizeByte(r);
	colorAsFloats[1] = NormalizeByte(g);
	colorAsFloats[2] = NormalizeByte(b);
	colorAsFloats[3] = NormalizeByte(a);
}

bool Rgba8::operator==(const Rgba8& compare) const
{
	return (r == compare.r && g == compare.g && b == compare.b && a == compare.a);
}
