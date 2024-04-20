#include "Engine/Renderer/VertexBuffer.hpp"
#define  DX_SAFE_RELEASE(dxObject)				\
{												\
	if ((dxObject)!=nullptr)					\
	{											\
		(dxObject)->Release();					\
		(dxObject) = nullptr;					\
	}											\
}												\

VertexBuffer::VertexBuffer(size_t size)
{
	m_size = size;
}

VertexBuffer::VertexBuffer(size_t size, unsigned int stride)
{
	m_size = size;
	m_stride = stride;
}



VertexBuffer::~VertexBuffer()
{
	DX_SAFE_RELEASE(m_buffer);
}

unsigned int VertexBuffer::GetStride() const
{
	return m_stride;
}
