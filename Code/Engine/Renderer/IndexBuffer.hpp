#pragma once
#include "Engine/Renderer/Renderer.hpp"
class IndexBuffer
{
	friend class Renderer;
public:
	IndexBuffer(size_t size);
	IndexBuffer(const VertexBuffer& copy) = delete;
	virtual ~IndexBuffer();
	unsigned int GetStride() const;



	ID3D11Buffer* m_buffer = nullptr;
	size_t m_size = 0;

};
