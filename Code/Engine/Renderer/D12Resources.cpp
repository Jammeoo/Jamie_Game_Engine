#include "Engine/Renderer/D12Resources.hpp"
#include "Engine/Renderer/RendererD12.hpp"
#include "Engine/Renderer/D12Helpers.hpp"

#ifdef ENABLE_D3D12
//--------------------------------------------------------------------------------------------------
bool DescriptorHeap::Initialize(unsigned int capacity, bool isShaderVisible)
{
	//m_mutex.lock();
	std::lock_guard lock{ m_mutex };
	assert(capacity && capacity < D3D12_MAX_SHADER_VISIBLE_DESCRIPTOR_HEAP_SIZE_TIER_2);
	assert(!(m_type == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER &&
		capacity > D3D12_MAX_SHADER_VISIBLE_SAMPLER_HEAP_SIZE));
	if (m_type == D3D12_DESCRIPTOR_HEAP_TYPE_DSV ||
		m_type == D3D12_DESCRIPTOR_HEAP_TYPE_RTV) 
	{
		isShaderVisible = false;
	}
	Release();
	
	auto* const device= m_ownerRender->GetDevice();
	assert(device);
	
	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.Flags = isShaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	desc.NumDescriptors = capacity;
	desc.Type = m_type;
	desc.NodeMask = 0;

	HRESULT hr;
	DXCall(hr = device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_heap)));
	if (!SUCCEEDED(hr)) return false;
	
	//m_freeHandles = std::move(std::make_unique<u32[]>(capacity));
	m_freeHandles.resize((size_t)capacity);
	m_capacity = capacity;
	m_size = 0;
	for (u32 i = 0; i < capacity; ++i) 
	{
		m_freeHandles[i] = i;
	}
	for (u32 i = 0; i < FRAME_BUFFER_COUNT; ++i) 
	{
		assert(m_deferredFreeIndices[i].empty());
	}
	m_descriptorSize = device->GetDescriptorHandleIncrementSize(m_type);
	m_CPUStart = m_heap->GetCPUDescriptorHandleForHeapStart();
	m_GPUStart = isShaderVisible ? m_heap->GetGPUDescriptorHandleForHeapStart() : D3D12_GPU_DESCRIPTOR_HANDLE{ 0 };
	//m_mutex.unlock();
	return true;


}

void DescriptorHeap::Release()
{
	//assert(!m_size);//#ToDo:Debug
	m_ownerRender->DeferredRelease(m_heap);
}

DescriptorHandle DescriptorHeap::Allocate()
{
	//m_mutex.lock();
	std::lock_guard lock{ m_mutex };
	assert(m_heap);
	assert(m_size < m_capacity);
	unsigned int index = m_freeHandles[m_size];
	unsigned int offset = index * m_descriptorSize;
	++m_size;

	DescriptorHandle handle;

	handle.m_cpu.ptr = m_CPUStart.ptr + offset;
	if (IsShaderVisible()) 
	{
		handle.m_gpu.ptr = m_GPUStart.ptr + offset;
	}
	handle.m_container = this;
	handle.m_index = index;
	//m_mutex.unlock();
	return handle;

}

void DescriptorHeap::Free(DescriptorHandle& handle)
{
	if (!handle.IsValid()) return;

	std::lock_guard lock{ m_mutex };
	assert(m_heap && m_size);
	assert(handle.m_container == this);
	assert(handle.m_cpu.ptr >= m_CPUStart.ptr);
	assert((handle.m_cpu.ptr - m_CPUStart.ptr) % m_descriptorSize == 0);
	assert(handle.m_index < m_capacity);
	unsigned int index = (unsigned int)(handle.m_cpu.ptr - m_CPUStart.ptr) / m_descriptorSize;
	assert(handle.m_index == index);

	u32 frameIndex = m_ownerRender->GetCurrentFrameIndex();
	m_deferredFreeIndices[frameIndex].push_back(index);
	m_ownerRender->SetDefferredReleasesFlag();

	handle = {};

}

void DescriptorHeap::ProcessDeferredFree(u32 frameIdx)
{
	//m_mutex.lock();
	std::lock_guard lock{ m_mutex };
	assert(frameIdx < FRAME_BUFFER_COUNT);
	std::vector<u32>& indices = m_deferredFreeIndices[frameIdx];
	if (!indices.empty()) 
	{
		for (auto index : indices) 
		{
			--m_size;
			//m_freeHandles.push_back(index);
			m_freeHandles[m_size] = index;
		}
		indices.clear();
	}
	//m_mutex.unlock();

}
//--------------------------------------------------------------------------------------------------

D12Texture::D12Texture(RendererD12* renderer, D12TextureInitInfo info):m_ownerRender(renderer)
{
	auto *const device = m_ownerRender->GetDevice();
	assert(device);
	m_isRaytracing = info.m_isRayTracingTex;
	D3D12_CLEAR_VALUE* clearValue = nullptr; 
	
	if (info.m_desc &&
		(info.m_desc->Flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET || 
			info.m_desc->Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL)
		)
	{
		clearValue = &info.m_clearValue;
	}
	else
	{
		clearValue = nullptr;
	}

	if (info.m_resource) 
	{
		//assert(!info.m_resource);
		//info.m_resource = m_resource;
		m_resource = info.m_resource;
	}
	else if (info.m_heap && info.m_desc)
	{
		assert(!info.m_resource);
		DXCall(device->CreatePlacedResource(info.m_heap, info.m_allocationInfo.Offset, info.m_desc,
			info.m_initialState, clearValue, IID_PPV_ARGS(&m_resource)));
	}
	else if(info.m_desc)
	{
		assert(!info.m_heap && !info.m_resource);
		DXCall(device->CreateCommittedResource(&HeapProperties.defaultHeap, D3D12_HEAP_FLAG_NONE, info.m_desc,
			info.m_initialState, clearValue, IID_PPV_ARGS(&m_resource)));

	}
	assert(m_resource);

	if(!info.m_isRayTracingTex)
	{
		m_srv = m_ownerRender->GetSRVHeap().Allocate();
		device->CreateShaderResourceView(m_resource, info.m_srvDesc, m_srv.m_cpu);
	}
	else
	{
		m_uav = m_ownerRender->GetUAVHeap().Allocate();
		device->CreateShaderResourceView(m_resource, info.m_srvDesc, m_uav.m_cpu);
	}


}

D12Texture::~D12Texture()
{
	Release();
}

void D12Texture::Release()
{
	if (m_isRaytracing) 
	{
		m_ownerRender->GetUAVHeap().Free(m_uav);
	}
	else
	{
		m_ownerRender->GetSRVHeap().Free(m_srv);
	}
	m_ownerRender->DeferredRelease(m_resource);

}

//--------------------------------------------------------------------------------------------------
//Render texture
D12RenderTexuture::D12RenderTexuture(RendererD12* renderer, D12TextureInitInfo info)
	:m_ownerRender(renderer)
{
	m_texture = new D12Texture(m_ownerRender, info);
	assert(info.m_desc);
	m_mipCount = GetResource()->GetDesc().MipLevels;
	assert(m_mipCount && m_mipCount <= D12Texture::MAX_MIPS);

	DescriptorHeap& rtvHeap = m_ownerRender->GetRTVHeap();
	D3D12_RENDER_TARGET_VIEW_DESC desc = {};
	desc.Format = info.m_desc->Format;
	desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	desc.Texture2D.MipSlice = 0;
	auto* const device = m_ownerRender->GetDevice();
	assert(device);

	for (u32 i = 0; i < m_mipCount; ++i) 
	{
		m_rtv[i] = rtvHeap.Allocate();
		device->CreateRenderTargetView(GetResource(), &desc, m_rtv[i].m_cpu);
		++desc.Texture2D.MipSlice;
	}

	

}


void D12RenderTexuture::Release()
{
	for (u32 i = 0; i < m_mipCount; ++i) 
	{
		m_ownerRender->GetRTVHeap().Free(m_rtv[i]);
		m_texture->Release();
		m_mipCount = 0;
	}
}

//--------------------------------------------------------------------------------------------------
//Depth buffer







D12DepthBuffer::D12DepthBuffer(RendererD12* renderer, D12TextureInitInfo info) : m_ownerRender(renderer)
{
	assert(info.m_desc);
	const DXGI_FORMAT dsvFormat = info.m_desc->Format;
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	if (info.m_desc->Format == DXGI_FORMAT_D32_FLOAT) 
	{
		info.m_desc->Format = DXGI_FORMAT_R32_TYPELESS;
		srvDesc.Format = DXGI_FORMAT_R32_FLOAT;

	}

	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.PlaneSlice = 0;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.f;

	assert(!info.m_srvDesc && !info.m_resource);
	info.m_srvDesc = &srvDesc;
	m_texture = new D12Texture(m_ownerRender, info);
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	dsvDesc.Format = dsvFormat;
	dsvDesc.Texture2D.MipSlice = 0;
	m_dsv = m_ownerRender->GetDSVHeap().Allocate();
	auto* const device = m_ownerRender->GetDevice();
	assert(device);
	device->CreateDepthStencilView(GetResource(), &dsvDesc, m_dsv.m_cpu);
}

void D12DepthBuffer::Release()
{
	m_ownerRender->GetDSVHeap().Free(m_dsv);
	m_texture->Release();
}


D12Buffer::D12Buffer(RendererD12* renderer, D12BufferInitInfo info, bool isCpuAccessible) : m_ownerRenderer(renderer)
{
	assert(!m_buffer && info.m_size && info.m_alignment);
	assert(info.m_size % info.m_alignment == 0);
	assert(renderer->GetHelper());
	D12Helpers* helper = renderer->GetHelper();
	m_size = info.m_size;
	m_buffer = helper->CreateBuffer(info.m_data, m_size, isCpuAccessible, info.m_initialState, info.m_flags,
		info.m_heap, info.m_allocationInfo.Offset);
	m_gpuAddress = m_buffer->GetGPUVirtualAddress();
	m_buffer->SetName(L"D12 Buffer - size: " + m_size);
}

void D12Buffer::Release()
{
	m_ownerRenderer->DeferredRelease(m_buffer);
	m_gpuAddress = 0;
	m_size = 0;
}

//----------------------------------------------------------------------------------------------------------------------//

ConstantBuffer::ConstantBuffer(RendererD12* renderer, D12BufferInitInfo info) :m_ownerRenderer(renderer), m_buffer{ renderer, info, true } 
{
	assert(renderer);
	m_buffer.GetBuffer()->SetName(L"Constant Buffer - size" + GetSize());
	D3D12_RANGE range{};
	DXCall(GetBuffer()->Map(0, &range, (void**)&m_cpuAddress));


}

u8* const ConstantBuffer::Allocate(u32 size)
{
	std::lock_guard lock{ m_mutex };
	u32 correctSize = (u32)m_ownerRenderer->GetHelper()->AlignSizeForConstantBuffer(size);
	assert(size == correctSize);
	assert(m_cpuOffset + size <= m_buffer.GetSize());
	if (m_cpuOffset + size <= m_buffer.GetSize()) 
	{
		u8* const address = m_cpuAddress + m_cpuOffset;
		m_cpuOffset += correctSize;
		return address;
	}
	return nullptr;
}









#endif // ENABLE_D3D12