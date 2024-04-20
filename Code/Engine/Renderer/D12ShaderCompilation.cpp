#include "Engine/Renderer/D12ShaderCompilation.hpp"
#ifdef ENABLE_D3D12

ShaderCompiler::ShaderCompiler()
{
	HRESULT hr = S_OK;
	DXCall(hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&m_compiler)));
	DXCall(hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&m_utils)));
	DXCall(hr = m_utils->CreateDefaultIncludeHandler(&m_includeHander));
}

IDxcBlob* ShaderCompiler::CompileThis(ShaderFileInfo info, std::string fullPath)
{
	HRESULT hr = S_OK;
	assert(m_compiler && m_utils && m_includeHander);
	//load the source file using Utils interface.
	ComPtr<IDxcBlobEncoding> sourceBlob = nullptr;
	std::wstring wJustPath = ToWString(fullPath.c_str());
	LPCWSTR justPath = wJustPath.c_str();
	DXCall(hr = m_utils->LoadFile(justPath, nullptr, &sourceBlob));
	if (!SUCCEEDED(hr)) return nullptr;
	assert(sourceBlob && sourceBlob->GetBufferSize());

	std::wstring file = ToWString(info.file);
	std::wstring func = ToWString(info.function);
	std::wstring prof = ToWString(m_profileStrings[(u32)info.type]);

	LPCWSTR args[]
	{
		file.c_str(),									//optional shader source file name for error reporting
		L"-E",func.c_str(),								//Entry function
		L"-T",prof.c_str(),								//Target profile
		DXC_ARG_ALL_RESOURCES_BOUND,
#if _DEBUG
			DXC_ARG_DEBUG,
			DXC_ARG_SKIP_OPTIMIZATIONS,
#else
			DXC_ARG_OPTIMIZATION_LEVEL3,
#endif
			DXC_ARG_WARNINGS_ARE_ERRORS,
			L"-Qstrip_reflect",								//Strip reflection into a separate blob
			L"-Qstrip_debug",								//Strip debug information in to a separate blob
	};
	OutputDebugStringA("Compiling ");
	OutputDebugStringA(info.file);

	return CompileThis(sourceBlob.Get(), args, _countof(args));
}


IDxcBlob* ShaderCompiler::CompileThis(IDxcBlobEncoding* sourceBlob, LPCWSTR* args, u32 num_Args)
{
	DxcBuffer buffer{};
	buffer.Encoding = DXC_CP_ACP;//Auto-detect text format?
	buffer.Ptr = sourceBlob->GetBufferPointer();
	buffer.Size = sourceBlob->GetBufferSize();
	HRESULT hr = S_OK;
	ComPtr<IDxcResult> results = nullptr;
	DXCall(hr = m_compiler->Compile(&buffer, args, num_Args, m_includeHander.Get(), IID_PPV_ARGS(&results)));
	if (!SUCCEEDED(hr)) return nullptr;

	ComPtr<IDxcBlobUtf8> errors = nullptr;
	DXCall(hr = results->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr));
	if (!SUCCEEDED(hr)) return nullptr;
	if (errors && errors->GetStringLength())
	{
		OutputDebugStringA("\n Shader compilation error: \n");
		OutputDebugStringA(errors->GetStringPointer());
	}
	else
	{
		OutputDebugStringA(" [ Succeeded ] ");
	}
	OutputDebugStringA("\n");
	HRESULT status = S_OK;
	hr = results->GetStatus(&status);
	if (!SUCCEEDED(hr) || !SUCCEEDED(status)) return nullptr;

	ComPtr<IDxcBlob> shader = nullptr;
	DXCall(hr = results->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shader), nullptr));
	if (!SUCCEEDED(hr)) return nullptr;

	return shader.Detach();

}

std::wstring ShaderCompiler::ToWString(const char* c)
{
	std::string s = c;
	std::wstring result{ s.begin(),s.end() };
	return result;
}

#endif // ENABLE_D3D12

