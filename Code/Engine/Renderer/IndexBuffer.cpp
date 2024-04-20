#include "Engine/Renderer/IndexBuffer.hpp"

#define  DX_SAFE_RELEASE(dxObject)				\
{												\
	if ((dxObject)!=nullptr)					\
	{											\
		(dxObject)->Release();					\
		(dxObject) = nullptr;					\
	}											\
}												\

IndexBuffer::IndexBuffer(size_t size)
{
	m_size = size;
}

IndexBuffer::~IndexBuffer()
{
 	DX_SAFE_RELEASE(m_buffer);
}

unsigned int IndexBuffer::GetStride() const
{
	return sizeof(Vertex_PCU);
}
