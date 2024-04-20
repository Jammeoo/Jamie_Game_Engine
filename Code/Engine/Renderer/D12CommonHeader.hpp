#pragma once
#include "Engine/Core/EngineCommon.hpp"

#ifdef ENABLE_D3D12
#include "Engine/Core/ErrorWarningAssert.hpp"


#define ENGINE_DEBUG_RENDER
#pragma comment (lib,"d3d12.lib")
#pragma comment (lib,"dxgi.lib")
#pragma comment (lib,"../packages/DirectXShaderCompiler/lib/x64/dxcompiler.lib")

#pragma comment (lib,"dxguid.lib")

#include <wrl.h>
#include <d3d12.h>

#include "ThirdParty/DirectXHelpers/d3dx12.h" //Helper function
#include <dxgi1_6.h>
#include "..\packages\DirectXShaderCompiler\inc\dxcapi.h"
#include "..\packages\DirectXShaderCompiler\inc\d3d12shader.h"
using namespace Microsoft::WRL;

#include <mutex>

// #ifdef _DEBUG
// #define NAME_D3D12Object(obj, name) obj->SetName(name);
// #else
// #define NAME_D3D12Object(x,name)
// #endif // _DEBUG
// 

#ifdef _DEBUG
// Sets the name of the COM object and outputs a debug string int Visual Studio's output panel.
#define NAME_D3D12_OBJECT(obj, name) obj->SetName(name); OutputDebugString(L"::D3D12 Object Created: "); OutputDebugString(name); OutputDebugString(L"\n");
// The indexed variant will include the index in the name of the object
#define NAME_D3D12_OBJECT_INDEXED(obj, n, name)            \
{                                                          \
wchar_t full_name[128];                                    \
if (swprintf_s(full_name, L"%s[%llu]", name, (u64)n) >0 ){ \
    obj->SetName(full_name);                               \
    OutputDebugString(L"::D3D12 Object Created: ");        \
    OutputDebugString(full_name);                          \
    OutputDebugString(L"\n");                              \
}}
#else
#define NAME_D3D12_OBJECT(x, name)
#define NAME_D3D12_OBJECT_INDEXED(x, n, name)
#endif // _DEBUG





#ifdef _DEBUG
#ifndef DXCall
#define DXCall(x)												\
if (!SUCCEEDED(x))												\
{																\
	ERROR_AND_DIE("Result Failed.");     \
}
#endif //!DXCall
#else
#ifndef DXCall
#define DXCall(x) x
#endif //!DXCall
#endif // _DEBUG


#include <assert.h>
#include <DirectXMath.h>
//unsigned integers
using u64 = uint64_t;
using u32 = uint32_t;
using u16 = uint16_t;
using u8 = uint8_t;
//signed integers
using s64 = int64_t;
using s32 = int32_t;
using s16 = int16_t;
using s8 = int8_t;

//using f32 = std::float32_t;

#ifndef DISABLE_COPY
#define  DISABLE_COPY(T)				\
		explicit T(const T&)=delete;	\
		T& operator=(const T&)=delete;
#endif

#ifndef DISABLE_MOVE
#define  DISABLE_MOVE(T)				\
		explicit T(T&)=delete;			\
		T& operator = (T&&) = delete;
#endif

#ifndef DISABLE_COPY_AND_MOVE
#define  DISABLE_COPY_AND_MOVE(T) DISABLE_COPY(T) DISABLE_MOVE(T)
#endif



constexpr u32 U32_INVALID_ID = 0xFFFFFFFF;
constexpr int FRAME_BUFFER_COUNT = 3;
constexpr int BACK_BUFFER_COUNT = 2;
//constexpr DXGI_FORMAT DEFAULT_RENDER_TARGET_FORMAT = DXGI_FORMAT_R8G8B8A8_UNORM;
using id3d12Device = ID3D12Device8;
using id3d12GraphicsCommandList = ID3D12GraphicsCommandList6;







#endif // ENABLE_D3D12