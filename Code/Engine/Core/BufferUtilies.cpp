#include "Engine/Core/BufferUtilies.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Vec4.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/IntVec3.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/OBB2.hpp"
#include "Engine/Math/Plane2D.hpp"
#include "Engine/Math/Plane3D.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/Vertex_PCUTBN.hpp"


BufferWriter::BufferWriter(std::vector<unsigned char>& buffer)
	: m_buffer(buffer)
{

}

void BufferWriter::AppendByte(unsigned char byteValue)
{
	m_buffer.push_back(byteValue);
}

void BufferWriter::AppendFloat(float floatValue)
{
	//Float is 4 bytes sized, four time of append byte
	size_t floatSize = sizeof(float);
	GUARANTEE_OR_DIE(floatSize == 4, "");

	float* addressOfFloatF = &floatValue;
	unsigned char* addressOfFloatAsByteArray = reinterpret_cast<unsigned char*>(addressOfFloatF);
	//zero code reinterpret_cast<unsigned char*>()
	//now (unsigned char*) C style cast will be treated as a reinterpret_cast
	//Somethings, C style cast will be treated as static_cast 
	if (m_isCurrentlyWritingOppositeEndiannessOfNative) 
	{
		Reverse4BytesInPlace(addressOfFloatAsByteArray);
	}
	AppendByte(addressOfFloatAsByteArray[0]);
	AppendByte(addressOfFloatAsByteArray[1]);
	AppendByte(addressOfFloatAsByteArray[2]);
	AppendByte(addressOfFloatAsByteArray[3]);
}

void BufferWriter::AppendChar(char charValue)
{
	size_t charSize = sizeof(char);
	GUARANTEE_OR_DIE((charSize == 1), "");

	char* addressOfChar = &charValue;
	unsigned char* addressOfCharAsByteArray = reinterpret_cast<unsigned char*>(addressOfChar);

	AppendByte(addressOfCharAsByteArray[0]);
}

void BufferWriter::AppendShort(short shortValue)
{
	size_t shortSize = sizeof(short);
	GUARANTEE_OR_DIE(shortSize == 2, "");

	short* addressOfShortS = &shortValue;
	unsigned char* addressOfShortAsByteArray = reinterpret_cast<unsigned char*>(addressOfShortS);

	if (m_isCurrentlyWritingOppositeEndiannessOfNative)
	{
		Reverse2BytesInPlace(addressOfShortAsByteArray);
	}
	AppendByte(addressOfShortAsByteArray[0]);
	AppendByte(addressOfShortAsByteArray[1]);
}

void BufferWriter::AppendUnsignedShort(unsigned short uShortValue)
{
	size_t ushortSize = sizeof(unsigned short);
	GUARANTEE_OR_DIE(ushortSize == 2, "");

	unsigned short* addressOfUShortS = &uShortValue;
	unsigned char* addressOfUShortAsByteArray = reinterpret_cast<unsigned char*>(addressOfUShortS);
	if (m_isCurrentlyWritingOppositeEndiannessOfNative)
	{
		Reverse2BytesInPlace(addressOfUShortAsByteArray);
	}
	AppendByte(addressOfUShortAsByteArray[0]);
	AppendByte(addressOfUShortAsByteArray[1]);
}

void BufferWriter::AppendInt(int intValue)
{
	size_t intSize = sizeof(int);
	GUARANTEE_OR_DIE((intSize == 4), "");
	int* addressOfInt = &intValue;
	unsigned char* addressOfIntAsByteArray = reinterpret_cast<unsigned char*>(addressOfInt);
	if (m_isCurrentlyWritingOppositeEndiannessOfNative)
	{
		Reverse4BytesInPlace(addressOfIntAsByteArray);
	}
	AppendByte(addressOfIntAsByteArray[0]);
	AppendByte(addressOfIntAsByteArray[1]);
	AppendByte(addressOfIntAsByteArray[2]);
	AppendByte(addressOfIntAsByteArray[3]);
}

void BufferWriter::AppendUnsignedInt(unsigned int uintValue)
{
	size_t uintSize = sizeof(unsigned int);
	GUARANTEE_OR_DIE((uintSize == 4), "");
	unsigned int* addressOfUint = &uintValue;
	unsigned char* addressOfUintAsByteArray = reinterpret_cast<unsigned char*>(addressOfUint);
	if (m_isCurrentlyWritingOppositeEndiannessOfNative)
	{
		Reverse4BytesInPlace(addressOfUintAsByteArray);
	}
	AppendByte(addressOfUintAsByteArray[0]);
	AppendByte(addressOfUintAsByteArray[1]);
	AppendByte(addressOfUintAsByteArray[2]);
	AppendByte(addressOfUintAsByteArray[3]);
}

void BufferWriter::AppendInt64(int64_t int64Value)
{
	size_t int64Size = sizeof(int64_t);
	GUARANTEE_OR_DIE((int64Size == 8), "");
	int64_t* addressOfInt64 = &int64Value;
	unsigned char* addressOfInt64AsByteArray = reinterpret_cast<unsigned char*>(addressOfInt64);

	if (m_isCurrentlyWritingOppositeEndiannessOfNative)
	{
		Reverse8BytesInPlace(addressOfInt64AsByteArray);
	}

	AppendByte(addressOfInt64AsByteArray[0]);
	AppendByte(addressOfInt64AsByteArray[1]);
	AppendByte(addressOfInt64AsByteArray[2]);
	AppendByte(addressOfInt64AsByteArray[3]);
	AppendByte(addressOfInt64AsByteArray[4]);
	AppendByte(addressOfInt64AsByteArray[5]);
	AppendByte(addressOfInt64AsByteArray[6]);
	AppendByte(addressOfInt64AsByteArray[7]);
}

void BufferWriter::AppendUint64(uint64_t uint64Value)
{
	size_t uint64Size = sizeof(int64_t);
	GUARANTEE_OR_DIE((uint64Size == 8), "");
	uint64_t* addressOfUint64 = &uint64Value;
	unsigned char* addressOfUint64AsByteArray = reinterpret_cast<unsigned char*>(addressOfUint64);
	if (m_isCurrentlyWritingOppositeEndiannessOfNative)
	{
		Reverse8BytesInPlace(addressOfUint64AsByteArray);
	}
	AppendByte(addressOfUint64AsByteArray[0]);
	AppendByte(addressOfUint64AsByteArray[1]);
	AppendByte(addressOfUint64AsByteArray[2]);
	AppendByte(addressOfUint64AsByteArray[3]);
	AppendByte(addressOfUint64AsByteArray[4]);
	AppendByte(addressOfUint64AsByteArray[5]);
	AppendByte(addressOfUint64AsByteArray[6]);
	AppendByte(addressOfUint64AsByteArray[7]);
}

void BufferWriter::AppendDouble(double doubleValue)
{
	size_t doubleSize = sizeof(double);
	GUARANTEE_OR_DIE((doubleSize == 8), "");
	double* addressOfDouble = &doubleValue;
	unsigned char* addressOfDoubleAsByteArray = reinterpret_cast<unsigned char*>(addressOfDouble);
	if (m_isCurrentlyWritingOppositeEndiannessOfNative)
	{
		Reverse8BytesInPlace(addressOfDoubleAsByteArray);
	}
	AppendByte(addressOfDoubleAsByteArray[0]);
	AppendByte(addressOfDoubleAsByteArray[1]);
	AppendByte(addressOfDoubleAsByteArray[2]);
	AppendByte(addressOfDoubleAsByteArray[3]);
	AppendByte(addressOfDoubleAsByteArray[4]);
	AppendByte(addressOfDoubleAsByteArray[5]);
	AppendByte(addressOfDoubleAsByteArray[6]);
	AppendByte(addressOfDoubleAsByteArray[7]);
}

void BufferWriter::AppendBool(bool boolValue)
{
	unsigned char boolByte = '0';
	if (boolValue == true) 
	{
		boolByte = '1';
	}
	if (boolValue == false) 
	{
		boolByte = '0';
	}
	GUARANTEE_OR_DIE((boolByte == '0' || boolByte == '1'), "InValid Bool Value");

	AppendByte(boolByte);
}

void BufferWriter::AppendStringWithZeroTerminator(std::string const& stringValue)
{
	int size = (int)stringValue.size();
	for (int charIndex = 0; charIndex < size; charIndex++) 
	{
		unsigned char byteValue = stringValue[charIndex];
		AppendByte(byteValue);
	}
	AppendByte('\0');
}

void BufferWriter::AppendSizedString(std::string const& stringValue)
{
	size_t length = stringValue.length();
	AppendUnsignedInt((unsigned int)length);
	for (int charIndex = 0; charIndex < (int)length; charIndex++)
	{
		unsigned char byteValue = stringValue[charIndex];
		AppendByte(byteValue);
	}
}

void BufferWriter::AppendVec2(Vec2 v)
{
	AppendFloat(v.x);
	AppendFloat(v.y);
}

void BufferWriter::AppendVec3(Vec3 const& v)
{
	AppendFloat(v.x);
	AppendFloat(v.y);
	AppendFloat(v.z);
}

void BufferWriter::AppendVec4(Vec4 const& v)
{
	AppendFloat(v.x);
	AppendFloat(v.y);
	AppendFloat(v.z);
	AppendFloat(v.w);
}

void BufferWriter::AppendIntVec2(IntVec2 intv2)
{
	AppendInt(intv2.x);
	AppendInt(intv2.y);
}

void BufferWriter::AppendIntVec3(IntVec3 const& intv3)
{
	AppendInt(intv3.x);
	AppendInt(intv3.y);
	AppendInt(intv3.z);
}

void BufferWriter::AppendRgba8(Rgba8 const& rgba8)
{
	AppendByte(rgba8.r);
	AppendByte(rgba8.g);
	AppendByte(rgba8.b);
	AppendByte(rgba8.a);
}

void BufferWriter::AppendAABB2D(AABB2 const& aabb2)
{
	AppendVec2(aabb2.m_mins);
	AppendVec2(aabb2.m_maxs);
}

void BufferWriter::AppendAABB3D(AABB3 const& aabb3)
{
	AppendVec3(aabb3.m_mins);
	AppendVec3(aabb3.m_maxs);
}

void BufferWriter::AppendOBB2D(OBB2 const& obb2)
{
	AppendVec2(obb2.m_center);
	AppendVec2(obb2.m_iBasisNormal);
	AppendVec2(obb2.m_halfDimensions);
}

void BufferWriter::AppendPlane2D(Plane2D const& plane2D)
{
	AppendVec2(plane2D.m_normal);
	AppendFloat(plane2D.m_distFromOrigin);
}

void BufferWriter::AppendPlane3D(Plane3D const& plane3D)
{
	AppendVec3(plane3D.m_normal);
	AppendFloat(plane3D.m_distFromOrigin);
}

void BufferWriter::AppendVertexPCU(Vertex_PCU const& vertexPCU)
{
	AppendVec3(vertexPCU.m_position);
	AppendRgba8(vertexPCU.m_color);
	AppendVec2(vertexPCU.m_uvTexCoords);
}

void BufferWriter::AppendVertexPCUTBN(Vertex_PCUTBN const& vertexPCUTBN)
{
	AppendVec3(vertexPCUTBN.m_position);
	AppendRgba8(vertexPCUTBN.m_color);
	AppendVec2(vertexPCUTBN.m_uvTexCoords);
	AppendVec3(vertexPCUTBN.m_tangent);
	AppendVec3(vertexPCUTBN.m_binormal);
	AppendVec3(vertexPCUTBN.m_normal);
}

void BufferWriter::OverwriteByte(unsigned char newByte, size_t overWriteStartPosi)
{
	GUARANTEE_OR_DIE((overWriteStartPosi + 1 < m_buffer.size()), "The Start Position is out of boundary!");

	m_buffer[overWriteStartPosi] = newByte;

}

void BufferWriter::OverwirteUint(unsigned int newUint, size_t writePosOffset)//wiretePosOffset
{
	//unsigned int* addressOfUint = &newUint;
	//unsigned char* addressOfUintAsByteArray = reinterpret_cast<unsigned char*>(addressOfUint);
	//OverwriteByte(addressOfUintAsByteArray[0], wiretePosOffset + 0);
	//OverwriteByte(addressOfUintAsByteArray[1], wiretePosOffset + 1);
	//OverwriteByte(addressOfUintAsByteArray[2], wiretePosOffset + 2);
	//OverwriteByte(addressOfUintAsByteArray[3], wiretePosOffset + 3);


	unsigned char* bufferStart = m_buffer.data();
	unsigned int* writePosition = (unsigned int*)(bufferStart + writePosOffset);
	if (m_isCurrentlyWritingOppositeEndiannessOfNative) 
	{
		Reverse4BytesInPlace(&newUint);
	}
	*writePosition = newUint;

}

// void ConvexScene2d_parseAndValidate4CC(BufferParser& bufParse, char* fourCharacter4CC) 
// {
// 
// }
inline void BufferWriter::Reverse2BytesInPlace(void* ptrTo16BitWord)
{
	unsigned int* asUint16Ptr = reinterpret_cast<unsigned int*>(ptrTo16BitWord);
	unsigned int orignalUint16 = *asUint16Ptr;
	unsigned int reversedUint16 = ((orignalUint16 & 0x00ff) << 8 |
									(orignalUint16 & 0xff00 >> 8));

	*asUint16Ptr = reversedUint16;
}

eBufferEndian BufferWriter::GetPlatformNativeEndianMode()
{
	unsigned int testInt = 0x12345678;
	unsigned char* testChar = reinterpret_cast<unsigned char*>(&testInt);
	eBufferEndian mode = NATIVE;
	if (testChar[0] == 0x78)
	{
		mode = LITTLE_ENDIAN;
	}
	else if (testChar[0] == 0x12)
	{
		mode = BIG_ENDIAN;
	}
	else
	{
		ERROR_AND_DIE("Count not identify ")
	}
	return mode;
}
  
void BufferWriter::SetEndianMode(eBufferEndian mode)
{
	eBufferEndian nativeMode = GetPlatformNativeEndianMode();
	m_endianMode = mode;
	m_isCurrentlyWritingOppositeEndiannessOfNative = (nativeMode != mode);
}

void BufferWriter::Reverse4BytesInPlace(void* ptrTo32BitDWord)
{
	unsigned int* asUint32Ptr = reinterpret_cast<unsigned int*>(ptrTo32BitDWord);
	unsigned int orignalUint32 = *asUint32Ptr;
	unsigned int reversedUint32 =	((orignalUint32 & 0x000000ff) << 24	| 
									((orignalUint32 & 0x0000ff00) << 8)	|
									((orignalUint32 & 0x00ff0000) >> 8)	|
									((orignalUint32 & 0xff000000) >> 24));

	*asUint32Ptr = reversedUint32;
}

void BufferWriter::Reverse8BytesInPlace(void* ptrTo64BitQWord)
{
	uint64_t* asUint64Ptr = reinterpret_cast<uint64_t*>(ptrTo64BitQWord);
	uint64_t orignalUint64 = *asUint64Ptr;

	uint64_t reversedUint64 = (
		(orignalUint64 & 0x00000000000000ff) << 56 | (orignalUint64 & 0xff00000000000000 >> 56) |
		(orignalUint64 & 0x000000000000ff00) << 40 | (orignalUint64 & 0x00ff000000000000 >> 40) |
		(orignalUint64 & 0x0000000000ff0000) << 24 | (orignalUint64 & 0x0000ff0000000000 >> 24) |
		(orignalUint64 & 0x00000000ff000000) << 8  | (orignalUint64 & 0x000000ff00000000 >> 8));
	*asUint64Ptr = reversedUint64;

}

void BufferWriter::AppendString(std::string const& stringValue)
{
	unsigned int size = (unsigned int)stringValue.length();//Not include '0\'
	for (unsigned int charIndex = 0; charIndex < size; charIndex++) 
	{
		unsigned char byteValue = stringValue[charIndex];
		m_buffer.push_back(byteValue);
	}
	m_buffer.push_back('\0');
}

void BufferWriter::AppendSizedStringWithLength(std::string const& stringValue, unsigned int length)
{
	for (int charIndex = 0; charIndex < (int)length; charIndex++)
	{
		unsigned char byteValue = stringValue[charIndex];
		AppendByte(byteValue);
	}
}

void BufferWriter::AppendRgb(Rgba8 const& rgb)
{
	AppendByte(rgb.r);
	AppendByte(rgb.g);
	AppendByte(rgb.b);
}

void BufferWriter::AppendUint16(uint16_t uint16Value)
{
	size_t uint16Size = sizeof(uint16_t);
	GUARANTEE_OR_DIE((uint16Size == 2), "");
	uint16_t* addressOfUint16 = &uint16Value;
	unsigned char* addressOfUint16AsByteArray = reinterpret_cast<unsigned char*>(addressOfUint16);
	if (m_isCurrentlyWritingOppositeEndiannessOfNative)
	{
		Reverse2BytesInPlace(addressOfUint16AsByteArray);
	}
	AppendByte(addressOfUint16AsByteArray[0]);
	AppendByte(addressOfUint16AsByteArray[1]);
}

BufferParser::BufferParser(unsigned char const* bufferToParse, size_t bufferSize)
	:m_bufferStart(bufferToParse), m_bufferSizeInBytes(bufferSize) 
{

}

BufferParser::BufferParser(std::vector<unsigned char>const& buffer)
	:m_bufferStart(buffer.data()), m_bufferSizeInBytes(buffer.size())
{

}

unsigned char BufferParser::ParseByte()
{
	return m_bufferStart[m_currentReadOffsetInBytes++];
	//Same with following code

	//unsigned char byteValue = m_bufferStart[m_bufferCurrentReadInBytes];
	//++m_bufferCurrentReadInBytes; 
	//return byteValue;
}

float BufferParser::ParseFloat()
{
	//Check to make sure there are at least 4 4 bytes left in buffer to parse!
	GUARANTEE_OR_DIE((m_currentReadOffsetInBytes + sizeof(float) <= m_bufferSizeInBytes), "Reading gets out of bound!");

	unsigned char const* memoryAddressOfBytesInBuffer = &m_bufferStart[m_currentReadOffsetInBytes];
	float const* memoryAddressOfFloatInBuffer = reinterpret_cast<float const*>(memoryAddressOfBytesInBuffer);
	m_currentReadOffsetInBytes += sizeof(float);

	if (m_isCurrentlyWritingOppositeEndiannessOfNative)
	{
		Reverse4BytesInPlace((void*)memoryAddressOfFloatInBuffer);
	}

	return *memoryAddressOfFloatInBuffer;


	//float finalValue;
	//float* addressOfFinalValue = &finalValue;
	//unsigned char* floatAsArrayOfByte = reinterpret_cast<unsigned char*>(addressOfFinalValue);
	//floatAsArrayOfByte[0] = ParseByte();
	//floatAsArrayOfByte[1] = ParseByte();
	//floatAsArrayOfByte[2] = ParseByte();
	//floatAsArrayOfByte[3] = ParseByte();
	//if (m_isCurrentlyWritingOppositeEndiannessOfNative)
	//{
	//	Reverse4BytesInPlace(&finalValue);
	//}
}

char BufferParser::ParseChar()
{
	GUARANTEE_OR_DIE((m_currentReadOffsetInBytes + sizeof(char) <= m_bufferSizeInBytes), "Reading gets out of bound!");

	unsigned char const* memoryAddressOfBytesInBuffer = &m_bufferStart[m_currentReadOffsetInBytes];
	char const* memoryAddressOfCharInBuffer = reinterpret_cast<char const*>(memoryAddressOfBytesInBuffer);
	m_currentReadOffsetInBytes += sizeof(char);
	return *memoryAddressOfCharInBuffer;
}

short BufferParser::ParseShort()
{
	GUARANTEE_OR_DIE((m_currentReadOffsetInBytes + sizeof(short) <= m_bufferSizeInBytes), "Reading gets out of bound!");

	unsigned char const* memoryAddressOfBytesInBuffer = &m_bufferStart[m_currentReadOffsetInBytes];
	short const* memoryAddressOfShortInBuffer = reinterpret_cast<short const*>(memoryAddressOfBytesInBuffer);
	m_currentReadOffsetInBytes += sizeof(short);

	if (m_isCurrentlyWritingOppositeEndiannessOfNative)
	{
		Reverse2BytesInPlace((void*)memoryAddressOfShortInBuffer);
	}
	return *memoryAddressOfShortInBuffer;
}

unsigned short BufferParser::ParseUnsignedShort()
{
	GUARANTEE_OR_DIE((m_currentReadOffsetInBytes + sizeof(unsigned short) <= m_bufferSizeInBytes), "Reading gets out of bound!");

	unsigned char const* memoryAddressOfBytesInBuffer = &m_bufferStart[m_currentReadOffsetInBytes];
	unsigned short const* memoryAddressOfUshortInBuffer = reinterpret_cast<unsigned short const*>(memoryAddressOfBytesInBuffer);
	m_currentReadOffsetInBytes += sizeof(unsigned short);
	if (m_isCurrentlyWritingOppositeEndiannessOfNative)
	{
		Reverse2BytesInPlace((void*)memoryAddressOfUshortInBuffer);
	}

	return *memoryAddressOfUshortInBuffer;
}

int BufferParser::ParseInt()
{
	GUARANTEE_OR_DIE((m_currentReadOffsetInBytes + sizeof(int) <= m_bufferSizeInBytes), "Reading gets out of bound!");

	unsigned char const* memoryAddressOfBytesInBuffer = &m_bufferStart[m_currentReadOffsetInBytes];
	int const* memoryAddressOfIntInBuffer = reinterpret_cast<int const*>(memoryAddressOfBytesInBuffer);
	m_currentReadOffsetInBytes += sizeof(int);

	if (m_isCurrentlyWritingOppositeEndiannessOfNative)
	{
		Reverse4BytesInPlace((void*)memoryAddressOfIntInBuffer);
	}
	return *memoryAddressOfIntInBuffer;
}

unsigned int BufferParser::ParseUnsignedInt()
{
	GUARANTEE_OR_DIE((m_currentReadOffsetInBytes + sizeof(unsigned int) <= m_bufferSizeInBytes), "Reading gets out of bound!");

	unsigned char const* memoryAddressOfBytesInBuffer = &m_bufferStart[m_currentReadOffsetInBytes];
	unsigned int const* memoryAddressOfUintInBuffer = reinterpret_cast<unsigned int const*>(memoryAddressOfBytesInBuffer);
	m_currentReadOffsetInBytes += sizeof(unsigned int);

	if (m_isCurrentlyWritingOppositeEndiannessOfNative)
	{
		Reverse4BytesInPlace((void*)memoryAddressOfUintInBuffer);
	}
	return *memoryAddressOfUintInBuffer;
}



uint16_t BufferParser::ParseUint16()
{
	GUARANTEE_OR_DIE((m_currentReadOffsetInBytes + sizeof(uint16_t) <= m_bufferSizeInBytes), "Reading gets out of bound!");

	unsigned char const* memoryAddressOfBytesInBuffer = &m_bufferStart[m_currentReadOffsetInBytes];
	uint16_t const* memoryAddressOfUint16InBuffer = reinterpret_cast<uint16_t const*>(memoryAddressOfBytesInBuffer);
	m_currentReadOffsetInBytes += sizeof(uint16_t);

	if (m_isCurrentlyWritingOppositeEndiannessOfNative)
	{
		Reverse2BytesInPlace((void*)memoryAddressOfUint16InBuffer);
	}
	return *memoryAddressOfUint16InBuffer;
}

int64_t BufferParser::ParseInt64()
{
	GUARANTEE_OR_DIE((m_currentReadOffsetInBytes + sizeof(int64_t) <= m_bufferSizeInBytes), "Reading gets out of bound!");

	unsigned char const* memoryAddressOfBytesInBuffer = &m_bufferStart[m_currentReadOffsetInBytes];
	int64_t const* memoryAddressOfInt64InBuffer = reinterpret_cast<int64_t const*>(memoryAddressOfBytesInBuffer);
	m_currentReadOffsetInBytes += sizeof(int64_t);

	if (m_isCurrentlyWritingOppositeEndiannessOfNative)
	{
		Reverse8BytesInPlace((void*)memoryAddressOfInt64InBuffer);
	}
	return *memoryAddressOfInt64InBuffer;
}

uint64_t BufferParser::ParseUint64()
{
	GUARANTEE_OR_DIE((m_currentReadOffsetInBytes + sizeof(uint64_t) <= m_bufferSizeInBytes), "Reading gets out of bound!");

	unsigned char const* memoryAddressOfBytesInBuffer = &m_bufferStart[m_currentReadOffsetInBytes];
	uint64_t const* memoryAddressOfUint64InBuffer = reinterpret_cast<uint64_t const*>(memoryAddressOfBytesInBuffer);
	m_currentReadOffsetInBytes += sizeof(uint64_t);

	if (m_isCurrentlyWritingOppositeEndiannessOfNative)
	{
		Reverse8BytesInPlace((void*)memoryAddressOfUint64InBuffer);
	}

	return *memoryAddressOfUint64InBuffer;
}

double BufferParser::ParseDouble()
{
	GUARANTEE_OR_DIE((m_currentReadOffsetInBytes + sizeof(double) <= m_bufferSizeInBytes), "Reading gets out of bound!");

	unsigned char const* memoryAddressOfBytesInBuffer = &m_bufferStart[m_currentReadOffsetInBytes];
	double const* memoryAddressOfDoubleInBuffer = reinterpret_cast<double const*>(memoryAddressOfBytesInBuffer);
	m_currentReadOffsetInBytes += sizeof(double);
	if (m_isCurrentlyWritingOppositeEndiannessOfNative)
	{
		Reverse8BytesInPlace((void*)memoryAddressOfDoubleInBuffer);
	}

	return *memoryAddressOfDoubleInBuffer;
}

bool BufferParser::ParseBool()
{
	GUARANTEE_OR_DIE((m_currentReadOffsetInBytes + sizeof(unsigned char) <= m_bufferSizeInBytes), "Reading gets out of bound!");

	unsigned char const* memoryAddressOfBytesInBuffer = &m_bufferStart[m_currentReadOffsetInBytes];
	
	m_currentReadOffsetInBytes += sizeof(unsigned char);
	if (*memoryAddressOfBytesInBuffer == '1')
	{
		return true;
	}
	if (*memoryAddressOfBytesInBuffer == '0')
	{
		return false;
	}
	ERROR_AND_DIE("Parse sesult is not 1/0! Bool parse falied!")
}

Vec2 BufferParser::ParseVec2()
{
	float x = ParseFloat();
	float y = ParseFloat();
	return Vec2(x, y);
}

Vec3 BufferParser::ParseVec3()
{
	float x = ParseFloat();
	float y = ParseFloat();
	float z = ParseFloat();
	return Vec3(x, y, z);
}

IntVec2 BufferParser::ParseIntVec2()
{
	int x = ParseInt();
	int y = ParseInt();
	return IntVec2(x, y);
}

Rgba8 BufferParser::ParseRgba8()
{
	unsigned char r = ParseByte();
	unsigned char g = ParseByte();
	unsigned char b = ParseByte();
	unsigned char a = ParseByte();
	return Rgba8(r, g, b, a);
}

Rgba8 BufferParser::ParseRgb()
{
	unsigned char r = ParseByte();
	unsigned char g = ParseByte();
	unsigned char b = ParseByte();
	return Rgba8(r, g, b, 255);
}

AABB2 BufferParser::ParseAABB2()
{
	float minX = ParseFloat();
	float minY = ParseFloat();
	float maxX = ParseFloat();
	float maxY = ParseFloat();
	return AABB2(minX, minY, maxX, maxY);
}

Vertex_PCU BufferParser::ParseVertexPCU()
{
	Vec3 posi = ParseVec3();
	Rgba8 color = ParseRgba8();
	Vec2 uv = ParseVec2();
	return Vertex_PCU(posi, color, uv);
}

Vertex_PCUTBN BufferParser::ParseVertexPCUTBN()
{
	Vec3 posi = ParseVec3();
	Rgba8 color = ParseRgba8();
	Vec2 uv = ParseVec2();
	Vec3 tangent = ParseVec3();
	Vec3 binormal = ParseVec3();
	Vec3 normal = ParseVec3();
	return Vertex_PCUTBN(posi, color, uv, binormal, tangent, normal);
}

std::string BufferParser::ParseZeroTerminatedString()
{
	std::string resultString;
	unsigned char const* memoryAddressOfStringInBuffer = &m_bufferStart[m_currentReadOffsetInBytes];

	unsigned char currentChar = '0';
	while (currentChar != '\0')
	{
		currentChar =  memoryAddressOfStringInBuffer[m_currentReadOffsetInBytes++];
		resultString.push_back((char)currentChar);
	}
	//resultString.push_back('\0');
	return resultString;
}

void BufferParser::ParseZeroTerminatedString(std::string& zeroTermString)
{
	
	unsigned char const* memoryAddressOfStringInBuffer = &m_bufferStart[m_currentReadOffsetInBytes];

	unsigned char currentChar = '0';
	unsigned int index = 0;
	while (currentChar != '\0')
	{
		currentChar = memoryAddressOfStringInBuffer[index++];
		m_currentReadOffsetInBytes++;
		if (currentChar == '\0')
		{
			break;
		}
		zeroTermString.push_back((char)currentChar);
	}
}

std::string BufferParser::ParseLengthPrecededString(unsigned int length)
{
	GUARANTEE_OR_DIE((m_currentReadOffsetInBytes + length <= m_bufferSizeInBytes), "Reading gets out of bound!");
	std::string resultString;
	unsigned char const* memoryAddressOfStringInBuffer = &m_bufferStart[m_currentReadOffsetInBytes];
	unsigned char currentChar = '0';
	for (unsigned int charIndex = 0; charIndex < length; charIndex++) 
	{
		currentChar = memoryAddressOfStringInBuffer[m_currentReadOffsetInBytes++];
		resultString.push_back((char)currentChar);
	}
	return resultString;
}

void BufferParser::ParseStringAfter32BitLength(std::string& LengthPrecededString)
{
	unsigned int uint32Length = ParseUnsignedInt();

	GUARANTEE_OR_DIE((m_currentReadOffsetInBytes + uint32Length <= m_bufferSizeInBytes), "Reading gets out of bound!");
	std::string resultString;
	unsigned char const* memoryAddressOfStringInBuffer = &m_bufferStart[m_currentReadOffsetInBytes];
	unsigned char currentChar = '0';
	for (unsigned int charIndex = 0; charIndex < uint32Length; charIndex++)
	{
		currentChar = memoryAddressOfStringInBuffer[charIndex];
		m_currentReadOffsetInBytes++;
		resultString.push_back((char)currentChar);
	}
	LengthPrecededString = resultString;
}

void BufferParser::SetEndianMode(eBufferEndian targetEndianMode)
{
	eBufferEndian mode = GetPlatformNativeEndianMode();
	m_endianMode = mode;
	m_isCurrentlyWritingOppositeEndiannessOfNative = (m_endianMode != mode);
}

eBufferEndian BufferParser::GetPlatformNativeEndianMode()
{
	unsigned int testInt = 0x12345678;
	unsigned char* testChar = reinterpret_cast<unsigned char*>(&testInt);
	eBufferEndian mode = NATIVE;
	if (testChar[0] == 0x78)
	{
		mode = LITTLE_ENDIAN;
	}
	else if (testChar[0] == 0x12)
	{
		mode = BIG_ENDIAN;
	}
	else
	{
		ERROR_AND_DIE("Count not identify ")
	}
	return mode;
}

void BufferParser::Reverse2BytesInPlace(void* ptrTo16BitWord)
{
	unsigned int* asUint16Ptr = reinterpret_cast<unsigned int*>(ptrTo16BitWord);
	unsigned int orignalUint16 = *asUint16Ptr;
	unsigned int reversedUint16 = ((orignalUint16 & 0x00ff) << 8 |
		(orignalUint16 & 0xff00 >> 8));

	*asUint16Ptr = reversedUint16;
}

void BufferParser::Reverse4BytesInPlace(void* ptrTo32BitDWord)
{
	unsigned int* asUint32Ptr = reinterpret_cast<unsigned int*>(ptrTo32BitDWord);
	unsigned int orignalUint32 = *asUint32Ptr;
	unsigned int reversedUint32 = ((orignalUint32 & 0x000000ff) << 24 |
		((orignalUint32 & 0x0000ff00) << 8) |
		((orignalUint32 & 0x00ff0000) >> 8) |
		((orignalUint32 & 0xff000000) >> 24));

	*asUint32Ptr = reversedUint32;
}

void BufferParser::Reverse8BytesInPlace(void* ptrTo64BitQWord)
{
	int64_t* asUint64Ptr = reinterpret_cast<int64_t*>(ptrTo64BitQWord);
	int64_t orignalUint64 = *asUint64Ptr;

	int64_t reversedUint64 = (
		(orignalUint64 & 0x00000000000000ff) << 56 | (orignalUint64 & 0xff00000000000000 >> 56) |
		(orignalUint64 & 0x000000000000ff00) << 40 | (orignalUint64 & 0x00ff000000000000 >> 40) |
		(orignalUint64 & 0x0000000000ff0000) << 24 | (orignalUint64 & 0x0000ff0000000000 >> 24) |
		(orignalUint64 & 0x00000000ff000000) << 8 | (orignalUint64 & 0x000000ff00000000 >> 8));
	*asUint64Ptr = reversedUint64;
}

void BufferParser::JumpToPosition(size_t position)
{
	m_currentReadOffsetInBytes = position;
}
