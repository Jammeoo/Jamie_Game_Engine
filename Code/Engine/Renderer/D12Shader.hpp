#pragma once
#include "Engine/Core/EngineCommon.hpp"
#ifdef ENABLE_D3D12
#include "Engine/Renderer/D12CommonHeader.hpp"


struct ShaderType 
{
	enum type :u32
	{
		vertex = 0,
		hull,
		domain,
		geometry,
		pixel,
		compute,
		amplification,
		mesh,

		count
	};
};

struct EngineShader {
	enum id :u32 
	{
		FULLSCREEN_TRIANGLE_VS = 0,
		FILL_COLOR_PS = 1,
		POST_PROCESS_PS = 2,
		UNLIT_VS = 3,
		UNLIT_PS = 4,
		count
	};
};

// typedef struct CompiledShader 
// {
// 	u64 m_size;
// 	const u8* m_byteCode;
// }const *CompiledShaderPtr;

struct CompiledShader
{
	CompiledShader() 
	{
		m_size = 0;
		
	}
	u64 m_size;
	//u8* m_byteCode;
	std::vector<u8> m_byteCode;
	 
}/*const* CompiledShaderPtr*/;

struct ShaderFileInfo 
{
	const char* file;
	const char* function;
	EngineShader::id id;
	ShaderType::type type;
};





class D12Shaders
{
public:
	D12Shaders(const std::string& engineShaderPath);

	bool Startup();
	void Shutdown();


	bool LoadEngineShaders();

	bool CompileShaders();
	D3D12_SHADER_BYTECODE GetEngineShaderByID(EngineShader::id id);
	D3D12_SHADER_BYTECODE GetCompiledShaderByID(EngineShader::id id);

private:

	//each element in this array points to an offset within the shaders blob.
	//CompiledShaderPtr m_engineShaders[EngineShader::count]{};

	std::vector<CompiledShader*> m_engineShaders;
	//std::vector<D3D12_SHADER_BYTECODE*> m_engineShaders;
	std::vector<ComPtr<IDxcBlob>> m_compiledShaders;
	//This is chunk of memory that contains all compiled engine shaders.
	//The blob is an array of shader byte code consisting of a u64 size and
	//an array of bytes
	//std::unique_ptr<u8[]> m_shadersBlob{};
	//std::vector<u8>

	std::string m_engineShadersPath = "Data/Shaders/";//"Engine/Renderer/Shaders/";
	std::string m_shadersName[EngineShader::count]{ "FullScreenTriangle.hlsl", "FillColor.hlsl","PostProcess.hlsl","Unlit.hlsl","Unlit.hlsl" };

	ShaderFileInfo m_shaderFiles[EngineShader::count];


	//std::vector<CompiledShader*> m_compiledShaders;
};



#endif // ENABLE_D3D12