#include "Engine/Renderer/D12Content.hpp"
#include "Engine/Core/XmlUtils.hpp"
#ifdef ENABLE_D3D12
extern RendererD12* g_theRendererD12;

#include "Engine/Renderer/RendererD12.hpp"
#include "Engine/Renderer/D12Helpers.hpp"
#include "Engine/Renderer/D12Surface.hpp"
#include "Engine/Core/Vertex_PCUTBN.hpp"
#include "Engine/Core/VertexUtils.hpp"
#define  DX_SAFE_RELEASE(dxObject)				\
{												\
	if ((dxObject)!=nullptr)					\
	{											\
		(dxObject)->Release();					\
		(dxObject) = nullptr;					\
	}											\
}												\







D12Content::~D12Content()
{

}

bool D12Content::InitializeMeshResource()
{
	std::lock_guard lock{ m_meshMutex };
	for (u32 i = 0; i < ModelList::COUNT; i++)
	{
		std::string modelName = m_modelXMLNameList[i];
		Add(modelName);
	}
	AddTriangleResource();
	// Create the depth/stencil buffer and view.
	D3D12_RESOURCE_DESC depthStencilDesc;
	depthStencilDesc.Alignment = 0;
	depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthStencilDesc.Width = g_theRendererD12->GetSurface()->GetWidth();
	depthStencilDesc.Height = g_theRendererD12->GetSurface()->GetHeight();
	depthStencilDesc.DepthOrArraySize = 1;
	depthStencilDesc.MipLevels = 0;

	// Correction 11/12/2016: SSAO chapter requires an SRV to the depth buffer to read from 
	// the depth buffer.  Therefore, because we need to create two views to the same resource:
	//   1. SRV format: DXGI_FORMAT_R24_UNORM_X8_TYPELESS
	//   2. DSV Format: DXGI_FORMAT_D24_UNORM_S8_UINT
	// we need to create the depth buffer resource with a typeless format.  
	depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;

	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	// Create the depth buffer
	{
		D12TextureInitInfo info{};
		info.m_desc = &depthStencilDesc;
		info.m_initialState = D3D12_RESOURCE_STATE_DEPTH_WRITE;
		info.m_clearValue.Format = depthStencilDesc.Format;
		info.m_clearValue.DepthStencil.Depth = 1.f;
		info.m_clearValue.DepthStencil.Stencil = 0;

		m_contentDepthBuffer = new D12DepthBuffer(g_theRendererD12, info);
		NAME_D3D12_OBJECT(m_contentDepthBuffer->GetResource(), L"Content Depth Buffer");
	}


	return true;
}

void D12Content::AddTriangleResource()
{
	// Define the geometry for a triangle.
	//float aspectRatio = 2.f;// m_ownerRenderer->GetConfig().m_window->GetConfig().m_clientAspect;
	Vertex_PCUTBN one = Vertex_PCUTBN(Vertex_PNCU(Vec3(0.0f, 0.25f, 0.5f), -Vec3::Z_AXIS, Rgba8::BLACK));
	Vertex_PCUTBN three = Vertex_PCUTBN(Vertex_PNCU(Vec3(-0.25f, -0.25f, 0.5f), -Vec3::Z_AXIS, Rgba8::BLACK));
	Vertex_PCUTBN two = Vertex_PCUTBN(Vertex_PNCU(Vec3(0.25f, -0.25f, 0.5f), -Vec3::Z_AXIS, Rgba8::BLACK));
	Vertex_PCUTBN triangleVertices[] = { one, three, two };

	const UINT vertexBufferSize = sizeof(triangleVertices);

	std::vector<Vertex_PCUTBN> vBuffer;
	vBuffer.push_back(one);
	vBuffer.push_back(three);
	vBuffer.push_back(two);

	D12GPUMesh* gpuMesh = new D12GPUMesh(vBuffer);
	gpuMesh->CreateMeshAndIndexView();
	m_gpuMeshList.push_back(gpuMesh);
}

void D12Content::AddSphereModelResource()
{
	std::vector<Vertex_PCUTBN> vBuffer;
	std::vector<unsigned int> indexBuffer;
	AddVertsForUVSphereZ3D(vBuffer, indexBuffer, Vec3::Z_AXIS, 1.f, 32.f, 16.f);
	
	D12GPUMesh* gpuMesh = new D12GPUMesh(vBuffer, indexBuffer);
	gpuMesh->CreateMeshAndIndexView();
	m_gpuMeshList.push_back(gpuMesh);
}

void D12Content::Shutdown()
{
	for (u32 i = 0; i < (u32)m_rootSignatures.size(); i++) 
	{
		DX_SAFE_RELEASE(m_rootSignatures[i]);
	}
	for (u32 i = 0; i < (u32)m_pipelineStates.size(); i++)
	{
		g_theRendererD12->DeferredRelease(m_pipelineStates[i]);
	}
	

	for (u32 i = 0; i < m_gpuMeshList.size(); i++)
	{
		D12GPUMesh* mesh = m_gpuMeshList[i];
		delete mesh;
		mesh = nullptr;
	}
	for (u32 i = 0; i < m_meshViews.size(); i++)
	{
		MeshView* mesh = m_meshViews[i];
		delete mesh;
		mesh = nullptr;
	}
	for (u32 i = 0; i < m_meshVertexBuffers.size(); i++)
	{
		ID3D12Resource* mesh = m_meshVertexBuffers[i];
		g_theRendererD12->DeferredRelease(mesh);
	}
	for (u32 i = 0; i < m_meshIndexBuffers.size(); i++)
	{
		ID3D12Resource* index = m_meshIndexBuffers[i];
		g_theRendererD12->DeferredRelease(index);
	}
	m_contentDepthBuffer->Release();

	m_meshViews.clear();
	m_meshVertexBuffers.clear();
	m_meshIndexBuffers.clear();
	m_gpuMeshList.clear();
}

void D12Content::Add(std::string const& name)
{
	//std::lock_guard lock{ m_meshMutex };
	std::string modelToLoad = m_modelXMLPathPrefix + name;
	D12GPUMesh* gpuMesh = new D12GPUMesh(modelToLoad);
	gpuMesh->CreateMeshAndIndexView();
	assert(gpuMesh && gpuMesh->m_vertexMeshBuffer && gpuMesh->m_meshView);
	//m_meshViews.push_back(gpuMesh->m_meshView);
	//m_meshVertexBuffers.push_back(gpuMesh->m_vertexMeshBuffer);
	//m_meshIndexBuffers.push_back(gpuMesh->m_indexMeshBuffer);
	m_gpuMeshList.push_back(gpuMesh);
}

void D12Content::Remove(u32 Index)
{
	//std::lock_guard lock{ m_meshMutex };
	ID3D12Resource* mesh = m_meshVertexBuffers[Index];
	//g_theRendererD12->DeferredRelease(mesh);
	DX_SAFE_RELEASE(mesh);
	m_meshViews.erase(m_meshViews.begin() + Index);
	m_meshVertexBuffers.erase(m_meshVertexBuffers.begin() + Index);
	m_meshIndexBuffers.erase(m_meshIndexBuffers.begin() + Index);
	m_gpuMeshList.erase(m_gpuMeshList.begin() + Index);
}

void D12Content::Render(D3D12_CPU_DESCRIPTOR_HANDLE targetRTV)
{
	id3d12GraphicsCommandList* cmdList = g_theRendererD12->GetCmdListOwner()->GetCommandList();
	assert(cmdList);
	//cmdList->OMSetRenderTargets(1, &targetRTV, 1, nullptr);
	float array[4] = { 0.f };
	Rgba8 clearColor = g_theRendererD12->m_clearColor;
	clearColor.GetAsFloats(array);
	cmdList->ClearRenderTargetView(targetRTV, array, 0, nullptr);
	const D3D12_CPU_DESCRIPTOR_HANDLE dsv{ m_contentDepthBuffer->GetDSV() };
	cmdList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
	cmdList->OMSetRenderTargets(1, &targetRTV, 0, &dsv);


	g_theRendererD12->DrawIndexedVertexArray(m_gpuMeshList[0]);
	//g_theRendererD12->DrawIndexedVertexArray(m_gpuMeshList[1]);
	g_theRendererD12->DrawIndexedVertexArray(m_gpuMeshList[4]);

// 	for (int i = 0; i < m_meshViews.size(); i++)
// 	{
// 		g_theRendererD12->DrawIndexedVertexArray(m_gpuMeshList[i]);
// 	}
}

void D12Content::SetRootSigAnsPSO()
{
	std::lock_guard lock{ m_meshMutex };

	id3d12GraphicsCommandList* cmdList = g_theRendererD12->GetCmdListOwner()->GetCommandList();
	assert(g_theRendererD12->GetCBufferRootSig());
	cmdList->SetGraphicsRootSignature(g_theRendererD12->GetCBufferRootSig());
	ID3D12DescriptorHeap* const cHeaps[]{ g_theRendererD12->GetConstBHeap().GetHeap() };
	cmdList->SetDescriptorHeaps(1, &cHeaps[0]);

	//Connect resource with root descriptor table
	cmdList->SetGraphicsRootDescriptorTable(0, g_theRendererD12->GetConstBHeap().GetHeap()->GetGPUDescriptorHandleForHeapStart());

	//cmdList->SetGraphicsRootDescriptorTable(0, g_theRendererD12->GetConstBHeap().GetHeap()->GetGPUDescriptorHandleForHeapStart());
	//cmdList->SetGraphicsRootDescriptorTable(1, g_theRendererD12->GetConstBHeap().GetHeap()->GetGPUDescriptorHandleForHeapStart());
	//id3d12GraphicsCommandList* cmdList = g_theRendererD12->GetCmdListOwner()->GetCommandList();
	cmdList->SetGraphicsRootSignature(m_rootSignatures[PSOROOTSIG_ID::model_mesh]);
	cmdList->SetPipelineState(m_pipelineStates[PSOROOTSIG_ID::model_mesh]);
	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

}

constexpr D3D12_ROOT_SIGNATURE_FLAGS D12Content::GetRootSignatureFlags(ShaderFlags::flags flags)
{
	D3D12_ROOT_SIGNATURE_FLAGS default_flags{ D12RootSignatureDesc::defaultFlags };
	if (flags & ShaderFlags::vertex)           default_flags &= ~D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS;
	if (flags & ShaderFlags::hull)             default_flags &= ~D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;
	if (flags & ShaderFlags::domain)           default_flags &= ~D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS;
	if (flags & ShaderFlags::geometry)         default_flags &= ~D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
	if (flags & ShaderFlags::pixel)            default_flags &= ~D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;
	if (flags & ShaderFlags::amplification)    default_flags &= ~D3D12_ROOT_SIGNATURE_FLAG_DENY_AMPLIFICATION_SHADER_ROOT_ACCESS;
	if (flags & ShaderFlags::mesh)             default_flags &= ~D3D12_ROOT_SIGNATURE_FLAG_DENY_MESH_SHADER_ROOT_ACCESS;
	return default_flags;
}

bool D12Content::CreateRootSignatureAndPSOForMesh()
{
	using params = OpaqueRootParameter;
	D12RootParameter parameters[params::count]{};

	D12Helpers* helper = g_theRendererD12->GetHelper();

	//D12DescriptorRange range
	//{
	//	D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
	//	D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND,0,0,
	//	D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE
	//};
	//parameters[params::position_buffer].AsDescriptorTable(D3D12_SHADER_VISIBILITY_ALL, &range, 1);
	//parameters[params::global_shader_data].AsCBV(D3D12_SHADER_VISIBILITY_ALL, 0);
	//parameters[params::srv_indices].AsSRV(D3D12_SHADER_VISIBILITY_PIXEL, 1);
	//parameters[params::position_buffer].AsSRV(D3D12_SHADER_VISIBILITY_ALL, 0);
	//D12RootSignatureDesc rootSigDesc = D12RootSignatureDesc(helper, &parameters[0], _countof(parameters), D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
	//cmdList->SetGraphicsRootShaderResourceView()
	//ID3D12RootSignature* rootSignature = rootSigDesc.Create();

	//id3d12GraphicsCommandList* cmdList = g_theRendererD12->GetCmdListOwner()->GetCommandList();
	//assert(g_theRendererD12->GetCBufferRootSig());
	//cmdList->SetGraphicsRootSignature(g_theRendererD12->GetCBufferRootSig());
	//ID3D12DescriptorHeap* const cHeaps[]{ g_theRendererD12->GetConstBHeap().GetHeap() };
	//cmdList->SetDescriptorHeaps(1, &cHeaps[0]);
	//
	////Connect resource with root descriptor table
	//cmdList->SetGraphicsRootDescriptorTable(0, g_theRendererD12->GetConstBHeap().GetHeap()->GetGPUDescriptorHandleForHeapStart());

	ID3D12RootSignature* rootSignature = g_theRendererD12->GetCBufferRootSig();
	assert(rootSignature);
	//rootSignature->SetName(L"Mesh Root Signature");
	m_rootSignatures.push_back(rootSignature);

	D12PipelineStateSubobjectStream psoStream = D12PipelineStateSubobjectStream();

	D3D12_RT_FORMAT_ARRAY rt_array{};
	rt_array.NumRenderTargets = 1;
	rt_array.RTFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoStream.renderTargetFormats = rt_array;

	//Create Input Layout
	D3D12_INPUT_ELEMENT_DESC inputElementDesc[] = {
		{"POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT,		0,		0,								D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{"COLOR",		0, DXGI_FORMAT_R8G8B8A8_UNORM,		0,		D3D12_APPEND_ALIGNED_ELEMENT,	D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{"TEXCOORD",	0, DXGI_FORMAT_R32G32_FLOAT,		0,		D3D12_APPEND_ALIGNED_ELEMENT,	D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{"TANGENT",		0, DXGI_FORMAT_R32G32B32_FLOAT,		0,		D3D12_APPEND_ALIGNED_ELEMENT,	D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{"BINORMAL",	0, DXGI_FORMAT_R32G32B32_FLOAT,		0,		D3D12_APPEND_ALIGNED_ELEMENT,	D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{"NORMAL",		0, DXGI_FORMAT_R32G32B32_FLOAT,		0,		D3D12_APPEND_ALIGNED_ELEMENT,	D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
	};
	std::vector<D3D12_INPUT_ELEMENT_DESC> inputLayout(inputElementDesc, inputElementDesc + 4);
	psoStream.inputLayout = { inputLayout.data() , (UINT)inputLayout.size() };

	psoStream.rootSignature = rootSignature;
	psoStream.vs = g_theRendererD12->GetShaders()->GetCompiledShaderByID(EngineShader::UNLIT_VS);
	psoStream.ps = g_theRendererD12->GetShaders()->GetCompiledShaderByID(EngineShader::UNLIT_PS);
	
	psoStream.primitiveTopology = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoStream.depthStencilFormat = DXGI_FORMAT_D32_FLOAT;
	psoStream.rasterizer = rasterizerState.noCull;
	psoStream.depthStencil1 = depthState.enabled;
	psoStream.blend = blendState.alphaBlend;
	
	//psoStream.sampleMask = UINT_MAX;
	ID3D12PipelineState* pso = helper->CreatePipelineState(&psoStream, sizeof(psoStream));
	
	pso->SetName(L"Mesh pipeline states");
	m_pipelineStates.push_back(pso);

	return (rootSignature && pso);
}

#endif // ENABLE_D3D12
