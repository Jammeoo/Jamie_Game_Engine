#pragma once
#include "Engine/Renderer/Renderer.hpp"

class VertexBuffer
{
	friend class Renderer;
public:
	VertexBuffer(size_t size);
	VertexBuffer(size_t size, unsigned int stride);
	VertexBuffer(const VertexBuffer& copy) = delete;
	virtual ~VertexBuffer();
	unsigned int GetStride() const;


	unsigned int m_stride = 0;
	ID3D11Buffer* m_buffer = nullptr;
	size_t m_size = 0;
	//D3D_PRIMITIVE_TOPOLOGY m_primityType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

};
