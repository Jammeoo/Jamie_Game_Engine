#pragma once
#include <map>
#include <string>
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

//In order to be able to put this in to the same list witch is a std::map, so we create a base class and we
class NamedPropertyBase 
{
protected:
	virtual ~NamedPropertyBase(){}
};
//----------------------------------------------------------------------------------------------------------------------//
template<typename T>
class NamedPropertyOfType :public NamedPropertyBase 
{
	friend class NamedProperties;
protected:
	NamedPropertyOfType(T const& value);
	T m_value;
};

template<typename T>
NamedPropertyOfType<T>::NamedPropertyOfType(T const& value) :m_value(value)
{

}


class NamedProperties
{
public:
	template<typename T>
	void			SetValue(std::string const& keyName, T const& newValue);
	template<typename T>
	T			GetValue(std::string const& keyName, T const& defalultValue);

// 	void			PopulateFromXmlElementAttributes(XmlElement const& element);
// 	void			SetValue(std::string const& keyName, std::string const& newValue);
// 	std::string		GetValue(std::string const& keyName, std::string const& defaultValue) const;
// 	bool			GetValue(std::string const& keyName, bool defaultValue) const;
// 	int			GetValue(std::string const& keyName, int defaultValue) const;
// 	float			GetValue(std::string const& keyName, float defaultValue) const;
// 	std::string		GetValue(std::string const& keyName, char const* defaultValue) const;
// 	Rgba8			GetValue(std::string const& keyName, Rgba8 const& defaultValue) const;
// 	Vec2			GetValue(std::string const& keyName, Vec2 const& defaultValue) const;
// 	IntVec2		GetValue(std::string const& keyName, IntVec2 const& defaultValue) const;
// 	Vec3		GetValue(std::string const& keyName, Vec3 const& defaultValue)const;
// 	EulerAngles GetValue(std::string const& keyName, EulerAngles const& defaultValue)const;
protected:
private:
	std::map< std::string, NamedPropertyBase* >	m_keyValuePairs;
};

//----------------------------------------------------------------------------------------------------------------------//
template<typename T>
void NamedProperties::SetValue(std::string const& keyName, T const& value)
{
	m_keyValuePairs[keyName] = new NamedPropertyOfType<T>(value);
}

//----------------------------------------------------------------------------------------------------------------------//
template<typename T>
T NamedProperties::GetValue(std::string const& keyName, T const& defalultValue)
{
	std::map<std::string, NamedPropertyBase*>::iterator found = m_keyValuePairs.find(keyName);
	if (found == m_keyValuePairs.end()) 
	{
		return defalultValue;
	}
	NamedPropertyBase* property = found->second;
	NamedPropertyOfType<T>* typeProperty = dynamic_cast<NamedPropertyOfType<T>*>(property);
	if (!typeProperty) 
	{
		//Key was found but value is not the correct type!
		ERROR_RECOVERABLE(Stringf("GetValue asked for value of the wrong type!"));
		return defalultValue;
	}
	return typeProperty->m_value;
}
