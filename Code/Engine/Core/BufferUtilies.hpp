#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Core/Rgba8.hpp"
#include <vector>
#include <string>
struct Vec2;
struct Vec3;
struct Vec4;
struct IntVec2;
struct IntVec3;
struct Rgba8;
struct AABB2;
struct AABB3;
struct OBB2;
struct Plane2D;
struct Plane3D;
struct Vertex_PCU;
struct Vertex_PCUTBN;

enum eBufferEndian
{
	NATIVE = 0,
	LITTLE_ENDIAN,
	BIG_ENDIAN
};


class BufferWriter //Or serializer 
{
public:
	BufferWriter(std::vector<unsigned char>& buffer);
	void AppendByte(unsigned char byteValue);
	void AppendFloat(float floatValue);
	void AppendChar(char charValue);
	void AppendShort(short shortValue);
	void AppendUnsignedShort(unsigned short uShortValue);
	void AppendInt(int intValue);
	void AppendUnsignedInt(unsigned int uintValue);
	void AppendUint16(uint16_t uint16Value);

	void AppendInt64(int64_t int64Value);
	void AppendUint64(uint64_t uint64Value);
	void AppendDouble(double doubleValue);
	void AppendBool(bool boolValue);
	void AppendRgb(Rgba8 const& rgb);

	void AppendStringWithZeroTerminator(std::string const& stringValue);
	void AppendSizedString(std::string const& stringValue);
	void AppendSizedStringWithLength(std::string const& stringValue, unsigned int length);

	void AppendVec2(Vec2 v);
	void AppendVec3(Vec3 const& v);
	void AppendVec4(Vec4 const& v);
	void AppendIntVec2(IntVec2 intv2);
	void AppendIntVec3(IntVec3 const& intv3);
	void AppendRgba8(Rgba8 const& rgba8);
	void AppendAABB2D(AABB2 const& aabb2);
	void AppendAABB3D(AABB3 const& aabb3);
	void AppendOBB2D(OBB2 const& obb2);
	void AppendPlane2D(Plane2D const& plane2D);
	void AppendPlane3D(Plane3D const& plane3D);
	void AppendVertexPCU(Vertex_PCU const& vertexPCU);
	void AppendVertexPCUTBN(Vertex_PCUTBN const& vertexPCUTBN);

	void AppendString(std::string const& stringValue);

	eBufferEndian GetPlatformNativeEndianMode();
	void SetEndianMode(eBufferEndian mode);
	eBufferEndian GetEndianMode()const { return m_endianMode; }

	void Reverse2BytesInPlace(void* ptrTo16BitWord);
	void Reverse4BytesInPlace(void* ptrTo32BitDWord);
	void Reverse8BytesInPlace(void* ptrTo64BitQWord);

	void OverwriteByte(unsigned char newByte, size_t overWriteStartPosi);
	void OverwirteUint(unsigned int newUint, size_t wiretePosOffset);

public:
	std::vector<unsigned char>& m_buffer;
	eBufferEndian m_endianMode = NATIVE;
	bool m_isCurrentlyWritingOppositeEndiannessOfNative = false;
};

class BufferParser 
{
public:
	BufferParser(unsigned char const* bufferToParse, size_t bufferSize);
	BufferParser(std::vector<unsigned char>const& buffer);

	unsigned char	ParseByte();
	float			ParseFloat();
	char			ParseChar();
	short			ParseShort();
	unsigned short	ParseUnsignedShort();
	int				ParseInt();
	unsigned int	ParseUnsignedInt();
	uint16_t		ParseUint16();
	int64_t			ParseInt64();
	uint64_t		ParseUint64();
	double			ParseDouble();
	bool			ParseBool();

	Vec2 ParseVec2();
	Vec3 ParseVec3();
	IntVec2 ParseIntVec2();
	Rgba8 ParseRgba8();
	Rgba8 ParseRgb();
	AABB2 ParseAABB2();
	Vertex_PCU ParseVertexPCU();
	Vertex_PCUTBN ParseVertexPCUTBN();

	std::string ParseZeroTerminatedString();
	void ParseZeroTerminatedString(std::string& zeroTermString);
	std::string ParseLengthPrecededString(unsigned int length);
	void ParseStringAfter32BitLength(std::string& LengthPrecededString);
	void SetEndianMode(eBufferEndian targetEndianMode);
	eBufferEndian GetPlatformNativeEndianMode();

	void Reverse2BytesInPlace(void* ptrTo16BitWord);
	void Reverse4BytesInPlace(void* ptrTo32BitDWord);
	void Reverse8BytesInPlace(void* ptrTo64BitQWord);

	void JumpToPosition(size_t position);

public:
	unsigned char const* m_bufferStart = nullptr;
	size_t m_currentReadOffsetInBytes = 0;
	size_t m_bufferSizeInBytes = 0;
	bool m_isCurrentlyWritingOppositeEndiannessOfNative = false;
	eBufferEndian m_endianMode = NATIVE;
};