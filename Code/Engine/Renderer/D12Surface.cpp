#include "Engine/Renderer/D12Surface.hpp"
#include "Engine/Renderer/Window.hpp"
#include "Engine/Renderer/RendererD12.hpp"
#ifdef ENABLE_D3D12

#define  DX_SAFE_RELEASE(dxObject)				\
{												\
	if ((dxObject)!=nullptr)					\
	{											\
		(dxObject)->Release();					\
		(dxObject) = nullptr;					\
	}											\
}												\


D12Surface::D12Surface(RendererD12* renderer, Window* window) :m_ownerRenderer(renderer), m_window(window)
{
	assert(m_window->GetHwnd());
}


void D12Surface::CreateSwapChain(IDXGIFactory7* factory, ID3D12CommandQueue* cmd_queue, DXGI_FORMAT format /*= DEFAULTBACKBUFFERFORMAT*/)
{
	assert(factory && cmd_queue);
	Release();
	m_format = format;
	const IntVec2 windowSize = m_window->GetClientDimensions();
	DXGI_SWAP_CHAIN_DESC1 desc{};
	desc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	desc.BufferCount = BACK_BUFFER_COUNT;
	desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	desc.Flags = 0;
	desc.Format = m_format;					//DXGI_FORMAT_R8G8B8A8_UNORM_SRGB
	desc.Width = windowSize.x;
	desc.Height = windowSize.y;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Scaling = DXGI_SCALING_STRETCH;
	desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	desc.Stereo = false;

	DXGI_SWAP_CHAIN_FULLSCREEN_DESC fsSwapChainDesc = { 0 };
	fsSwapChainDesc.Windowed = TRUE;

	IDXGISwapChain1* swapChain;
	HWND hwnd = (HWND)m_window->GetHwnd();
	DXCall(factory->CreateSwapChainForHwnd(cmd_queue, hwnd, &desc, &fsSwapChainDesc, nullptr, &swapChain));
	DXCall(factory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER));
	DXCall(swapChain->QueryInterface(IID_PPV_ARGS(&m_swapChain)));

	DX_SAFE_RELEASE(swapChain);

	m_currentBackBufferIndex = m_swapChain->GetCurrentBackBufferIndex();

	for (u32 i = 0; i < BACK_BUFFER_COUNT; ++i)
	{
		m_renderTargetData[i].m_rtv = m_ownerRenderer->GetRTVHeap().Allocate();
	}

	Finalize();

}

void D12Surface::Present() const
{
	assert(m_swapChain);
	DXCall(m_swapChain->Present(1, 0));
	m_currentBackBufferIndex = m_swapChain->GetCurrentBackBufferIndex();
}

void D12Surface::Release()
{
	for (u32 i = 0; i < BACK_BUFFER_COUNT; ++i) 
	{
		RenderTargetData& data = m_renderTargetData[i];
		DX_SAFE_RELEASE(data.m_resource);
		m_ownerRenderer->GetRTVHeap().Free(data.m_rtv);
	}

	DX_SAFE_RELEASE(m_swapChain);
}

void D12Surface::Finalize()
{
	//create RTVs for back-buffers
	for (u32 i = 0; i < BACK_BUFFER_COUNT; ++i) 
	{
		RenderTargetData& data = m_renderTargetData[i];
		assert(!data.m_resource);
		DXCall(m_swapChain->GetBuffer(i, IID_PPV_ARGS(&data.m_resource)));
		D3D12_RENDER_TARGET_VIEW_DESC desc{};
		desc.Format = m_format;
		desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		m_ownerRenderer->GetDevice()->CreateRenderTargetView(data.m_resource, &desc, data.m_rtv.m_cpu);
		
	}

	DXGI_SWAP_CHAIN_DESC desc{};
	DXCall(m_swapChain->GetDesc(&desc));
	u32 width = desc.BufferDesc.Width;
	u32 height = desc.BufferDesc.Height;
	assert((u32)m_window->GetClientDimensions().x == width && (u32)m_window->GetClientDimensions().y == height);

	m_viewport.TopLeftX = 0.f;
	m_viewport.TopLeftY = 0.f;
	m_viewport.Width = (float)width;
	m_viewport.Height = (float)height;

	m_viewport.MinDepth = 0.f;
	m_viewport.MaxDepth = 1.f;

	m_scissorRect = { 0,0,(long)width,(long)height };

}
#endif // ENABLE_D3D12