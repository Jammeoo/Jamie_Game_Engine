#pragma once
#include <string>
#include "Engine/Core/EngineCommon.hpp"


class HashedCaseInsensitiveString
{
public:
	HashedCaseInsensitiveString() = default;
	HashedCaseInsensitiveString(HashedCaseInsensitiveString const& copyFrom) = default;
	HashedCaseInsensitiveString(std::string const& text);
	HashedCaseInsensitiveString(char const* text);


	static unsigned int CalHashForText(char const* text);
	static unsigned int CalHashForText(std::string const& text);


	bool operator==(HashedCaseInsensitiveString const& compareRHS)const;
	bool operator!=(HashedCaseInsensitiveString const& compareRHS)const;
	bool operator<(HashedCaseInsensitiveString const& compareRHS)const;

	bool operator==(std::string const& text)const;
	bool operator!=(std::string const& text)const;
	bool operator==(char const* text)const;
	bool operator!=(char const* text)const;
	void operator=(HashedCaseInsensitiveString const& compareRHS);
	void operator=(std::string const& assignFrom);
	void operator=(char const* assignFrom);


private:
	std::string m_oringinText;
	unsigned int m_hash = 0;
};
typedef HashedCaseInsensitiveString HCIString;