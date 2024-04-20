#include "Engine/Renderer/ConstantBuffer.hpp"
#define  DX_SAFE_RELEASE(dxObject)				\
{												\
	if ((dxObject)!=nullptr)					\
	{											\
		(dxObject)->Release();					\
		(dxObject) = nullptr;					\
	}											\
}												\


ConstantBuffer::ConstantBuffer(size_t size)
{
	m_size = size;
}

ConstantBuffer::~ConstantBuffer()
{
	DX_SAFE_RELEASE(m_buffer);
}

