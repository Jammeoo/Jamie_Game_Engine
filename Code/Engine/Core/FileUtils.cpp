#include "Engine/Core/FileUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
int FileReadToBuffer(std::vector <uint8_t>& outBuffer, const std::string& filename)
{
	int is_ok = EXIT_FAILURE;
	FILE* fp;
	size_t fSize;
	errno_t err = fopen_s(&fp, filename.c_str(), "rb");
	if (err)
	{
		return is_ok;
	}
	// obtain file size:
	fseek(fp, 0, SEEK_END);
	fSize = (size_t)ftell(fp);
	outBuffer.resize(fSize);
	rewind(fp);
	size_t result = fread(outBuffer.data(), sizeof(uint8_t), fSize, fp);
	if (ferror(fp)) 
	{
		ERROR_AND_DIE("I / O error when reading");
	}else if (feof(fp))
	{
		is_ok = EXIT_SUCCESS;
	}
	fclose(fp);
	//return is_ok;
	return (int)result;
}

int FileReadToString(std::string& outString, const std::string& filename)
{
	std::vector <uint8_t> vectorToString;
	int result = FileReadToBuffer(vectorToString, filename);
// 	for (int index = 0; index < vectorToString.size(); index++) 
// 	{
// 		outString += std::to_string(vectorToString[index]);
// 	}
	std::string temString = std::string((char*)vectorToString.data(), vectorToString.size());
	outString = temString;
	return result;
}

int FileWriteFromBuffer(std::vector <uint8_t>const& resourcebuffer, const std::string& filename)
{
	int is_ok = EXIT_FAILURE;
	FILE* fp;
	//size_t fSize;
	errno_t err = fopen_s(&fp, filename.c_str(), "wb");
	if (err)
	{
		return is_ok;
	}
	// obtain file size:
	//fseek(fp, 0, SEEK_END);
	//fSize = (size_t)ftell(fp);
	//outBuffer.resize(fSize);
	//rewind(fp);
	size_t result = fwrite(resourcebuffer.data(), sizeof(uint8_t), resourcebuffer.size(), fp);
	if (ferror(fp))
	{
		ERROR_AND_DIE("I / O error when reading");
	}
	else if (feof(fp))
	{
		is_ok = EXIT_SUCCESS;
	}
	fclose(fp);
	return (int)result;
}

bool IsFileExisted(const std::string& filename)
{
	FILE* fp;
	if (fopen_s(&fp, filename.c_str(), "r") == 0) {
		fclose(fp);
		return true;
	}
	else 
	{
		return false;
	}
}
#ifdef ENABLE_D3D12


bool ReadShadersFromFile(const std::string& filePath, std::vector <uint8_t>& byteCodeofThis, u64& size)
{
	bool is_ok = false;
	FILE* fp;
	size_t fSize;
	errno_t err = fopen_s(&fp, filePath.c_str(), "rb");
	if (err)
	{
		return is_ok;
	}
	// obtain file size:
	fseek(fp, 0, SEEK_END);
	fSize = (size_t)ftell(fp);
	rewind(fp);
	
	std::vector <uint8_t> tempBuffer;
	tempBuffer.resize(fSize);
	//uint8_t* tempBuffer = nullptr;
	size_t result = fread(tempBuffer.data(), sizeof(uint8_t), fSize, fp);
	for (int index = 0; index < (int)fSize; index++)
	{
		byteCodeofThis.push_back(tempBuffer[index]);
	}

	size = (u64)fSize;
	if (ferror(fp))
	{
		ERROR_AND_DIE("I / O error when reading");
	}
	else if (feof(fp))
	{
		is_ok = true;
	}
	if (result != fSize) 
	{
		is_ok = false;
		ERROR_AND_DIE("Reading error");
	}
	fclose(fp);
	is_ok = true;
	return is_ok;
}
#endif // ENABLE_D3D12