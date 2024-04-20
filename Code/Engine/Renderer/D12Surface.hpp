#pragma once

#include "Engine/Renderer/D12CommonHeader.hpp"
#include "Engine/Renderer/D12Resources.hpp"
#ifdef ENABLE_D3D12
class Window;
class RendererD12;
class D12Surface
{
public:
	constexpr static DXGI_FORMAT DEFAULTBACKBUFFERFORMAT = DXGI_FORMAT_R8G8B8A8_UNORM;


public:
	explicit D12Surface(RendererD12* renderer, Window* window);
		
	~D12Surface() { Release(); }

	void CreateSwapChain(IDXGIFactory7* factory, ID3D12CommandQueue* cmd_queue, DXGI_FORMAT format = DEFAULTBACKBUFFERFORMAT);

	void Present()const;
	
	u32 GetWidth()const { return (u32)m_viewport.Width; }
	u32 GetHeight()const { return (u32)m_viewport.Height; }
	ID3D12Resource* const GetBackBuffer()const 
	{ 
		return m_renderTargetData[m_currentBackBufferIndex].m_resource; 
	}

	D3D12_CPU_DESCRIPTOR_HANDLE GetRTV()const { return m_renderTargetData[m_currentBackBufferIndex].m_rtv.m_cpu; }
	const D3D12_VIEWPORT& GetViewport()const { return m_viewport; }
	const D3D12_RECT& GetScissorRect()const { return m_scissorRect; }

private:
	struct RenderTargetData 
	{
		ID3D12Resource* m_resource = nullptr;
		DescriptorHandle m_rtv{};
	};
	void Release();
	void Finalize();

	RendererD12* m_ownerRenderer = nullptr;
	Window* m_window = nullptr;
	IDXGISwapChain4* m_swapChain = nullptr;
	RenderTargetData m_renderTargetData[BACK_BUFFER_COUNT]{};
	mutable u32 m_currentBackBufferIndex = 0;

	D3D12_VIEWPORT m_viewport{};
	D3D12_RECT m_scissorRect{};
	DXGI_FORMAT m_format = DEFAULTBACKBUFFERFORMAT;
};

#endif // ENABLE_D3D12