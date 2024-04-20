#pragma once

#include "Engine/Core/EngineCommon.hpp"
#ifdef ENABLE_D3D12
#include "Engine/Renderer/D12CommonHeader.hpp"
#include "Engine/Renderer/D12Helpers.hpp"
#include "Engine/Math/IntVec2.hpp"
class D12RenderTexuture;
class D12DepthBuffer;
class RendererD12;

struct D12FrameInfo 
{
	u32 m_surfaceWidth;
	u32 m_surfaceHeight;
};



class D12GPass
{
public:
	struct GPassPsoStruct {
	public:
		D12PipelineStateSubobjectrootSignature rootSignature{};
		D12PipelineStateSubobjectvs	vs{};
		D12PipelineStateSubobjectps ps{};
		D12PipelineStateSubobjectprimitiveTopology primitiveTopology{};
		D12PipelineStateSubobjectrenderTargetFormats renderTargetFormats;
		D12PipelineStateSubobjectdepthStencilFormat depthStencilFormat{};
		D12PipelineStateSubobjectrasterizer rasterizer{};
		D12PipelineStateSubobjectdepthStencil1 depth{};

	}gPassPsoStream;

public:
	D12GPass(RendererD12* renderer) :m_ownerRenderer(renderer)
	{

	}
	bool Initialize();
	bool CreateGPassPsoAndRootSignature();
	void Shutdown();

	bool CreateBuffers(IntVec2 const& size);
	void DepthPrepass(id3d12GraphicsCommandList* cmdList, D12FrameInfo const& info);
	void Render(id3d12GraphicsCommandList* cmdList, D12FrameInfo const& info);


	void SetRenderTargetsForDepthPrepass(id3d12GraphicsCommandList* cmdList);
	void SetRenderTargetsForGPass(id3d12GraphicsCommandList* cmdList);

	void SetSize(IntVec2 size);

	void AddTransitionForDepthPrePass(D12ResourceBarrier* barriers);
	void AddTransitionForGPass(D12ResourceBarrier* barriers);
	void AddTransitionForPostProcess(D12ResourceBarrier* barriers);
	D12RenderTexuture* GetMainBuffer() { return m_gPassMainBuffer; }
	D12DepthBuffer* GetDepthBuffer() { return m_gPassDepthBuffer; }
private:
	RendererD12* m_ownerRenderer = nullptr;

	IntVec2 m_initialDimensions = IntVec2(100, 100);
	D12RenderTexuture* m_gPassMainBuffer = nullptr;
	D12DepthBuffer* m_gPassDepthBuffer = nullptr;

	DXGI_FORMAT m_mainBufferFormat = DXGI_FORMAT_R16G16B16A16_FLOAT;
	DXGI_FORMAT m_depthBufferFormat = DXGI_FORMAT_D32_FLOAT;
	IntVec2 m_dimension = m_initialDimensions;

	ID3D12RootSignature* m_gpassRootSig = nullptr;
	ID3D12PipelineState* m_gpassPso = nullptr;
	GPassPsoStruct m_stream;

	

};



#endif // ENABLE_D3D12