#include "Engine/Renderer/D12Shader.hpp"

#ifdef ENABLE_D3D12

#include "Engine/Core/FileUtils.hpp"
#include "Engine/Renderer/D12ShaderCompilation.hpp"


D12Shaders::D12Shaders(const std::string& engineShaderPath):m_engineShadersPath(engineShaderPath)
{
	m_shaderFiles[0] = { "FullScreenTriangle.hlsl","FullScreenTriangleVS",EngineShader::FULLSCREEN_TRIANGLE_VS,ShaderType::vertex };
	m_shaderFiles[1] = { "FillColor.hlsl","FillColorPS",EngineShader::FILL_COLOR_PS,ShaderType::pixel };
	m_shaderFiles[2] = { "PostProcess.hlsl","PostProcessPS",EngineShader::POST_PROCESS_PS,ShaderType::pixel };
	m_shaderFiles[3] = { "Unlit.hlsl","VertexMain",EngineShader::UNLIT_VS,ShaderType::vertex };
	m_shaderFiles[4] = { "Unlit.hlsl","PixelMain",EngineShader::UNLIT_PS,ShaderType::pixel };
}

bool D12Shaders::Startup()
{
	return LoadEngineShaders();
}

void D12Shaders::Shutdown()
{
	m_engineShaders.clear();
}



bool D12Shaders::LoadEngineShaders()
{
	for (int index = 0; index < EngineShader::count; ++index)
	{
		CompiledShader* shader = new CompiledShader();
		std::string currentShaderPath = m_engineShadersPath + m_shadersName[index];
		int result = ReadShadersFromFile(currentShaderPath, shader->m_byteCode, shader->m_size);
		m_engineShaders.push_back(shader);
		if (result == false) return false;
	}
	return true;
}


bool D12Shaders::CompileShaders()
{
	//std::vector<ComPtr<IDxcBlob>> shaders;
	ShaderCompiler* compiler = new ShaderCompiler();
	for (u32 index = 0; index < (u32)m_engineShaders.size(); ++index) 
	{
		ShaderFileInfo& info = m_shaderFiles[index];
		std::string currentShaderPath = m_engineShadersPath + m_shadersName[index];
		ComPtr<IDxcBlob> compiledShader = compiler->CompileThis(info, currentShaderPath);

		if (compiledShader != nullptr && compiledShader->GetBufferPointer() && compiledShader->GetBufferSize()) 
		{
			m_compiledShaders.emplace_back(std::move(compiledShader));
			//CompiledShader* saveCompiledShader = new CompiledShader();
			//saveCompiledShader->m_size = compiledShader->GetBufferSize();
			//
			//D3D12_SHADER_BYTECODE byteCode = { compiledShader->GetBufferPointer(),compiledShader->GetBufferSize() };
			//char* firstElement = (char*)byteCode.pShaderBytecode;
			//std::vector<u8> thisByteCode(firstElement, firstElement + byteCode.BytecodeLength);
			//saveCompiledShader->m_byteCode = thisByteCode;
			//m_engineShaders.emplace_back(std::move(saveCompiledShader));

		}
		else
		{
			return false;
		}
	}
	return true;
}

D3D12_SHADER_BYTECODE D12Shaders::GetEngineShaderByID(EngineShader::id id)
{
	assert(id < EngineShader::count);
	CompiledShader* shader = m_engineShaders[id];
	assert(shader->m_byteCode.data() && shader->m_size);
	D3D12_SHADER_BYTECODE result{};
	result.pShaderBytecode = shader->m_byteCode.data();
	result.BytecodeLength = shader->m_size;
	return result;
}

D3D12_SHADER_BYTECODE D12Shaders::GetCompiledShaderByID(EngineShader::id id)
{
	assert(id < EngineShader::count);
	ComPtr<IDxcBlob> shader = m_compiledShaders[id];
	assert(shader->GetBufferPointer() && shader->GetBufferSize());
	D3D12_SHADER_BYTECODE result{};
	result.pShaderBytecode = shader->GetBufferPointer();
	result.BytecodeLength = shader->GetBufferSize();
	return result;
}

#endif // ENABLE_D3D12

