#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include <string>
#include <vector>
#include "Engine/Renderer/D12CommonHeader.hpp"
int FileReadToBuffer(std::vector <uint8_t>& outBuffer, const std::string& filename);
int FileReadToString(std::string& outString, const std::string& filename);
int FileWriteFromBuffer(std::vector <unsigned char>const& resourcebuffer, const std::string& filename);
bool IsFileExisted(const std::string& filename);

#ifdef ENABLE_D3D12

bool ReadShadersFromFile(const std::string& filePath, std::vector <u8>& shadersBlob, u64& size);
#endif // ENABLE_D3D12