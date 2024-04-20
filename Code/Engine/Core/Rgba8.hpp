#pragma once
struct Rgba8
{
public:
	unsigned char r;
	unsigned char g;
	unsigned char b;
	unsigned char a;
	static const Rgba8 WHITE;
	static const Rgba8 RED;
	static const Rgba8 GREEN;
	static const Rgba8 LIGHTGREEN;
	static const Rgba8 BLUE;
	static const Rgba8 GREEN_BLUE;
	static const Rgba8 PINK;
	static const Rgba8 YELLOW;
	static const Rgba8 PURPLE;
	static const Rgba8 DEEP_BLUE;
	static const Rgba8 BLACK;
	static const Rgba8 BLUE_PURPLE;
	static const Rgba8 LIGHT_RED;
	static const Rgba8 DARK_RED;
	static const Rgba8 GRAY;
	static const Rgba8 LIGHT_GRAY;
	Rgba8();
	explicit Rgba8(unsigned char initialR, unsigned char initialG, unsigned char initialB, unsigned char initialA=255);
	void   SetFromText(char const* text);
	Rgba8 GetLighterColor();
	void	GetAsFloats(float* colorAsFloats) const;

	// Operators (const)
	bool		operator==(const Rgba8& compare) const;
};