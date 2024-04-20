#include "Engine/Core/StringUtils.hpp"
#include <stdarg.h>
#include "Engine/Core/ErrorWarningAssert.hpp"


//-----------------------------------------------------------------------------------------------
constexpr int STRINGF_STACK_LOCAL_TEMP_LENGTH = 2048;


//-----------------------------------------------------------------------------------------------
const std::string Stringf( char const* format, ... )
{
	char textLiteral[ STRINGF_STACK_LOCAL_TEMP_LENGTH ];
	va_list variableArgumentList;
	va_start( variableArgumentList, format );
	vsnprintf_s( textLiteral, STRINGF_STACK_LOCAL_TEMP_LENGTH, _TRUNCATE, format, variableArgumentList );	
	va_end( variableArgumentList );
	textLiteral[ STRINGF_STACK_LOCAL_TEMP_LENGTH - 1 ] = '\0'; // In case vsnprintf overran (doesn't auto-terminate)

	return std::string( textLiteral );
}


//-----------------------------------------------------------------------------------------------
const std::string Stringf( int maxLength, char const* format, ... )
{
	char textLiteralSmall[ STRINGF_STACK_LOCAL_TEMP_LENGTH ];
	char* textLiteral = textLiteralSmall;
	if( maxLength > STRINGF_STACK_LOCAL_TEMP_LENGTH )
		textLiteral = new char[ maxLength ];

	va_list variableArgumentList;
	va_start( variableArgumentList, format );
	vsnprintf_s( textLiteral, maxLength, _TRUNCATE, format, variableArgumentList );	
	va_end( variableArgumentList );
	textLiteral[ maxLength - 1 ] = '\0'; // In case vsnprintf overran (doesn't auto-terminate)

	std::string returnValue( textLiteral );
	if( maxLength > STRINGF_STACK_LOCAL_TEMP_LENGTH )
		delete[] textLiteral;

	return returnValue;
}

Strings SplitStringOnDelimiter(std::string const& originalString, char delimiterToSplitOn)
{
	std::string inputString = originalString;
	std::string subString;
	Strings result;
	size_t index = 0;
	while (index != std::string::npos) 
	{
		//Search for first delimiter, save the position in index
		index = inputString.find(delimiterToSplitOn);

		subString = inputString.substr(0, index);
		result.emplace_back(subString);

		inputString.erase(0, index + 1);

	}

	return result;

}

Strings SplitStringOnDelimiter(std::string const& originalString, std::string delimiterToSplitOn)
{
	size_t pos_start = 0;
	size_t pos_end;
	size_t delim_len = delimiterToSplitOn.length();
	std::string token;
	Strings result;

	while ((pos_end = originalString.find(delimiterToSplitOn, pos_start)) != std::string::npos) {
		token = originalString.substr(pos_start, pos_end - pos_start);
		pos_start = pos_end + delim_len;
		result.push_back(token);
	}
	result.push_back(originalString.substr(pos_start));
	return result;
}

Strings SplitStringWithQuotesAndRemoveQuotes(const std::string& originalString, char delimiterToSplitOn)
{
	//Same as SplitStringOnDelimiter but ignores delimiters inside double quotes.
	
	std::string inputString = originalString;
	std::string subString;
	std::string subSecondString;
	Strings result;
	size_t index = 0;
	size_t quotesIndex = 0;

	//quotesIndex = inputString.find('"');
	//if (quotesIndex == std::string::npos) 
	//{
	//	ERROR_AND_DIE("No Quotes in this string");
	//}

	while (index != std::string::npos)											//fji*dw*dwaf"dawf*dwad*dwa"awdwa*dwad*dawdwaf

	{
		//Search for first delimiter, save the position in index
		index = inputString.find(delimiterToSplitOn);
		quotesIndex = inputString.find('"');
		//compare the index and quotesIndex

		//Pick the smallest to use
		if (index <= quotesIndex) 
		{
			subString = inputString.substr(0, index);
			if (index != 0) 
			{
				result.push_back(subString);
			}
			inputString.erase(0, index + 1);									//fji + dw + dwaf"dawf*dwad*dwa"awdwa*dwad*dawdwaf
		}
		else
		{
			subString = inputString.substr(0, quotesIndex);						//dwaf"dawf*dwad*dwa"awdwa*dwad*dawdwaf
			//result.push_back(subString);
			inputString.erase(0, quotesIndex + 1);								//dwaf + dawf*dwad*dwa"awdwa*dwad*dawdwaf
			quotesIndex = inputString.find('"');
			subSecondString = inputString.substr(0, quotesIndex);
			subString.append(subSecondString.c_str());							
			inputString.erase(0, quotesIndex + 1);								//dwafdawf*dwad*dwa + awdwa*dwad*dawdwaf
			result.push_back(subString);
		}

	}

	return result;

}

Strings SplitStringWithQuotes(const std::string& originalString, char delimiterToSplitOn)
{
	//Same as SplitStringOnDelimiter but ignores delimiters inside double quotes.

	std::string inputString = originalString;
	std::string subString;
	std::string subSecondString;
	Strings result;
	size_t index = 0;
	size_t quotesIndex = 0;


	while (index != std::string::npos)											//fji*dw*dwaf"dawf*dwad*dwa"awdwa*dwad*dawdwaf

	{
		//Search for first delimiter, save the position in index
		index = inputString.find(delimiterToSplitOn);
		quotesIndex = inputString.find('"');
		//compare the index and quotesIndex

		//Pick the smallest to use
		if (index <= quotesIndex)
		{
			subString = inputString.substr(0, index);
			if (index != 0 && !subString.empty())
			{
				result.push_back(subString);
			}
			inputString.erase(0, index + 1);									//fji + dw + dwaf"dawf*dwad*dwa"awdwa*dwad*dawdwaf
		}
		else
		{
			subString = inputString.substr(0, quotesIndex + 1);						//dwaf"dawf*dwad*dwa"awdwa*dwad*dawdwaf
			//result.push_back(subString);
			inputString.erase(0, quotesIndex + 1);								//dwaf" + dawf*dwad*dwa"awdwa*dwad*dawdwaf
			quotesIndex = inputString.find('"');
			subSecondString = inputString.substr(0, quotesIndex + 1);
			subString.append(subSecondString.c_str());
			inputString.erase(0, quotesIndex + 1);								//dwaf"dawf*dwad*dwa" + awdwa*dwad*dawdwaf
			result.push_back(subString);
		}

	}

	return result;
}

void TrimString(std::string& originalString, char delimiterToTrim)
{
	//Removes any occurrence of delimiter from the front and back of the string.
	std::string inputString = originalString;
	//int frontDe = inputString.find(delimiterToTrim);
	size_t frontDe = inputString.find_first_of(delimiterToTrim);
	inputString.erase(inputString.begin() + frontDe, inputString.begin() + frontDe + 1);

	size_t backDe = inputString.find_last_of(delimiterToTrim);
	inputString.erase(inputString.begin() + backDe, inputString.begin() + backDe + 1);

	originalString = inputString;
}

