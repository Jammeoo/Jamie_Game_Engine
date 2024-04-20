#include "Engine/Renderer/D12GPass.hpp"

#ifdef ENABLE_D3D12
#include "Engine/Renderer/D12Resources.hpp"

#include "Engine/Renderer/D12Shader.hpp"
#include "Engine/Renderer/RendererD12.hpp"

#if _DEBUG
constexpr float clearValue[4]{ 0.5f,0.5f,0.5f,1.f };
#else
constexpr float clearValue[4]{ 0.f,0.f,0.f,1.f };
#endif

#define  DX_SAFE_RELEASE(dxObject)				\
{												\
	if ((dxObject)!=nullptr)					\
	{											\
		(dxObject)->Release();					\
		(dxObject) = nullptr;					\
	}											\
}												\

struct Gpass_ROOT_PARAM_INDICES
{
	enum : u32 {
		root_constants,
		count
	};
};

bool D12GPass::Initialize()
{
	return (CreateBuffers(m_initialDimensions) && CreateGPassPsoAndRootSignature());
}

bool D12GPass::CreateGPassPsoAndRootSignature()
{
	assert(!m_gpassRootSig && !m_gpassPso);

	//Create a GPass root signature
	using idx = Gpass_ROOT_PARAM_INDICES;


	D12RootParameter parameters[idx::count]{};
	parameters[0].AsConstants(3, D3D12_SHADER_VISIBILITY_PIXEL, 1);

	D12Helpers* helper = m_ownerRenderer->GetHelper();
	const D12RootSignatureDesc rootSignature = D12RootSignatureDesc(helper, &parameters[0], idx::count);
	m_gpassRootSig = rootSignature.Create();
	assert(m_gpassRootSig);

	//Create a GPass PSO

	struct {
		D12PipelineStateSubobjectrootSignature rootSignature{  };
		D12PipelineStateSubobjectvs	vs{};
		D12PipelineStateSubobjectps ps{};
		D12PipelineStateSubobjectprimitiveTopology primitiveTopology{};
		D12PipelineStateSubobjectrenderTargetFormats renderTargetFormats;
		D12PipelineStateSubobjectdepthStencilFormat depthStencilFormat{};
		D12PipelineStateSubobjectrasterizer rasterizer{};
		D12PipelineStateSubobjectdepthStencil1 depth{};
	
	}localStream;

	localStream.rootSignature = m_gpassRootSig;
	localStream.vs = m_ownerRenderer->GetShaders()->GetCompiledShaderByID(EngineShader::FULLSCREEN_TRIANGLE_VS);
	localStream.ps = m_ownerRenderer->GetShaders()->GetCompiledShaderByID(EngineShader::FILL_COLOR_PS);
	localStream.primitiveTopology = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	localStream.depthStencilFormat = m_depthBufferFormat;
	localStream.rasterizer = rasterizerState.noCull;
	localStream.depth = depthState.disabled;

	//m_stream.rootSignature = m_gpassRootSig;
	//m_stream.vs = m_ownerRenderer->GetShaders()->GetEngineShaderByID(EngineShader::FULLSCREEN_TRIANGLE_VS);
	//m_stream.ps = m_ownerRenderer->GetShaders()->GetEngineShaderByID(EngineShader::FILL_COLOR_PS);
	//m_stream.primitiveTopology = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	//m_stream.depthStencilFormat = m_depthBufferFormat;
	//m_stream.rasterizer = rasterizerState.noCull;
	//m_stream.depth = depthState.disabled;

	D3D12_RT_FORMAT_ARRAY rtfArray{};
	rtfArray.NumRenderTargets = 1;
	rtfArray.RTFormats[0] = m_mainBufferFormat;
	//m_stream.renderTargetFormats = rtfArray;
	localStream.renderTargetFormats = rtfArray;


	m_gpassPso = helper->CreatePipelineState(&localStream, sizeof(localStream));
	DebuggerPrintf("D12 Object Created: GPass Pipeline State Object\n");

	bool result = (m_gpassPso && m_gpassRootSig);
	return result;
}

void D12GPass::Shutdown()
{
	m_gPassMainBuffer->Release();
	m_gPassDepthBuffer->Release();
	m_dimension = m_initialDimensions;

	
	DX_SAFE_RELEASE(m_gpassRootSig);
	m_ownerRenderer->DeferredRelease(m_gpassPso);
	//DX_SAFE_RELEASE(m_gpassPso);

}

bool D12GPass::CreateBuffers(IntVec2 const& size)
{
	assert(size.x && size.y);
	if (m_gPassDepthBuffer) 
	{
		m_gPassDepthBuffer->Release();
	}
	if (m_gPassMainBuffer) 
	{
		m_gPassMainBuffer->Release();
	}

	D3D12_RESOURCE_DESC desc{};
	desc.Alignment = 0;//0 is the same as 64KB(or 4MB for MSAA);
	desc.DepthOrArraySize = 1;
	desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
	desc.Format = m_mainBufferFormat;
	desc.Width = size.x;
	desc.Height = size.y;
	desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	desc.MipLevels = 0;//Make space for all mip levels
	desc.SampleDesc = { 1,0 };

	//Create the main buffer
	{
		D12TextureInitInfo info{};
		info.m_desc = &desc;
		info.m_initialState = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		info.m_clearValue.Format = desc.Format;
		memcpy(&info.m_clearValue.Color, &clearValue[0], sizeof(clearValue));
		assert(m_ownerRenderer);
		m_gPassMainBuffer = new D12RenderTexuture(m_ownerRenderer, info);
		DebuggerPrintf("D12 Object Created: GPass Main Buffer\n");
		
	}

	desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
	desc.Format = m_depthBufferFormat;
	desc.MipLevels = 1;

	// Create the depth buffer
	{
		D12TextureInitInfo info{};
		info.m_desc = &desc;
		info.m_initialState = D3D12_RESOURCE_STATE_DEPTH_READ | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
		info.m_clearValue.Format = desc.Format;
		info.m_clearValue.DepthStencil.Depth = 0.f;
		info.m_clearValue.DepthStencil.Stencil = 0;

		m_gPassDepthBuffer = new D12DepthBuffer(m_ownerRenderer, info);
		DebuggerPrintf("D12 Object Created: GPass Depth Buffer\n");
	}

	//m_flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;

	bool result = (m_gPassMainBuffer->GetResource() && m_gPassDepthBuffer->GetResource());

	return result;
}

void D12GPass::DepthPrepass(id3d12GraphicsCommandList* cmdList, D12FrameInfo const& info)
{
	UNUSED(cmdList);
	UNUSED(info);
}

void D12GPass::Render(id3d12GraphicsCommandList* cmdList, D12FrameInfo const& info)
{
	cmdList->SetGraphicsRootSignature(m_gpassRootSig);
	cmdList->SetPipelineState(m_gpassPso);

	static u32 frame{ 0 };
	struct 
	{
		float m_width;
		float m_height;
		u32 m_frame;
	}constants{ (float)info.m_surfaceWidth,(float)info.m_surfaceHeight,++frame };

	using idx = Gpass_ROOT_PARAM_INDICES;

	cmdList->SetGraphicsRoot32BitConstants(idx::root_constants, 3, &constants, 0);
	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	cmdList->DrawInstanced(3, 1, 0, 0);
}

void D12GPass::SetRenderTargetsForDepthPrepass(id3d12GraphicsCommandList* cmdList)
{
	const D3D12_CPU_DESCRIPTOR_HANDLE dsv{ m_gPassDepthBuffer->GetDSV() };
	cmdList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 0.f, 0, 0, nullptr);
	cmdList->OMSetRenderTargets(0, nullptr, 0, &dsv);
}

void D12GPass::SetRenderTargetsForGPass(id3d12GraphicsCommandList* cmdList)
{
	const D3D12_CPU_DESCRIPTOR_HANDLE rtv{ m_gPassMainBuffer->GetRTV(0) };
	const D3D12_CPU_DESCRIPTOR_HANDLE dsv{ m_gPassDepthBuffer->GetDSV() };
	cmdList->ClearRenderTargetView(rtv, clearValue, 0, nullptr);
	cmdList->OMSetRenderTargets(1, &rtv, 0, &dsv);
}

void D12GPass::SetSize(IntVec2 size)
{
	IntVec2 d = m_dimension;
	if (size.x > d.x || size.y > d.y)
	{
		if (size.x > d.x) 
		{
			d.x = size.x;
		}
		if (size.y > d.y)
		{
			d.y = size.y;
		}
		CreateBuffers(d);
	}
}

void D12GPass::AddTransitionForDepthPrePass(D12ResourceBarrier* barriers)
{
	barriers->Add(m_gPassMainBuffer->GetResource(),
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_BARRIER_FLAG_BEGIN_ONLY);
	barriers->Add(m_gPassDepthBuffer->GetResource(),
		D3D12_RESOURCE_STATE_DEPTH_READ | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
		D3D12_RESOURCE_STATE_DEPTH_WRITE/*, m_flags*/);

	//m_flags = D3D12_RESOURCE_BARRIER_FLAG_END_ONLY;
}

void D12GPass::AddTransitionForGPass(D12ResourceBarrier* barriers)
{
	barriers->Add(m_gPassMainBuffer->GetResource(),
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_BARRIER_FLAG_END_ONLY);

	barriers->Add(m_gPassDepthBuffer->GetResource(),
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		D3D12_RESOURCE_STATE_DEPTH_READ | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
}

void D12GPass::AddTransitionForPostProcess(D12ResourceBarrier* barriers)
{
	barriers->Add(m_gPassMainBuffer->GetResource(),
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

	//barriers->Add(m_gPassDepthBuffer->GetResource(),
	//	D3D12_RESOURCE_STATE_DEPTH_READ | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
	//	D3D12_RESOURCE_STATE_DEPTH_WRITE/*, D3D12_RESOURCE_BARRIER_FLAG_BEGIN_ONLY*/);
}

#endif // ENABLE_D3D12