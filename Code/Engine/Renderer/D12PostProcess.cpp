#include "Engine/Renderer/D12PostProcess.hpp"

#ifdef ENABLE_D3D12
#include "Engine/Renderer/RendererD12.hpp"
#include "Engine/Renderer/D12Helpers.hpp"
#include "Engine/Renderer/D12Shader.hpp"
#include "Engine/Renderer/D12GPass.hpp"
#include "Engine/Renderer/D12Surface.hpp"

#define  DX_SAFE_RELEASE(dxObject)				\
{												\
	if ((dxObject)!=nullptr)					\
	{											\
		(dxObject)->Release();					\
		(dxObject) = nullptr;					\
	}											\
}												\

struct FX_ROOT_PARAM_INDICES
{
	enum : u32 {
		root_constants,
		desciptor_table,
		count
	};
};

bool D12PostProcess::Initialize()
{
	return CreateFXPosAndRootSignature();
}

void D12PostProcess::Shutdown()
{
	DX_SAFE_RELEASE(m_fxRootSig);
	m_ownerRenderer->DeferredRelease(m_fxPSO);
	//DX_SAFE_RELEASE(m_fxPSO);

}

bool D12PostProcess::CreateFXPosAndRootSignature()
{
	assert(!m_fxRootSig && !m_fxPSO);
	
	D12DescriptorRange range
	{
		D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
		D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND,0,0,
		D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE
	};

	using idx = FX_ROOT_PARAM_INDICES;
	D12RootParameter parameters[idx::count]{};
	parameters[idx::root_constants].AsConstants(1, D3D12_SHADER_VISIBILITY_PIXEL, 1);
	parameters[idx::desciptor_table].AsDescriptorTable(D3D12_SHADER_VISIBILITY_PIXEL, &range, 1);
	//Create post process root signature
	D12Helpers* helper = m_ownerRenderer->GetHelper();
	const D12RootSignatureDesc rootSignature = D12RootSignatureDesc(helper, &parameters[0], _countof(parameters));
	m_fxRootSig = rootSignature.Create();
	assert(m_fxRootSig);
	DebuggerPrintf("D12 Object Created: FX Post Processing Root Signature\n");
	//Create post process PSO

	struct {
		D12PipelineStateSubobjectrootSignature rootSignature{  };
		D12PipelineStateSubobjectvs	vs{};
		D12PipelineStateSubobjectps ps{};
		D12PipelineStateSubobjectprimitiveTopology primitiveTopology{};
		D12PipelineStateSubobjectrenderTargetFormats renderTargetFormats;
		D12PipelineStateSubobjectrasterizer rasterizer{};

	}localStream;

	localStream.rootSignature = m_fxRootSig;
	//localStream.vs = m_ownerRenderer->GetShaders()->GetEngineShaderByID(EngineShader::FULLSCREEN_TRIANGLE_VS);
	//localStream.ps = m_ownerRenderer->GetShaders()->GetEngineShaderByID(EngineShader::POST_PROCESS_PS);
	localStream.vs = m_ownerRenderer->GetShaders()->GetCompiledShaderByID(EngineShader::FULLSCREEN_TRIANGLE_VS);
	localStream.ps = m_ownerRenderer->GetShaders()->GetCompiledShaderByID(EngineShader::POST_PROCESS_PS);
	localStream.primitiveTopology = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	localStream.rasterizer = rasterizerState.noCull;

	//m_stream.rootSignature = m_gpassRootSig;
	//m_stream.vs = m_ownerRenderer->GetShaders()->GetEngineShaderByID(EngineShader::FULLSCREEN_TRIANGLE_VS);
	//m_stream.ps = m_ownerRenderer->GetShaders()->GetEngineShaderByID(EngineShader::FILL_COLOR_PS);
	//m_stream.primitiveTopology = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	//m_stream.depthStencilFormat = m_depthBufferFormat;
	//m_stream.rasterizer = rasterizerState.noCull;
	//m_stream.depth = depthState.disabled;

	D3D12_RT_FORMAT_ARRAY rtfArray{};
	rtfArray.NumRenderTargets = 1;
	rtfArray.RTFormats[0] = m_ownerRenderer->GetSurface()->DEFAULTBACKBUFFERFORMAT;
	//m_stream.renderTargetFormats = rtfArray;
	localStream.renderTargetFormats = rtfArray;


	m_fxPSO = helper->CreatePipelineState(&localStream, sizeof(localStream));
	DebuggerPrintf("D12 Object Created: FX Post Processing State Object\n");

	bool result = (m_fxPSO && m_fxRootSig);
	return result;
}


void D12PostProcess::PostProcess(id3d12GraphicsCommandList* cmdList, D3D12_CPU_DESCRIPTOR_HANDLE targetRTV)
{
	cmdList->SetGraphicsRootSignature(m_fxRootSig);
	cmdList->SetPipelineState(m_fxPSO);

	using idx = FX_ROOT_PARAM_INDICES;
	cmdList->SetGraphicsRoot32BitConstant(idx::root_constants, m_ownerRenderer->GetGPass()->GetMainBuffer()->GetSRV().GetIndex(), 0);
	cmdList->SetGraphicsRootDescriptorTable(idx::desciptor_table, m_ownerRenderer->GetSRVHeap().GetGPUStart());

	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//NOTE: Don't need to clear the render target, because each pixel will
	//		be overwrite by pixels form gPass main buffer.
	//		We also don't need a depth buffer
	cmdList->OMSetRenderTargets(1, &targetRTV, 1, nullptr);
	cmdList->DrawInstanced(3, 1, 0, 0);

}

#endif // ENABLE_D3D12