#include "Engine/Core/HashedCaseInsensitiveString.hpp"

HashedCaseInsensitiveString::HashedCaseInsensitiveString(std::string const& text)
	:m_oringinText(text),
	m_hash(CalHashForText(text))
{

}

HashedCaseInsensitiveString::HashedCaseInsensitiveString(char const* text)
	:m_oringinText(text),
	m_hash(CalHashForText(text))
{

}

unsigned int HashedCaseInsensitiveString::CalHashForText(char const* text)
{
	unsigned int hash = 0;
	for (char const* readPos = text; *readPos != '\0'; ++readPos) 
	{
		hash *= 31;
		hash += (unsigned int)tolower(*readPos);
	}
	return hash;

}

unsigned int HashedCaseInsensitiveString::CalHashForText(std::string const& text)
{
	return CalHashForText(text.c_str());
}

bool HashedCaseInsensitiveString::operator<(HashedCaseInsensitiveString const& compareRHS) const
{
	if (m_hash < compareRHS.m_hash)
	{
		return true;
	}
	else if (m_hash > compareRHS.m_hash)
	{
		return false;
	}
	else
	{
		//Hash value are equeal; use an actual string comparsion to determine equality, <, stc
		return _stricmp(m_oringinText.c_str(), compareRHS.m_oringinText.c_str()) < 0;
	}
}

bool HashedCaseInsensitiveString::operator!=(HashedCaseInsensitiveString const& compareRHS) const
{
	return !(*this== compareRHS);
}

bool HashedCaseInsensitiveString::operator==(std::string const& text) const
{
	return (_stricmp(m_oringinText.c_str(), text.c_str()) == 0);
}

bool HashedCaseInsensitiveString::operator==(HashedCaseInsensitiveString const& compareRHS) const
{
	if (m_hash != compareRHS.m_hash)
	{

		return false;
	}
	else
	{
		//has
		bool areStringEqual = (0 == _stricmp(m_oringinText.c_str(), compareRHS.m_oringinText.c_str()));
		return areStringEqual;
	}

}

