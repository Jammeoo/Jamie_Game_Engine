#pragma once
//-----------------------------------------------------------------------------------------------
#include <string>
#include <vector>
typedef std::vector< std::string >		Strings;

//-----------------------------------------------------------------------------------------------
const std::string Stringf( char const* format, ... );
const std::string Stringf( int maxLength, char const* format, ... );
Strings SplitStringOnDelimiter(std::string const& originalString, char delimiterToSplitOn);
Strings SplitStringOnDelimiter(std::string const& originalString, std::string delimiterToSplitOn);


Strings SplitStringWithQuotesAndRemoveQuotes(const std::string& originalString, char delimiterToSplitOn); 
Strings SplitStringWithQuotes(const std::string& originalString, char delimiterToSplitOn);

void TrimString(std::string& originalString, char delimiterToTrim); 
