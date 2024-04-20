#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include <d3d11.h>
#define  DX_SAFE_RELEASE(dxObject)				\
{												\
	if ((dxObject)!=nullptr)					\
	{											\
		(dxObject)->Release();					\
		(dxObject) = nullptr;					\
	}											\
}												\


Texture::Texture()
{

}

Texture::~Texture()
{
	DX_SAFE_RELEASE(m_shaderRecourceView);
	DX_SAFE_RELEASE(m_texture);
}
