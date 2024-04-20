#pragma once
#include "Engine/Renderer/RendererD12.hpp"
#ifdef ENABLE_D3D12
//#define D3D_COMPILE_STANDARD_FILE_INCLUDE ((ID3DInclude*)(UINT_PTR)1)				

#define  DX_SAFE_RELEASE(dxObject)				\
{												\
	if ((dxObject)!=nullptr)					\
	{											\
		(dxObject)->Release();					\
		(dxObject) = nullptr;					\
	}											\
}												\



#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Renderer/D12Surface.hpp"

#include "Engine/Renderer/D12Shader.hpp"
#include "Engine/Renderer/D12ShaderCompilation.hpp"
#include "Engine/Renderer/D12GPass.hpp"
#include "Engine/Renderer/D12PostProcess.hpp"
#include "Engine/Renderer/D12UploadContext.hpp"
#include "Engine/Renderer/D12Content.hpp"
#include "Engine/Renderer/RayTracingHelper.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "ThirdParty/stb/stb_image.h"

// #ifdef USE_PIX
// #include "pix3.h"
// #endif // USE_PIX


//constexpr D3D_FEATURE_LEVEL MINIMUM_FEATURE_LEVEL = D3D_FEATURE_LEVEL_11_0;
constexpr D3D_FEATURE_LEVEL MINIMUM_FEATURE_LEVEL = D3D_FEATURE_LEVEL_12_1;

RendererD12::RendererD12(RendererD12Config const& config) :m_config(config) 
{
	std::string engineShadersPath = "Data/Shaders/";
	m_theShader = new D12Shaders(engineShadersPath);
	m_usingRaster = m_config.m_isRaster;
	if (m_usingRaster) 
	{
		m_gPass = new D12GPass(this);
		m_postProcess = new D12PostProcess(this);
	}
	else
	{
		m_rayTracer = new RayTracingHelper(this);
	}

	m_helper = new D12Helpers(this);
	m_resourceBarriers = new D12ResourceBarrier();
	m_uploadContext = new D12UploadContext(this);
	m_context = new D12Content();

}


RendererD12::~RendererD12()
{
	delete m_context;
	m_context = nullptr;
}



void RendererD12::Startup()
{
	//determine which adapter to use
	//create a ID3D12Device (this is a virtual adapter)

	if (m_device) Shutdown();
	
	unsigned int dxgiFactoryFlags = 0;
#ifdef _DEBUG
	//Enable debugging layer.
	{
		ComPtr<ID3D12Debug3> debugInterface;
		HRESULT hrseult = D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface));
		if (!SUCCEEDED(hrseult))
		{
			//ERROR_AND_DIE("Could not Enable Debugging Interface.");
			DebuggerPrintf("Debug layer is not available.\n");
		}
		debugInterface->EnableDebugLayer();
#if 1
#pragma message("Warning: GPU based validation is enabled. This will considerable slow down the render!")
		debugInterface->SetEnableGPUBasedValidation(1);
#endif

		dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
	}
#endif // _DEBUG
	HRESULT hr = S_OK;
	hr = CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&m_dxgiFactory));
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("Could not create D3D 12 DXGIFactory.");
	}

	//determine what is the maximum feature level that is supported
	ComPtr<IDXGIAdapter1> mainAdapter;
	mainAdapter.Attach(DetermineMainAdapter());

	if (!mainAdapter)
	{
		ERROR_AND_DIE("Could not attach or determine main adapter.");
	}

	D3D_FEATURE_LEVEL maxFeatureLevel = GetMaxFeatureLevel(mainAdapter.Get());
	if (maxFeatureLevel < MINIMUM_FEATURE_LEVEL)
	{
		//ERROR_AND_DIE("Final check for main device.");
		ERROR_AND_DIE("D3D_FEATURE_LEVEL_12_1 is not supported");
	}

	hr = D3D12CreateDevice(mainAdapter.Get(), maxFeatureLevel, IID_PPV_ARGS(&m_device));
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("Could not create D3D12 Device.");
	}

	m_device->SetName(L"MAIN DEVICE");

	DebuggerPrintf("MAIN DEVICE IS CREATED.\n");
	
#ifdef _DEBUG
	{
		ComPtr<ID3D12InfoQueue> infoQueue;
		HRESULT hr0 = m_device->QueryInterface(IID_PPV_ARGS(&infoQueue));
		if (!SUCCEEDED(hr0))
		{
			ERROR_AND_DIE("Could not info Queue.");
		}

		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);

	}
#endif // _DEBUG

	bool result = true;

	result &= m_rtvDescHeap.Initialize(512, false);
	result &= m_dsvDescHeap.Initialize(512, false);
	result &= m_srvDescHeap.Initialize(4096, true);
	result &= m_uavDescHeap.Initialize(512, true);
	result &= m_constBufferHeap.Initialize(8, true);
	if (!result)
	{
		ERROR_AND_DIE("Heaps initialization failed.");
	}
	DebuggerPrintf("D12 Object Created: RTV descriptor Heap\n");
	DebuggerPrintf("D12 Object Created: DSV descriptor Heap\n");
	DebuggerPrintf("D12 Object Created: SRV descriptor Heap\n");
	DebuggerPrintf("D12 Object Created: UAV descriptor Heap\n");

	for (u32 i = 0; i < FRAME_BUFFER_COUNT; i++)
	{
		D12BufferInitInfo bIInfo = ConstantBuffer::GetDefaultInitInfo((u32)(1024));
		new(&m_constantBuffers[i]) ConstantBuffer{ this, bIInfo };
		m_constantBuffers[i].GetBuffer()->SetName(L"Global Constant Buffer " + i);
	}
	if (!m_usingRaster) 
	{
		CheckRaytracingSupport();
	}
	
	bool resultShaderStartUp = m_theShader->Startup();
	bool resultShaderCompilation = m_theShader->CompileShaders();
	//new(&m_gfxCommand) D12Command(m_device, D3D12_COMMAND_LIST_TYPE_DIRECT);

	bool cResult = CreateRootSigForCameraConstant();
	assert(cResult);
	GUARANTEE_OR_DIE(cResult, "Camera Constant is valid initialized! ");
	if (!m_usingRaster) 
	{
		//m_rayTracer->m_cameraRootSignature = m_constantBuffersRootSignature;
		m_rayTracer->m_device = m_device;
		m_rayTracer->InitialPipelineStates();
		m_gfxCommand = new D12Command(m_device, D3D12_COMMAND_LIST_TYPE_DIRECT, m_rayTracer->m_pipelineState.Get());
	}
	else
	{
		m_gfxCommand = new D12Command(m_device, D3D12_COMMAND_LIST_TYPE_DIRECT);
	}



	m_rtvDescHeap.GetHeap()->SetName(L"RTV Descriptor Heap");
	m_dsvDescHeap.GetHeap()->SetName(L"DSV Descriptor Heap");
	m_srvDescHeap.GetHeap()->SetName(L"SRV Descriptor Heap");
	m_uavDescHeap.GetHeap()->SetName(L"UAV Descriptor Heap");
	m_constBufferHeap.GetHeap()->SetName(L"CBuffer Descriptor Heap");
	CreateSurface(m_config.m_window);
	//CreateARootSignature();
	//CreateARootSignatureWithHelper();



	if (m_usingRaster) 
	{
		bool resultPostProcess = m_postProcess->Initialize();
		bool resultGPassInitialization = m_gPass->Initialize();

		if (!resultGPassInitialization || !resultPostProcess)
		{
			ERROR_AND_DIE("Rasterization intialization failed");
		}

		D12FrameInfo frameInfo;
		frameInfo.m_surfaceHeight = m_surface->GetHeight();
		frameInfo.m_surfaceWidth = m_surface->GetWidth();
		IntVec2 WidthAndHeight = IntVec2(frameInfo.m_surfaceWidth, frameInfo.m_surfaceHeight);
		m_gPass->SetSize(WidthAndHeight);
	}

	bool resultUploadInitialization = m_uploadContext->Initialize();
	bool resultContext = m_context->InitializeMeshResource();
	if (!resultUploadInitialization || !resultContext || !resultShaderStartUp || !resultShaderCompilation) 
	{
		ERROR_AND_DIE("One of the Modules intialization failed");
	}
	



	////Create descriptor heap to save the view
	//D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc = {};
	//cbvHeapDesc.NumDescriptors = 1;
	//cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	//cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	//m_device->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(&m_constBufferDescHeap));
	//Create constant buffer view
	//ModelConstantsD12 initialModelConstants = {};
	//u32 sizeOfModel = (u32)sizeof(ModelConstantsD12);
	//for (u32 i = 0; i < FRAME_BUFFER_COUNT; i++) 
	//{
	//	ConstantBuffer& cBuffer = m_constantBuffers[i];
	//	CameraConstantsD12* currentDataPointer = cBuffer.Allocate<CameraConstantsD12>();
	//	assert(currentDataPointer);
	//	memcpy(currentDataPointer, &initialCameraConstants, size);
	//
	//	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
	//	cbvDesc.BufferLocation = cBuffer.GetGPUAddress();
	//	cbvDesc.SizeInBytes = size;
	//	m_device->CreateConstantBufferView(&cbvDesc, m_constBufferHeap.GetCPUStart());
	//}

	CameraConstantsD12 initialCameraConstants = {};
	u32 sizeOfCamera = (u32)sizeof(CameraConstantsD12);
	D12BufferInitInfo bIInfo2 = ConstantBuffer::GetDefaultInitInfo((u32)(1024));
	new(&m_tryCB) ConstantBuffer{ this, bIInfo2 };

	NAME_D3D12_OBJECT(m_tryCB.GetBuffer(), L"Try Constant Buffer");
	CameraConstantsD12* currentTryDataPointer = m_tryCB.Allocate<CameraConstantsD12>();

	assert(currentTryDataPointer);
	memcpy(currentTryDataPointer, &initialCameraConstants, sizeOfCamera);
	//ModelConstantsD12* currentModelDataPointer = m_tryCB.Allocate<ModelConstantsD12>();
	//assert(currentModelDataPointer);
	//memcpy(currentModelDataPointer, &initialModelConstants, sizeOfModel);

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvCameraDesc = {};
	cbvCameraDesc.BufferLocation = m_tryCB.GetGPUAddress();
	cbvCameraDesc.SizeInBytes = sizeOfCamera/* + sizeOfModel*/;

	
	m_device->CreateConstantBufferView(&cbvCameraDesc, m_constBufferHeap.GetCPUStart());


	if (m_usingRaster) 
	{
		m_context->CreateRootSignatureAndPSOForMesh();
		m_gfxCommand->ExecuteCmdLists();
		m_gfxCommand->GetCommandList()->Close();
	}
	else
	{
		//m_rayTracer->LoadContextAssets(m_context->m_gpuMeshList[0]);
		m_rayTracer->AddSphereModelResource();
		//m_rayTracer->AddTeapotModelResource();
		bool resultRayTracing = m_rayTracer->Initialize();
		GUARANTEE_OR_DIE((resultRayTracing), "Initilizaiton of Ray tracing failed!");
	}
}



void RendererD12::Shutdown()
{

	//Shutdown the modules
	m_uploadContext->ShutDown();
	if (m_usingRaster) 
	{
		m_postProcess->Shutdown();
		m_gPass->Shutdown();
	}
	else
	{

	}

	m_context->Shutdown();

	RemoveSurface();
	m_gfxCommand->Release();
	
	for (u32 i = 0; i < FRAME_BUFFER_COUNT; i++) 
	{
		m_constantBuffers[i].Release();
	}

	m_tryCB.Release();
	//DX_SAFE_RELEASE(m_constantBuffersRootSignature);

	//Note: we don't call process deferred releases at the end 
	//because some resources(such as swap chain) can't be released
	//before their depending resources are released
	for (u32 i = 0; i < FRAME_BUFFER_COUNT; ++i) 
	{
		ProcessDeferredReleases(i);
	}

	m_theShader->Shutdown();


	DX_SAFE_RELEASE(m_dxgiFactory);

	m_rtvDescHeap.ProcessDeferredFree(0);
	m_dsvDescHeap.ProcessDeferredFree(0);
	m_srvDescHeap.ProcessDeferredFree(0);
	m_uavDescHeap.ProcessDeferredFree(0);
	m_constBufferHeap.ProcessDeferredFree(0);
	m_rtvDescHeap.Release();
	m_dsvDescHeap.Release();
	m_srvDescHeap.Release();
	m_uavDescHeap.Release();
	m_constBufferHeap.Release();
	ProcessDeferredReleases(0);
	
#ifdef _DEBUG
	{ 
		{
			ComPtr<ID3D12InfoQueue> infoQueue;
			HRESULT hr0 = m_device->QueryInterface(IID_PPV_ARGS(&infoQueue));
			if (!SUCCEEDED(hr0))
			{
				ERROR_AND_DIE("Could not info Queue.");
			}

			infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, false);
			infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, false);
			infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, false);
		}

		ComPtr<ID3D12DebugDevice2> debugDevice;
		HRESULT hr1 = m_device->QueryInterface(IID_PPV_ARGS(&debugDevice));
		if (!SUCCEEDED(hr1))
		{
			ERROR_AND_DIE("Could not Create Debug Device Object.");
		}

		DX_SAFE_RELEASE(m_device);
		hr1 = debugDevice->ReportLiveDeviceObjects(D3D12_RLDO_SUMMARY | D3D12_RLDO_DETAIL | D3D12_RLDO_IGNORE_INTERNAL);
		if (!SUCCEEDED(hr1))
		{
			ERROR_AND_DIE("Could not Create Report Live Device Objects.");
		}
		
	}
#endif // _DEBUG

	DX_SAFE_RELEASE(m_device);

}




void RendererD12::RenderSurface()
{

	id3d12GraphicsCommandList* cmdList = m_gfxCommand->GetCommandList();
	u32 frameIdx = GetCurrentFrameIndex();
	if (m_deferredReleasesFlag[frameIdx])
	{
		ProcessDeferredReleases(frameIdx);
	}
	ID3D12Resource* const currentBackBuffer = m_surface->GetBackBuffer();

	D12FrameInfo frameInfo;
	frameInfo.m_surfaceHeight = m_surface->GetHeight();
	frameInfo.m_surfaceWidth = m_surface->GetWidth();


	//Record commands
	cmdList->RSSetViewports(1, &m_surface->GetViewport());
	cmdList->RSSetScissorRects(1, &m_surface->GetScissorRect());

	if (m_usingRaster) 
	{
		ID3D12DescriptorHeap* const heaps[]{ GetSRVHeap().GetHeap() };
		cmdList->SetDescriptorHeaps(1, &heaps[0]);

		//Depth Pre-pass
		m_resourceBarriers->Add(currentBackBuffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_BARRIER_FLAG_BEGIN_ONLY);
		m_gPass->AddTransitionForDepthPrePass(m_resourceBarriers);
		m_resourceBarriers->Add(m_context->GetDeepthStencilResource(), D3D12_RESOURCE_STATE_DEPTH_WRITE,
			D3D12_RESOURCE_STATE_DEPTH_READ | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		//,D3D12_RESOURCE_BARRIER_FLAG_BEGIN_ONLY);
		m_resourceBarriers->Apply(cmdList);


		m_gPass->SetRenderTargetsForDepthPrepass(cmdList);
		m_gPass->DepthPrepass(cmdList, frameInfo);

		//Geometry and lighting pass

		m_gPass->AddTransitionForGPass(m_resourceBarriers);
		m_resourceBarriers->Apply(cmdList);

		m_gPass->SetRenderTargetsForGPass(cmdList);
		m_gPass->Render(cmdList, frameInfo);

		//Post process
		m_resourceBarriers->Add(currentBackBuffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_BARRIER_FLAG_END_ONLY);


		m_gPass->AddTransitionForPostProcess(m_resourceBarriers);
		m_resourceBarriers->Apply(cmdList);
		//Will write to the current back buffer,so back buffer is a render target

		m_postProcess->PostProcess(cmdList, m_surface->GetRTV());

		//After post process

		//SetModelConstants(m_player->GetModelMatrix());

		m_context->SetRootSigAnsPSO();
		m_resourceBarriers->Add(m_context->GetDeepthStencilResource(), D3D12_RESOURCE_STATE_DEPTH_READ | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_DEPTH_WRITE);
		m_resourceBarriers->Apply(cmdList);

		//add barrier before draw


		RenderContextWithRasterization();
	}
	else
	{
		m_resourceBarriers->Add(currentBackBuffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
		m_resourceBarriers->Apply(cmdList);
		RenderContextWithRaytracing();
	}




	m_helper->TransitionResource(cmdList, currentBackBuffer,
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PRESENT);
}

void __declspec(noinline)RendererD12::ProcessDeferredReleases(u32 frameIndex)
{
	//m_deferredReleasesMutex.lock();
	std::lock_guard lock{ m_deferredReleasesMutex };
	//Note: we clear this flag in the beginning. If we'd clear it at the end
	//	then it might overwrite some other thread that was trying to set it.
	// it's fine if overwriting happens before processing the items.


	m_deferredReleasesFlag[frameIndex] = 0;

	m_rtvDescHeap.ProcessDeferredFree(frameIndex);
	m_dsvDescHeap.ProcessDeferredFree(frameIndex);
	m_srvDescHeap.ProcessDeferredFree(frameIndex);
	m_uavDescHeap.ProcessDeferredFree(frameIndex);
	//if (m_usingRaster) 
	//{
	//	m_rayTracer->m_srvUavHeap->ProcessDeferredFree(frameIndex);
	//}
	//release pending resources
	std::vector<IUnknown*>& resources = m_deferredReleases[frameIndex];
	if (!resources.empty()) 
	{
		for (auto& resource : resources) 
		{
			if (resource != nullptr) 
			{
				DX_SAFE_RELEASE(resource);
			}
		}
		//for (int resourceIndex = 0; resourceIndex < (int)resources.size(); resourceIndex++) 
		//{
		//	DX_SAFE_RELEASE(resources[resourceIndex]);
		//}
		resources.clear();
	}

}

void RendererD12::DeferredRelease(IUnknown* resource)
{
	if (resource)
	{
		u32 frameIndex = GetCurrentFrameIndex();

		std::lock_guard lock{ m_deferredReleasesMutex };
		m_deferredReleases[frameIndex].push_back(resource);
		SetDefferredReleasesFlag();
		resource = nullptr;

	}
}

void RendererD12::BeginFrame()
{
	if (m_usingRaster) 
	{
		m_gfxCommand->BeginFrame(nullptr);
	}
	else
	{
		m_gfxCommand->BeginFrame(m_rayTracer->m_pipelineState.Get());
	}
	
	const u32 frameIdx = GetCurrentFrameIndex();

	// Reset (clear) the global constant buffer for the current frame.
	ConstantBuffer& cBuffer = m_constantBuffers[frameIdx];
	cBuffer.Clear();

	m_tryCB.Clear();
}

void RendererD12::EndFrame()
{
	m_gfxCommand->EndFrame(m_surface);
}

void RendererD12::BeginCamera(const Camera& camera)
{
	//ZeroMemory(&cbColorMultiplierData, sizeof(cbColorMultiplierData));
	CameraConstantsD12 localCameraConstants = {};
	Mat44 ProjectionMatrixInversed = camera.GetProjectionMatrix().Inverse();
	Mat44 ViewMatrixInversed = camera.GetViewMatrix().Inverse();
	localCameraConstants.ProjectionMatrix = camera.GetProjectionMatrix();
	localCameraConstants.ViewMatrix = camera.GetViewMatrix();

	localCameraConstants.ProjectionMatrixInverse = ProjectionMatrixInversed;
	localCameraConstants.ViewMatrixInverse = ViewMatrixInversed;
	u32 size = (u32)sizeof(CameraConstantsD12);

	//const u32 frameIdx = GetCurrentFrameIndex();
	//ConstantBuffer& cBuffer = m_constantBuffers[frameIdx];

	//CameraConstantsD12* currentDataPointer = cBuffer.Allocate<CameraConstantsD12>();
	//assert(currentDataPointer);
	//memcpy(cBuffer.GetCPUAddress(), &localCameraConstants, size);

	//For rasterization
	if (m_usingRaster) 
	{
		memcpy(m_tryCB.GetCPUAddress(), &localCameraConstants, size);

	}
	else
	{
		m_rayTracer->UpdateCameraBuffer(localCameraConstants);
		LightConstantsD12 localLitConstants = {};
		localLitConstants.LightPosiOne = m_pointLightPositionOne;
		localLitConstants.Roughness = m_roughness;
		localLitConstants.Metallic = m_metallic;
		localLitConstants.AmbientLightDir = m_ambientLightDir.GetNormalized();
		localLitConstants.AmbientLightIntensity = m_ambientLightIntensity;
		float rgbaFloat[4] = { 0 };
		m_ambientLightColor.GetAsFloats(rgbaFloat);
		Vec3 ambientLColor = Vec3(rgbaFloat[0], rgbaFloat[1], rgbaFloat[2]);
		localLitConstants.AmbientLightColorInFloat = ambientLColor;
		localLitConstants.LightReflectanceModelType = m_lightModelIndex;

		float offset = 0.2f;

		localLitConstants.LightPosiTwo = m_pointLightPositionOne + Vec3(offset, offset, 0.f);
		localLitConstants.FalloffStart = 0.5f;
		localLitConstants.FalloffEnd = 10.f;
		localLitConstants.LightPosiThree = m_pointLightPositionOne + Vec3(-offset, -offset, 0.f);
		//localLitConstants.SphereTwoMatrix = m_rayTracer->m_sphereModel_Two.GetModelMatrix();
		//localLitConstants.PotMatrix = m_rayTracer->m_potModel.GetModelMatrix();
		localLitConstants.mipmapValue = m_mipmapTester;
		ModelConstantsD12 localModelConsts = {};
		localModelConsts.ModelMatrix1 = m_rayTracer->m_sphereModel_One.GetModelMatrix();
		localModelConsts.ModelMatrix2 = m_rayTracer->m_sphereModel_Two.GetModelMatrix();
		localModelConsts.ModelMatrix3 = m_rayTracer->m_potModel.GetModelMatrix();
		localModelConsts.ModelMatrix4 = m_rayTracer->m_sphereModelRed.GetModelMatrix();
		ConstantsD12 constD12;
		constD12.lightCD12 = localLitConstants;
		constD12.modelCD12 = localModelConsts;
		m_rayTracer->UpdateLightBuffer(constD12);


		m_rayTracer->UpdateTopLevelAS();
	}
	



}

void RendererD12::EndCamera(const Camera& camera)
{
	//const u32 frameIdx = GetCurrentFrameIndex();
	//ConstantBuffer& cBuffer = m_constantBuffers[frameIdx];
	//ZeroMemory(&cbColorMultiplierData, sizeof(cbColorMultiplierData));
	UNUSED(camera);
}

void RendererD12::DrawIndexedVertexArray(D12GPUMesh* gpuMeshWithView)
{
	assert(gpuMeshWithView);
	id3d12GraphicsCommandList* cmdList = m_gfxCommand->GetCommandList();
	assert(cmdList);
	MeshView* meshView = gpuMeshWithView->m_meshView;

	cmdList->IASetVertexBuffers(0, 1, &meshView->m_vertexBufferView);
	cmdList->IASetIndexBuffer(&meshView->m_indexBufferView);
	cmdList->IASetPrimitiveTopology(meshView->m_primitiveTopology);
	cmdList->DrawIndexedInstanced(gpuMeshWithView->m_indexCount, 1, 0, 0, 0);

}

void RendererD12::RenderContextWithRasterization()
{
	m_context->Render(m_surface->GetRTV());	
}

void RendererD12::RenderContextWithRaytracing()
{
	id3d12GraphicsCommandList* cmdList = GetCmdListOwner()->GetCommandList();
	assert(cmdList);

	float array[4] = { 0.f };
	Rgba8 clearColor = Rgba8::GRAY;
	clearColor.GetAsFloats(array);
	cmdList->ClearRenderTargetView(m_surface->GetRTV(), array, 0, nullptr);
	//const D3D12_CPU_DESCRIPTOR_HANDLE dsv{ m_context->m_contentDepthBuffer->GetDSV() };
	//cmdList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	//cmdList->SetGraphicsRootSignature(m_constantBuffersRootSignature);
	D3D12_CPU_DESCRIPTOR_HANDLE targetRTV = m_surface->GetRTV();
	cmdList->OMSetRenderTargets(1, &targetRTV, FALSE, nullptr);

	m_rayTracer->OnRender();

}

// void RendererD12::SetModelConstants(const Mat44& modelMatrix /*= Mat44()*/, const Rgba8& modelColor /*= Rgba8::WHITE*/)
// {
// 	ModelConstantsD12 localModelContants = {};
// 	localModelContants.ModelMatrix1 = modelMatrix;
// 	modelColor.GetAsFloats(localModelContants.ModeColor);
// 	size_t size = (size_t)sizeof(ModelConstantsD12);
// 
// 	const u32 frameIdx = GetCurrentFrameIndex();
// 	ConstantBuffer& cBuffer = m_constantBuffers[frameIdx];
// 
// 	ModelConstantsD12* currentDataPointer = cBuffer.Allocate<ModelConstantsD12>();
// 	assert(currentDataPointer);
// 	memcpy(currentDataPointer, &localModelContants, size);
// 
// 	m_gfxCommand->GetCommandList()->SetGraphicsRootConstantBufferView(1, cBuffer.GetGPUAddress(currentDataPointer));
// }

void RendererD12::CreateSurface(Window* window)
{
	m_surface = new D12Surface(this, window);
	m_surface->CreateSwapChain(m_dxgiFactory, m_gfxCommand->GetCommandQueue());
}

void RendererD12::RemoveSurface()
{
	m_gfxCommand->Flush();
	m_surface->~D12Surface();
}

void RendererD12::CreateUploadHeapForAllTextures()
{

}

D12Texture* RendererD12::CreateTextureFromFile(char const* imageFilePath)
{
	//IntVec2 dimensions = IntVec2(0, 0);// IntVec2::ZERO;		// This will be filled in for us to indicate image width & height
	//int bytesPerTexel = 0; // This will be filled in for us to indicate how many color components the image had (e.g. 3=RGB=24bit, 4=RGBA=32bit)
	//int numComponentsRequested = 0; // don't care; we support 3 (24-bit RGB) or 4 (32-bit RGBA)

	// Load (and decompress) the image RGB(A) bytes from a file on disk into a memory buffer (array of bytes)
	stbi_set_flip_vertically_on_load(1); // We prefer uvTexCoords has origin (0,0) at BOTTOM LEFT
	//unsigned char* texelData = stbi_load(imageFilePath, &dimensions.x, &dimensions.y, &bytesPerTexel, numComponentsRequested);
	// Check if the load was successful
	Image image = Image(imageFilePath);
	GUARANTEE_OR_DIE(image.GetRawData(), Stringf("Failed to load image \"%s\"", imageFilePath));


	D12Texture* newTexture = CreateTextureFromImage(image);

	// Free the raw image textural data now that we've sent a copy of it down to the GPU to be stored in video memory
	//stbi_image_free(image);

	return newTexture;
}

D12Texture* RendererD12::CreateTextureFromImage(const Image& image)
{
	//Create texture info
	D12TextureInitInfo texInitInfo{};
	//D3D12_RESOURCE_DESC* texture2DDesc = texInitInfo.m_desc;
	texInitInfo.m_desc = new D3D12_RESOURCE_DESC();
	texInitInfo.m_desc->Width = image.GetDimensions().x;
	texInitInfo.m_desc->Height = image.GetDimensions().y;
	texInitInfo.m_desc->MipLevels = 1;
	texInitInfo.m_desc->DepthOrArraySize = 1;
	texInitInfo.m_desc->SampleDesc.Count = 1; // This is the number of samples per pixel, we just want 1 sample
	texInitInfo.m_desc->SampleDesc.Quality = 0;
	texInitInfo.m_desc->Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texInitInfo.m_desc->Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

	texInitInfo.m_desc->Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN; // The arrangement of the pixels. Setting to unknown lets the driver choose the most efficient one
	texInitInfo.m_desc->Flags = D3D12_RESOURCE_FLAG_NONE; // no flags

	// create a default heap where the upload heap will copy its contents into (contents being the texture)
	DXCall(m_device->CreateCommittedResource(&HeapProperties.defaultHeap// upload direct to GPU
		, D3D12_HEAP_FLAG_NONE,
		texInitInfo.m_desc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&texInitInfo.m_resource)
	));


	int bytePerPixel = 4; //Format = DXGI_FORMAT_R8G8B8A8_UNORM, 8 * 4;

	TextureSizeDesc textureSizeDesc;
	textureSizeDesc.m_numBytesPerPixel = bytePerPixel;
	textureSizeDesc.m_width = image.GetDimensions().x;
	textureSizeDesc.m_height = image.GetDimensions().y;
// 	//m_texturesInfo.push_back(textureSizeDesc);
// 	//#ToDo: Refactor this later
// 	if (m_textureBufferUploadHeap)
// 	{
// 		DX_SAFE_RELEASE(m_textureBufferUploadHeap);
// 	}
// 	u32 textureHeapSize = ((((textureSizeDesc.m_width * textureSizeDesc.m_numBytesPerPixel) + 255) & ~255) * (textureSizeDesc.m_height - 1)) + (textureSizeDesc.m_width * textureSizeDesc.m_numBytesPerPixel);
	// now we create an upload heap to upload our texture to the GPU
	UINT64 textureUploadBufferSize;
	m_device->GetCopyableFootprints(texInitInfo.m_desc, 0, 1, 0, nullptr, nullptr, nullptr, &textureUploadBufferSize);

	D3D12_RESOURCE_DESC resDesc = CD3DX12_RESOURCE_DESC::Buffer(textureUploadBufferSize);

	DXCall(m_device->CreateCommittedResource(&HeapProperties.uploadHeap, // upload heap
		D3D12_HEAP_FLAG_NONE, // no flags
		&resDesc, // resource description for a buffer (storing the image data in this heap just to copy to the default heap)
		D3D12_RESOURCE_STATE_GENERIC_READ, // We will copy the contents from this heap to the default heap above
		nullptr,
		IID_PPV_ARGS(&m_textureBufferUploadHeap)));


	unsigned int bytesPerRow = unsigned int(texInitInfo.m_desc->Width) * bytePerPixel; // number of bytes in each row of the image data
	unsigned int imageSize = bytesPerRow * unsigned int(texInitInfo.m_desc->Height); // total image size in bytes

	// store vertex buffer in upload heap
	D3D12_SUBRESOURCE_DATA textureData = {};
	textureData.pData = image.GetRawData(); // pointer to our image data
	textureData.RowPitch = bytesPerRow; // size of all resource
	textureData.SlicePitch = imageSize; // also the size of our triangle vertex data

	id3d12GraphicsCommandList* cmdList = m_gfxCommand->GetCommandList();
	GUARANTEE_OR_DIE(texInitInfo.m_resource, "Resource is empty!");
	UpdateSubresources(cmdList, texInitInfo.m_resource, m_textureBufferUploadHeap, 0, 0, 1, &textureData);

	m_resourceBarriers->Add(texInitInfo.m_resource, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);
	m_resourceBarriers->Apply(cmdList);

	// now we create a shader resource view (descriptor that points to the texture and describes it)
	texInitInfo.m_srvDesc = new D3D12_SHADER_RESOURCE_VIEW_DESC{};
	texInitInfo.m_srvDesc->Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	texInitInfo.m_srvDesc->Format = texInitInfo.m_desc->Format;
	texInitInfo.m_srvDesc->ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	texInitInfo.m_srvDesc->Texture2D.MipLevels = 1;

	texInitInfo.m_isRayTracingTex = true;

	D12Texture* texture = new D12Texture(this, texInitInfo);

	m_textureArray.push_back(texture);

	m_gfxCommand->ExecuteCmdLists();

	return texture;
}

D12Texture* RendererD12::CreateTextureCubeFromFile(std::vector<char const*> imageFilePathList)
{

	// Load (and decompress) the image RGB(A) bytes from a file on disk into a memory buffer (array of bytes)
	stbi_set_flip_vertically_on_load(0); // We prefer uvTexCoords has origin (0,0) at BOTTOM LEFT
	u32 textureNum = (unsigned int)imageFilePathList.size();
	std::vector<Image*> imageList;
	for (u32 imageIndex = 0; imageIndex < textureNum; imageIndex++) 
	{
		// Check if the load was successful
		Image* image =new Image(imageFilePathList[imageIndex]);
		imageList.push_back(image);
		GUARANTEE_OR_DIE(image, Stringf("Failed to load image \"%s\"", imageFilePathList[imageIndex]));
	}

	D12Texture* newTexture = CreateTextureCubeFromImage(imageList);

	// Free the raw image textural data now that we've sent a copy of it down to the GPU to be stored in video memory
	//stbi_image_free(texelData);

	return newTexture;
}




D12Texture* RendererD12::CreateTextureCubeFromImage(std::vector<Image*> imageList)
{
	int imageNum = (int)imageList.size();

	//Create texture info
	D12TextureInitInfo texInitInfo{};
	texInitInfo.m_desc = new D3D12_RESOURCE_DESC();
	texInitInfo.m_desc->Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	texInitInfo.m_desc->Alignment = 0;
	texInitInfo.m_desc->Width = imageList[0]->GetDimensions().x;
	texInitInfo.m_desc->Height = imageList[0]->GetDimensions().y;
	texInitInfo.m_desc->DepthOrArraySize = imageNum;
	texInitInfo.m_desc->MipLevels = 1;
	texInitInfo.m_desc->Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texInitInfo.m_desc->SampleDesc.Count = 1; // This is the number of samples per pixel, we just want 1 sample
	texInitInfo.m_desc->SampleDesc.Quality = 0;
	texInitInfo.m_desc->Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN; // The arrangement of the pixels. Setting to unknown lets the driver choose the most efficient one
	texInitInfo.m_desc->Flags = D3D12_RESOURCE_FLAG_NONE; // no flags

	// create a default heap where the upload heap will copy its contents into (contents being the texture)
	DXCall(m_device->CreateCommittedResource(&HeapProperties.defaultHeap// upload direct to GPU
		, D3D12_HEAP_FLAG_NONE,
		texInitInfo.m_desc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&texInitInfo.m_resource)
	));

	int bytePerPixel = 4; //Format = DXGI_FORMAT_R8G8B8A8_UNORM, 8 * 4;

	TextureSizeDesc textureSizeDesc;
	textureSizeDesc.m_numBytesPerPixel = bytePerPixel;
	textureSizeDesc.m_width = imageList[0]->GetDimensions().x;
	textureSizeDesc.m_height = imageList[0]->GetDimensions().y;
	
	// now we create an upload heap to upload our texture to the GPU
	UINT64 textureUploadBufferSize;
	m_device->GetCopyableFootprints(texInitInfo.m_desc, 0, imageNum, 0, nullptr, nullptr, nullptr, &textureUploadBufferSize);

	D3D12_RESOURCE_DESC resDesc = CD3DX12_RESOURCE_DESC::Buffer(textureUploadBufferSize);

	DXCall(m_device->CreateCommittedResource(&HeapProperties.uploadHeap, // upload heap
		D3D12_HEAP_FLAG_NONE, // no flags
		&resDesc, // resource description for a buffer (storing the image data in this heap just to copy to the default heap)
		D3D12_RESOURCE_STATE_GENERIC_READ, // We will copy the contents from this heap to the default heap above
		nullptr,
		IID_PPV_ARGS(&m_textureBufferUploadHeap)));

	unsigned int bytesPerRow = unsigned int(texInitInfo.m_desc->Width) * bytePerPixel; // number of bytes in each row of the image data
	unsigned int imageSize = bytesPerRow * unsigned int(texInitInfo.m_desc->Height); // total image size in bytes

	id3d12GraphicsCommandList* cmdList = m_gfxCommand->GetCommandList();
	GUARANTEE_OR_DIE(texInitInfo.m_resource, "Resource address is null!");

	//std::vector<Rgba8> totalTexelRgba8Data;
	//CombineImageRawData(imageList, totalTexelRgba8Data);
	std::vector<D3D12_SUBRESOURCE_DATA> textureDataArray;
	for (int texIndex = 0; texIndex < imageNum; texIndex++) 
	{
		D3D12_SUBRESOURCE_DATA textureData = {};
		textureData.pData = imageList[texIndex]->GetRawData(); // pointer to our image data
		textureData.RowPitch = bytesPerRow; // number of bytes in each row of the image data
		textureData.SlicePitch = imageSize; // total image size in bytes
		textureDataArray.push_back(textureData);
	}

	// store vertex buffer in upload heap


	UpdateSubresources(cmdList, texInitInfo.m_resource, m_textureBufferUploadHeap, 0, 0, 6, textureDataArray.data());
	m_resourceBarriers->Add(texInitInfo.m_resource, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);
	m_resourceBarriers->Apply(cmdList);

	// now we create a shader resource view (descriptor that points to the texture and describes it)
	texInitInfo.m_srvDesc = new D3D12_SHADER_RESOURCE_VIEW_DESC{};
	texInitInfo.m_srvDesc->Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	texInitInfo.m_srvDesc->ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
	texInitInfo.m_srvDesc->TextureCube.MostDetailedMip = 0;
	texInitInfo.m_srvDesc->TextureCube.MipLevels = 1;
	texInitInfo.m_srvDesc->TextureCube.ResourceMinLODClamp = 0.f;
	texInitInfo.m_srvDesc->Format = texInitInfo.m_desc->Format;

	texInitInfo.m_isRayTracingTex = true;
	D12Texture* texture = new D12Texture(this, texInitInfo);
	m_textureArray.push_back(texture);

	m_gfxCommand->ExecuteCmdLists();
	return texture;
}

void RendererD12::CombineImageRawData(std::vector<Image*> imageList, std::vector<Rgba8>& texelRgba8Data)
{
	for (int subResIndex = 0; subResIndex < imageList.size(); subResIndex++) 
	{
		std::vector<Rgba8> curImageData = imageList[subResIndex]->GeTexelData();
		for (int rgbaIndex = 0; rgbaIndex < (int)curImageData.size(); rgbaIndex++) 
		{
			texelRgba8Data.emplace_back(curImageData[rgbaIndex]);
		}
	}
}

bool RendererD12::CreateRootSigForCameraConstant()
{
	//Create root parameter(Descriptor table with one root parameter) without resources yet
	using idx = CAMERA_ROOT_PARAM_INDICES;
	D12DescriptorRange descriptorRanges[1];
	descriptorRanges[0] = D12DescriptorRange{ D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0 };
	//descriptorRanges[1] = D12DescriptorRange{ D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1 };
	//					D3D12_DESCRIPTOR_RANGE_TYPE rangeType, u32 descriptorCount, u32 shaderRegister
	D12RootParameter parameters[idx::count]{};
	parameters->AsDescriptorTable(D3D12_SHADER_VISIBILITY_ALL, descriptorRanges, 1);
	m_cameraConstDescriptorRanges = descriptorRanges[0];
	//Create and set root signature for 
	const D12RootSignatureDesc rootSignatureDesc = D12RootSignatureDesc(m_helper, &parameters[0], _countof(parameters), D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);


	m_constantBuffersRootSignature = rootSignatureDesc.Create();
	m_constantBuffersRootSignature->SetName(L"Constant Buffer Root Sig");
	//cmdList->SetGraphicsRootSignature(m_constantBuffersRootSignature);
	//ID3D12DescriptorHeap* const cHeaps[]{ GetConstBHeap().GetHeap() };
	//cmdList->SetDescriptorHeaps(1, &cHeaps[0]);
	//Connect resource with root descriptor table
	//cmdList->SetGraphicsRootDescriptorTable(idx::desciptor_table, GetConstBHeap().GetHeap()->GetGPUDescriptorHandleForHeapStart());
	//SetGraphicsRootDescriptorTable() = SetGraphicsRootConstantBufferView(), because we created resource a table, so we use the former one
	return true;
}

void RendererD12::CheckRaytracingSupport()
{
	D3D12_FEATURE_DATA_D3D12_OPTIONS5 options5 = {};
	DXCall(m_device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &options5, sizeof(options5)));
	if (options5.RaytracingTier < D3D12_RAYTRACING_TIER_1_0) 
	{
		ERROR_AND_DIE("Ray tracing not supported on this device")
	}
	else
	{
		DebuggerPrintf("Ray tracing is supported!\n");
	}
}

IDXGIAdapter4* RendererD12::DetermineMainAdapter()
{
	IDXGIAdapter4* adapter = nullptr;
	
	//get adapters in descending order of performance
	for (int i = 0; m_dxgiFactory->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&adapter)) != DXGI_ERROR_NOT_FOUND; ++i) 
	{
		if (SUCCEEDED(D3D12CreateDevice(adapter, MINIMUM_FEATURE_LEVEL, __uuidof(ID3D12Device), nullptr))) 
		{
			return adapter;
		}
		DX_SAFE_RELEASE(adapter);
	}
	return nullptr;
}

D3D_FEATURE_LEVEL RendererD12::GetMaxFeatureLevel(IDXGIAdapter1* adapter)
{
	constexpr D3D_FEATURE_LEVEL featureLevels[4]
	{
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_12_1,
	};
	D3D12_FEATURE_DATA_FEATURE_LEVELS featureLevelInfo{};
	featureLevelInfo.NumFeatureLevels = _countof(featureLevels);
	featureLevelInfo.pFeatureLevelsRequested = featureLevels;

	ComPtr<ID3D12Device> device;

	HRESULT hr;
	hr = D3D12CreateDevice(adapter, MINIMUM_FEATURE_LEVEL, IID_PPV_ARGS(&device));
	if (!SUCCEEDED(hr)) 
	{
		ERROR_AND_DIE("Could not Create Device.");
	}
	hr = device->CheckFeatureSupport(D3D12_FEATURE_FEATURE_LEVELS, &featureLevelInfo, sizeof(featureLevelInfo));
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("Could not Support feature levels.");
	}
	return featureLevelInfo.MaxSupportedFeatureLevel;
}

unsigned int RendererD12::GetCurrentFrameIndex()
{
	return m_gfxCommand->GetFrameIndex();
}

void RendererD12::SetDefferredReleasesFlag()
{
	unsigned int resultIndex = GetCurrentFrameIndex();
	m_deferredReleasesFlag[resultIndex] = 1;
}


#endif // ENABLE_D3D12