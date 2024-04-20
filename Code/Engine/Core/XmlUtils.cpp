#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/FloatRange.hpp"
int ParseXmlAttribute(XmlElement const& element, char const* attributeName, int defaultValue)
{
	int result = defaultValue;
	element.QueryIntAttribute(attributeName, &result);
	return result;
}

char ParseXmlAttribute(XmlElement const& element, char const* attributeName, char defaultValue)
{
	const char* result = nullptr;
	result = element.Attribute(attributeName);
	if (result == nullptr) 
	{
		return defaultValue;
	}
	return *result;
}

bool ParseXmlAttribute(XmlElement const& element, char const* attributeName, bool defaultValue)
{
	bool result = defaultValue;
	element.QueryBoolAttribute(attributeName, &result);
	return result;
}

float ParseXmlAttribute(XmlElement const& element, char const* attributeName, float defaultValue)
{
	float result = defaultValue;
	element.QueryFloatAttribute(attributeName, &result);
	return result;
}

Rgba8 ParseXmlAttribute(XmlElement const& element, char const* attributeName, Rgba8 const& defaultValue)
{
	const char* resultString = nullptr;
	resultString = element.Attribute(attributeName);
	if (resultString == nullptr) 
	{
		return defaultValue;
	}
	Rgba8 result = Rgba8();
	result.SetFromText(resultString);
	return result;
}

Vec2 ParseXmlAttribute(XmlElement const& element, char const* attributeName, Vec2 const& defaultValue)
{
	const char* resultString = nullptr;
	resultString = element.Attribute(attributeName);
	if (resultString == nullptr)
	{
		return defaultValue;
	}
	Vec2 result = Vec2();
	result.SetFromText(resultString);
	return result;
}

IntVec2 ParseXmlAttribute(XmlElement const& element, char const* attributeName, IntVec2 const& defaultValue)
{
	const char* resultString = nullptr;
	resultString = element.Attribute(attributeName);
	if (resultString == nullptr)
	{
		return defaultValue;
	}
	IntVec2 result = IntVec2();
	result.SetFromText(resultString);
	return result;
}

std::string ParseXmlAttribute(XmlElement const& element, char const* attributeName, std::string const& defaultValue)
{
	const char* resultString = nullptr;
	resultString = element.Attribute(attributeName);
	if (resultString == nullptr) 
	{
		return defaultValue;
	}
	return resultString;
}

Strings ParseXmlAttribute(XmlElement const& element, char const* attributeName, Strings const& defaultValues)
{
	const char* resultString = nullptr;
	resultString = element.Attribute(attributeName);
	if (resultString == nullptr) 
	{
		return defaultValues;
	}
	Strings resultStrings = SplitStringOnDelimiter(resultString, ',');
	return resultStrings;
}

Vec3 ParseXmlAttribute(XmlElement const& element, char const* attributeName, Vec3 const& defaultValue)
{
	const char* resultString = nullptr;
	resultString = element.Attribute(attributeName);
	if (resultString == nullptr)
	{
		return defaultValue;
	}
	Vec3 result = Vec3();
	result.SetFromText(resultString, ',');
	return result;
}

FloatRange ParseXmlAttribute(XmlElement const& element, char const* attributeName, FloatRange const& defaultValue)
{
	const char* resultString = nullptr;
	resultString = element.Attribute(attributeName);
	if (resultString == nullptr)
	{
		return defaultValue;
	}
	Vec2 dOC;
	dOC.SetFromText(resultString, ',');
	FloatRange result = FloatRange(dOC.x, dOC.y);
	return result;
}

