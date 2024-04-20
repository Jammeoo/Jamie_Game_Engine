#pragma once

#include "Engine/Core/EngineCommon.hpp"
#ifdef ENABLE_D3D12
#include "Engine/Renderer/D12CommonHeader.hpp"
#include "Engine/Renderer/D12Shader.hpp"
//#include <d3d12shader.h>
#include <dxcapi.h>

class ShaderCompiler
{
public:
	ShaderCompiler();
	~ShaderCompiler() = default;
	DISABLE_COPY_AND_MOVE(ShaderCompiler);
	IDxcBlob* CompileThis(ShaderFileInfo info, std::string fullPath);
	IDxcBlob* CompileThis(IDxcBlobEncoding* sourceBlob, LPCWSTR* args, u32 num_Args);
	std::wstring ToWString(const char* c);
private:
	//Note: Shader model 6.x can also be used(AS and MS are only supported from SM6.5 on).


	constexpr static const char* m_profileStrings[]{ "vs_6_5","hs_6_5","ds_6_5" ,"gs_6_5" ,"ps_6_5" ,"cs_6_5" ,"as_6_5" ,"ms_6_5" };
	static_assert(_countof(m_profileStrings) == ShaderType::count);
	ComPtr<IDxcCompiler3>			m_compiler = nullptr;
	ComPtr<IDxcUtils>				m_utils = nullptr;
	ComPtr<IDxcIncludeHandler>		m_includeHander = nullptr;
};


#endif // ENABLE_D3D12