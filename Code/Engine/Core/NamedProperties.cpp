#include "Engine/Core/NamedProperties.hpp"



// void NamedProperties::PopulateFromXmlElementAttributes(XmlElement const& element)
// {
// 	auto attribute = element.FirstAttribute();
// 	while (attribute) 
// 	{
// 		SetValue(attribute->Name(), attribute->Value());
// 		attribute = attribute->Next();
// 	}
// }
// 
// void NamedProperties::SetValue(std::string const& keyName, std::string const& newValue)
// {
// 	m_keyValuePairs[keyName] = newValue;
// }
// 
// std::string NamedProperties::GetValue(std::string const& keyName, std::string const& defaultValue) const
// {
// 	auto found = m_keyValuePairs.find(keyName);
// 	if (found == m_keyValuePairs.end()) 
// 	{
// 		return defaultValue;
// 	}
// 	std::string const& value = found->second;
// 	return value.c_str();
// }
// 
// bool NamedProperties::GetValue(std::string const& keyName, bool defaultValue) const
// {
// 	auto found = m_keyValuePairs.find(keyName);
// 	if (found == m_keyValuePairs.end())
// 	{
// 		return defaultValue;
// 	}
// 	//std::string const& value = found->second;
// 	if (found->second == "1"||found->second == "true")
// 	{
// 		return true;
// 	}
// 	else
// 	{
// 		return false;
// 	}
// }
// 
// int NamedProperties::GetValue(std::string const& keyName, int defaultValue) const
// {
// 	auto found = m_keyValuePairs.find(keyName);
// 	if (found == m_keyValuePairs.end())
// 	{
// 		return defaultValue;
// 	}
// 	std::string const& value = found->second;
// 	
// 	return  (int)atoi(value.c_str());
// }
// 
// float NamedProperties::GetValue(std::string const& keyName, float defaultValue) const
// {
// 	auto found = m_keyValuePairs.find(keyName);
// 	if (found == m_keyValuePairs.end())
// 	{
// 		return defaultValue;
// 	}
// 	std::string const& value = found->second;
// 	return  (float)atof(value.c_str());
// }
// 
// std::string NamedProperties::GetValue(std::string const& keyName, char const* defaultValue) const
// {
// 	auto found = m_keyValuePairs.find(keyName);
// 	if (found == m_keyValuePairs.end())
// 	{
// 		return defaultValue;
// 	}
// 	std::string const& value = found->second;
// 	return  value.c_str();
// }
// 
// Rgba8 NamedProperties::GetValue(std::string const& keyName, Rgba8 const& defaultValue) const
// {
// 	auto found = m_keyValuePairs.find(keyName);
// 	if (found == m_keyValuePairs.end())
// 	{
// 		return defaultValue;
// 	}
// 	std::string const& value = found->second;
// 	Rgba8 result;
// 	result.SetFromText(value.c_str());
// 	return  result;
// }
// 
// Vec2 NamedProperties::GetValue(std::string const& keyName, Vec2 const& defaultValue) const
// {
// 	auto found = m_keyValuePairs.find(keyName);
// 	if (found == m_keyValuePairs.end())
// 	{
// 		return defaultValue;
// 	}
// 	std::string const& value = found->second;
// 	Vec2 result;
// 	result.SetFromText(value.c_str());
// 	return  result;
// }
// 
// IntVec2 NamedProperties::GetValue(std::string const& keyName, IntVec2 const& defaultValue) const
// {
// 	auto found = m_keyValuePairs.find(keyName);
// 	if (found == m_keyValuePairs.end())
// 	{
// 		return defaultValue;
// 	}
// 	std::string const& value = found->second;
// 	IntVec2 result;
// 	result.SetFromText(value.c_str());
// 	return  result;
// }
// 
// Vec3 NamedProperties::GetValue(std::string const& keyName, Vec3 const& defaultValue) const
// {
// 	auto found = m_keyValuePairs.find(keyName);
// 	if (found == m_keyValuePairs.end())
// 	{
// 		return defaultValue;
// 	}
// 
// 	std::string const& value = found->second;
// 	Vec3 result;
// 	result.SetFromText(value.c_str(),',');
// 	return  result;
// }
// 
// EulerAngles NamedProperties::GetValue(std::string const& keyName, EulerAngles const& defaultValue) const
// {
// 	auto found = m_keyValuePairs.find(keyName);
// 	if (found == m_keyValuePairs.end())
// 	{
// 		return defaultValue;
// 	}
// 
// 	std::string const& value = found->second;
// 	EulerAngles result;
// 	result.SetFromText(value.c_str());
// 	return  result;
// }
