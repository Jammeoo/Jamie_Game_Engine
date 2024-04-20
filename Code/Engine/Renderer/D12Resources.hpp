#pragma once


#include "Engine/Renderer/D12CommonHeader.hpp"
//#include "Engine/Renderer/D12Resources.hpp"
#include <vector>

#ifdef ENABLE_D3D12

class DescriptorHeap;
class RendererD12;

struct DescriptorHandle
{
	D3D12_CPU_DESCRIPTOR_HANDLE m_cpu{};
	D3D12_GPU_DESCRIPTOR_HANDLE m_gpu{};

	[[nodiscard]]bool IsValid()const { return m_cpu.ptr != 0; }
	[[nodiscard]]bool IsShaderVisible()const { return m_gpu.ptr != 0; }
public:
	u32 GetIndex() { return m_index; }
private:
	unsigned int m_index = 0;
	friend class DescriptorHeap;
	DescriptorHeap* m_container = nullptr;
#ifdef _DEBUG
#endif // _DEBUG

};


class DescriptorHeap
{
public:
	explicit DescriptorHeap(RendererD12* renderer, D3D12_DESCRIPTOR_HEAP_TYPE type) :m_ownerRender(renderer), m_type(type) {};
	DISABLE_COPY_AND_MOVE(DescriptorHeap);
	~DescriptorHeap() {
	//assert(!m_heap);//#ToDo:Debug
// 		if (m_heap) 
// 		{
// 			//int i = 0;
// 		}
	}
	
	//Make sure heap was released
	bool Initialize(unsigned int capacity, bool isShaderVisible);
	void Release();

	DescriptorHandle Allocate();
	void Free(DescriptorHandle& handle);
	void ProcessDeferredFree(u32 frameIdx);

	[[nodiscard]]constexpr D3D12_DESCRIPTOR_HEAP_TYPE GetType() { return m_type; }
	[[nodiscard]]constexpr D3D12_CPU_DESCRIPTOR_HANDLE GetCPUStart() { return m_CPUStart; }
	[[nodiscard]]constexpr D3D12_GPU_DESCRIPTOR_HANDLE GetGPUStart() { return m_GPUStart; }
	[[nodiscard]]constexpr ID3D12DescriptorHeap* GetHeap() { return m_heap; }
	[[nodiscard]]constexpr u32 GetCapacity() { return m_capacity; }
	[[nodiscard]]constexpr u32 GetSize() { return m_size; }
	[[nodiscard]]constexpr u32 GetDescriptorSize() { return m_descriptorSize; }
	[[nodiscard]]constexpr bool IsShaderVisible() { return m_GPUStart.ptr != 0; }
private:
	RendererD12* m_ownerRender = nullptr;

	ID3D12DescriptorHeap* m_heap = nullptr;
	D3D12_CPU_DESCRIPTOR_HANDLE			m_CPUStart = {};
	D3D12_GPU_DESCRIPTOR_HANDLE			m_GPUStart = {};
	std::vector<unsigned int>			m_freeHandles;
	//std::unique_ptr<u32[]> m_freeHandles{};
	std::vector<unsigned int>			m_deferredFreeIndices[FRAME_BUFFER_COUNT];
	unsigned int						m_capacity = 0;
	unsigned int						m_size = 0;
	unsigned int						m_descriptorSize;
	const D3D12_DESCRIPTOR_HEAP_TYPE	m_type{};
	std::mutex							m_mutex;

};

struct D12TextureInitInfo
{
	ID3D12Heap*								m_heap = nullptr;
	ID3D12Resource*							m_resource = nullptr;
	D3D12_SHADER_RESOURCE_VIEW_DESC*		m_srvDesc = nullptr;
	D3D12_RESOURCE_DESC*					m_desc = nullptr;
	D3D12_RESOURCE_ALLOCATION_INFO1			m_allocationInfo{};
	D3D12_RESOURCE_STATES					m_initialState{};
	D3D12_CLEAR_VALUE						m_clearValue{};
	bool m_isRayTracingTex = false;
};

class D12Texture
{
public:
	constexpr static u32 MAX_MIPS = 14;//support up to 16K resolutions.

	D12Texture() = default;
	explicit D12Texture(RendererD12* renderer, D12TextureInitInfo info);
	~D12Texture();
	DISABLE_COPY(D12Texture);
	constexpr D12Texture(D12Texture&& o)
		:m_resource(o.m_resource), m_srv(o.m_srv) ,m_uav(o.m_uav)
	{
		o.Reset();
	}
	constexpr D12Texture& operator=(D12Texture&& o)
	{
		assert(this != &o);
		if (this != &o) 
		{
			Release();
			Move(o);
		}
		return *this;
	}
	[[nodiscard]]ID3D12Resource* GetResource()const { return m_resource; }
	[[nodiscard]]DescriptorHandle GetDescriptorHandleSRV()const { return m_srv; }
	[[nodiscard]] DescriptorHandle GetDescriptorHandleUAV()const { return m_uav; }
	void Release();
	
private:
	constexpr void Move(D12Texture& o)
	{
		m_resource = o.m_resource;
		m_ownerRender = o.m_ownerRender;
	}
	constexpr void Reset()
	{
		m_ownerRender = nullptr;
		m_resource = nullptr;
		m_srv = {};
		m_uav = {};
	}
	
private:
	ID3D12Resource* m_resource = nullptr;
	DescriptorHandle m_srv{};
	bool m_isRaytracing = false;
	DescriptorHandle m_uav{};
	RendererD12* m_ownerRender = nullptr;
};

class D12RenderTexuture
{
public:
	D12RenderTexuture() = default;
	explicit D12RenderTexuture(RendererD12* renderer, D12TextureInitInfo info);
	DISABLE_COPY(D12RenderTexuture);
	constexpr D12RenderTexuture(D12RenderTexuture&& o)
		:m_texture(std::move(o.m_texture)), m_mipCount(o.m_mipCount) 
	{
		for (u32 i = 0; i < m_mipCount; ++i) 
		{
			m_rtv[i] = o.m_rtv[i];
		}
		o.Reset();
	}
	constexpr D12RenderTexuture& operator=(D12RenderTexuture&& o)
	{
		assert(this != &o);
		if (this != &o)
		{
			Release();
			Move(o);
		}
		return *this;
	}
	~D12RenderTexuture() { Release(); }
	void Release();
	[[nodiscard]] u32 GetMipCount()const { return m_mipCount; }
	[[nodiscard]] D3D12_CPU_DESCRIPTOR_HANDLE GetRTV(u32 mipIndex)
	{
		assert(mipIndex < m_mipCount);
		return m_rtv[mipIndex].m_cpu;
	}
	[[nodiscard]]DescriptorHandle GetSRV()const { return m_texture->GetDescriptorHandleSRV(); }
	[[nodiscard]]ID3D12Resource* const GetResource()const { return m_texture->GetResource(); }

private:
	constexpr void Move(D12RenderTexuture& o)
	{
		m_texture = std::move(o.m_texture);
		m_ownerRender = o.m_ownerRender;
		m_mipCount = o.m_mipCount;
		for (u32 i = 0; i < m_mipCount; ++i) 
		{
			m_rtv[i] = o.m_rtv[i];
		}
		o.Reset();

	}

	constexpr void Reset()
	{
		for (u32 i = 0; i < m_mipCount; ++i)
		{
			m_ownerRender = nullptr;
			m_rtv[i] = {};
			m_mipCount = 0;
		}
	}


private:
	RendererD12* m_ownerRender = nullptr;
	D12Texture* m_texture = nullptr;
	DescriptorHandle m_rtv[D12Texture::MAX_MIPS]{};
	u32 m_mipCount = 0;


};


class D12DepthBuffer
{
public:
	D12DepthBuffer() = default;
	explicit D12DepthBuffer(RendererD12* renderer, D12TextureInitInfo info);
	DISABLE_COPY(D12DepthBuffer);
	constexpr D12DepthBuffer(D12DepthBuffer&& o)
		:m_texture(std::move(o.m_texture)), m_dsv(o.m_dsv)
	{
		o.m_dsv = {};
	}
	constexpr D12DepthBuffer& operator=(D12DepthBuffer&& o)
	{
		assert(this != &o);
		if (this != &o)
		{

			m_texture = std::move(o.m_texture);
			m_dsv = o.m_dsv;
			o.m_dsv = {};
		}
		return *this;
	}
	~D12DepthBuffer() { Release(); }
	void Release();
	[[nodiscard]]D3D12_CPU_DESCRIPTOR_HANDLE GetDSV(){ return m_dsv.m_cpu; }
	[[nodiscard]]DescriptorHandle GetSRV()const { return m_texture->GetDescriptorHandleSRV(); }
	[[nodiscard]]ID3D12Resource* const GetResource()const { return m_texture->GetResource(); }
private:
	RendererD12* m_ownerRender = nullptr;
	D12Texture* m_texture = nullptr;
	DescriptorHandle m_dsv{};

};

struct D12BufferInitInfo
{
	ID3D12Heap1* m_heap = nullptr;
	const void* m_data = nullptr;
	D3D12_RESOURCE_ALLOCATION_INFO1     m_allocationInfo{};
	D3D12_RESOURCE_STATES               m_initialState{};
	D3D12_RESOURCE_FLAGS                m_flags = D3D12_RESOURCE_FLAG_NONE;
	u32                                 m_size = 0;
	u32                                 m_alignment = 0;
};



class D12Buffer
{
public:
	D12Buffer() = default;
	explicit D12Buffer(RendererD12* renderer, D12BufferInitInfo info, bool isCpuAccessible);
	DISABLE_COPY(D12Buffer);
	constexpr D12Buffer(D12Buffer&& o)
		: m_buffer{ o.m_buffer }, m_gpuAddress{ o.m_gpuAddress }, m_size{ o.m_size }
	{
		o.Reset();
	}

	constexpr D12Buffer& operator=(D12Buffer&& o)
	{
		assert(this != &o);
		if (this != &o)
		{
			Release();
			Move(o);
		}

		return *this;
	}

	~D12Buffer() { Release(); }

	void Release();
	[[nodiscard]] constexpr ID3D12Resource* const GetBuffer() const { return m_buffer; }
	[[nodiscard]] constexpr D3D12_GPU_VIRTUAL_ADDRESS GetGPUAddress() const { return m_gpuAddress; }
	[[nodiscard]] constexpr u32 GetSize() const { return m_size; }

private:
	constexpr void Move(D12Buffer& o)
	{
		m_buffer = o.m_buffer;
		m_gpuAddress = o.m_gpuAddress;
		m_size = o.m_size;
		o.Reset();
	}

	constexpr void Reset()
	{
		m_buffer = nullptr;
		m_gpuAddress = 0;
		m_size = 0;
	}
	RendererD12* m_ownerRenderer = nullptr;
	ID3D12Resource* m_buffer{ nullptr };
	D3D12_GPU_VIRTUAL_ADDRESS   m_gpuAddress{ 0 };
	u32                         m_size{ 0 };
};

class ConstantBuffer
{
public:
	ConstantBuffer() = default;
	explicit ConstantBuffer(RendererD12* renderer, D12BufferInitInfo info);
	DISABLE_COPY_AND_MOVE(ConstantBuffer);
	~ConstantBuffer() { Release(); }

	void Release()
	{
		m_buffer.Release();
		m_cpuAddress = nullptr;
		m_cpuOffset = 0;
	}
	
	constexpr void Clear() { m_cpuOffset = 0; }
	[[nodiscard]] u8* const Allocate(u32 size);
	
	template<typename T>
	[[nodiscard]] T* const Allocate()
	{
		return (T* const)Allocate(sizeof(T));
	}

	[[nodiscard]] constexpr ID3D12Resource* const GetBuffer() const { return m_buffer.GetBuffer(); }
	[[nodiscard]] constexpr D3D12_GPU_VIRTUAL_ADDRESS GetGPUAddress() const { return m_buffer.GetGPUAddress(); }
	[[nodiscard]] constexpr u32 GetSize() const { return m_buffer.GetSize(); }
	[[nodiscard]] constexpr u8* const GetCPUAddress() const { return m_cpuAddress; }

	template<typename T>
	[[nodiscard]] constexpr D3D12_GPU_VIRTUAL_ADDRESS GetGPUAddress(T* const allocation)
	{
		std::lock_guard lock{ m_mutex };
		assert(m_cpuAddress);
		if (!m_cpuAddress) return {};
		const u8* const address{ (const u8* const)allocation };
		assert(address <= m_cpuAddress + m_cpuOffset);
		assert(address >= m_cpuAddress);
		const u64 offset{ (u64)(address - m_cpuOffset) };
		return m_buffer.GetGPUAddress()+ offset;
	}
	[[nodiscard]] constexpr static D12BufferInitInfo GetDefaultInitInfo(u32 size) 
	{
		assert(size);
		D12BufferInitInfo info{};
		info.m_size = size;
		info.m_alignment = D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT;
		return info;
	}

private:
	RendererD12* m_ownerRenderer = nullptr;
	D12Buffer    m_buffer{};
	u8* m_cpuAddress = nullptr;
	u32             m_cpuOffset = 0;
	std::mutex      m_mutex{};
};





#endif // ENABLE_D3D12