#pragma once

#include "Engine/Renderer/D12CommonHeader.hpp"
#include "Game/EngineBuildPreferences.hpp"
#include "Engine/Renderer/Window.hpp"
#include"Engine/Core/Rgba8.hpp"
#include"Engine/Renderer/Camera.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Math/Vec4.hpp"
#include "Engine/Math/Vec3.hpp"
#ifdef ENABLE_D3D12
#include "Engine/Renderer/D12Command.hpp"
#include "Engine/Renderer/D12Resources.hpp"
#include "Engine/Renderer/D12Helpers.hpp"
#include "Engine/Renderer/Image.hpp"
class D12Command;
class D12Shaders;
class D12GPass;
class D12Helpers;
class D12ResourceBarrier;
class D12PostProcess;
class D12UploadContext;
class ConstantBuffer;
class D12GPUMesh;
class D12Content;
class RayTracingHelper;


struct CAMERA_ROOT_PARAM_INDICES
{
	enum : u32 {
		//constant_buffer,
		desciptor_table_camera_constants,
		count
	};
};

struct MaterialType 
{
	enum type :u32 
	{
		opaque,
	};
};


class D12Surface;

struct RendererD12Config
{
	Window* m_window = nullptr;
	bool m_isRaster = false;
};

struct CameraConstantsD12
{
	Mat44 ProjectionMatrix;
	Mat44 ViewMatrix;
	Mat44 ProjectionMatrixInverse;
	Mat44 ViewMatrixInverse;
};

struct  LightConstantsD12
{
	Vec3 LightPosiOne;
	float Roughness;

	float Metallic;
	Vec3 AmbientLightDir;

	float AmbientLightIntensity;
	Vec3 AmbientLightColorInFloat;

	unsigned int LightReflectanceModelType;
	Vec3 LightPosiTwo;

	Vec3 LightPosiThree;
	float mipmapValue = 0.f;

	float FalloffStart;
	float FalloffEnd;
	Vec2 PaddingVec41;
	Vec4 PaddingVec42;
	Vec4 PaddingVec43;
	Mat44 SphereTwoMatrix;
	Mat44 PotMatrix;
};

struct ModelConstantsD12
{
	Mat44 ModelMatrix1;
	Mat44 ModelMatrix2;
	Mat44 ModelMatrix3;
	Mat44 ModelMatrix4;
};

struct ConstantsD12 
{
	LightConstantsD12 lightCD12;
	ModelConstantsD12 modelCD12;
};

class RendererD12 
{
public:
	RendererD12(RendererD12Config const& config);
	~RendererD12();


	void Startup();

	void Shutdown();

	void RenderSurface();
	void ProcessDeferredReleases(u32 frameIndex);

	void DeferredRelease(IUnknown* resource);

	void BeginFrame();
	void EndFrame();
	void ClearScreen(const Rgba8& clearColor);
	void BeginCamera(const Camera& camera);
	void EndCamera(const Camera& camera);
	void DrawIndexedVertexArray(D12GPUMesh* gpuMeshWithView);
	void RenderContextWithRasterization();
	void RenderContextWithRaytracing();
	//void SetModelConstants(const Mat44& modelMatrix = Mat44(), const Rgba8& modelColor = Rgba8::WHITE);
	//void AddModelToContent();
public:
	void CreateSurface(Window* window);
	void RemoveSurface();


	void CreateUploadHeapForAllTextures();
	D12Texture* CreateTextureFromFile(char const* imageFilePath);
	D12Texture* CreateTextureFromImage(const Image& image);
	
	D12Texture* CreateTextureCubeFromFile(std::vector<char const*> imageFilePathList);
	D12Texture* CreateTextureCubeFromImage(std::vector<Image *> imageList);
	void CombineImageRawData(std::vector<Image*> imageList, std::vector<Rgba8>& texelRgba8Data);
private:
	bool CreateRootSigForCameraConstant();
	void CheckRaytracingSupport();


public:
	id3d12Device* GetDevice()const { return m_device; }

	DescriptorHeap& GetRTVHeap() { return m_rtvDescHeap; }
	DescriptorHeap& GetDSVHeap() { return m_dsvDescHeap; }
	DescriptorHeap& GetSRVHeap() { return m_srvDescHeap; }
	DescriptorHeap& GetUAVHeap() { 
		return m_uavDescHeap; 
	}
	DescriptorHeap& GetConstBHeap() { return m_constBufferHeap; }
	//DXGI_FORMAT GetDefaultRenderTargetFormat() { return DXGI_FORMAT_R8G8B8A8_UNORM; }//DXGI_FORMAT_R8G8B8A8_UNORM_SRGB
	RendererD12Config const& GetConfig()const { return m_config; }
	IDXGIAdapter4* DetermineMainAdapter();
	D3D_FEATURE_LEVEL GetMaxFeatureLevel(IDXGIAdapter1* adapter);
	unsigned int GetCurrentFrameIndex();
	void SetDefferredReleasesFlag();
	D12Shaders* GetShaders() { return m_theShader; }
	D12Helpers* GetHelper() { return m_helper; }
	D12Surface* GetSurface() { return m_surface; }
	D12GPass* GetGPass() { return m_gPass; }
	ConstantBuffer& GetConstantBuffer() { return m_constantBuffers[GetCurrentFrameIndex()]; }
	ConstantBuffer& GetTryCBuffer() { return m_tryCB; }
	D12UploadContext* GetUploadContext() { return m_uploadContext; }
	D12Command* GetCmdListOwner() { return m_gfxCommand; }
	D12Content* GetContent() { return m_context; }
	ID3D12RootSignature* GetCBufferRootSig()const { return m_constantBuffersRootSignature; }
	D12DescriptorRange GetCameraConstRange()const { return m_cameraConstDescriptorRanges; }
	const Rgba8 m_clearColor = Rgba8::DEEP_BLUE;
	D12ResourceBarrier* GetResourceBarrier() { return m_resourceBarriers; }



public:
// 	void SwitchDrawMode() 
// 	{
// 		m_usingRaster = !m_usingRaster;
// 	}
	//void SwitchRenderBool() 
	//{
	//	m_doRendering = !m_doRendering;
	//}
	bool m_usingRaster = true;
	//bool m_doRendering = false;
	D12DescriptorRange m_cameraConstDescriptorRanges;
private:
	RendererD12Config m_config;
	id3d12Device* m_device = nullptr;
	IDXGIFactory7* m_dxgiFactory = nullptr;

	D12Command* m_gfxCommand = nullptr;

	DescriptorHeap m_rtvDescHeap{ this, D3D12_DESCRIPTOR_HEAP_TYPE_RTV };//Render target view
	DescriptorHeap m_dsvDescHeap{ this, D3D12_DESCRIPTOR_HEAP_TYPE_DSV };//Depth stencil
	DescriptorHeap m_srvDescHeap{ this, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV };//Shader resource view(shader visible)
	DescriptorHeap m_uavDescHeap{ this, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV };//shader visible too but for raytracing
	DescriptorHeap m_constBufferHeap{ this, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV };


	std::vector<IUnknown*> m_deferredReleases[FRAME_BUFFER_COUNT]{};
	u32 m_deferredReleasesFlag[FRAME_BUFFER_COUNT]{};
	std::mutex m_deferredReleasesMutex;


	//Surface 
	//std::vector<D12Surface*> m_surfaces;
	D12Surface* m_surface = nullptr;

	D12Shaders* m_theShader = nullptr;
	D12GPass* m_gPass = nullptr;
	D12Helpers* m_helper = nullptr;
	D12PostProcess* m_postProcess = nullptr;

	D12ResourceBarrier* m_resourceBarriers = nullptr;
	D12UploadContext* m_uploadContext = nullptr;
	D12Content* m_context = nullptr;
	ConstantBuffer m_constantBuffers[FRAME_BUFFER_COUNT];
	ConstantBuffer m_tryCB;

	ID3D12RootSignature* m_constantBuffersRootSignature = nullptr;
	//ID3D12DescriptorHeap* m_constBufferDescHeap = nullptr;

	RayTracingHelper* m_rayTracer = nullptr;

	//Texture
	ID3D12Resource* m_textureBufferUploadHeap = nullptr;
	struct TextureSizeDesc{
	public:
		u32 m_numBytesPerPixel = 0;
		u32 m_width = 0;
		u32 m_height = 0;
	};

	std::vector<D12Texture*> m_textureArray;

public:
	float m_roughness = 0.3f;
	float m_metallic = 0.2f;
	Vec3 m_pointLightPositionOne = Vec3(0.3f, 0.2f, 3.25f);
	Vec3 m_ambientLightDir = Vec3(0.f, 0.f, 1.f);
	float m_ambientLightIntensity = 0.5f;
	Rgba8 m_ambientLightColor = Rgba8(255, 255, 255, 255);
	unsigned int m_lightModelIndex = 1;


	float m_mipmapTester = 0.1f;
	//Vec3 m_potPosition = Vec3(0.f, 0.f, 1.f);
};

#endif // ENABLE_D3D12