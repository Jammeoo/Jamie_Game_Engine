#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#ifdef ENABLE_D3D12
#include "Engine/Renderer/D12CommonHeader.hpp"
#include "Engine/Renderer/AccelerationStructureHelper.hpp"
#include "Engine/Renderer/RendererD12.hpp"
#include "Engine/Renderer/D12Resources.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Core/Vertex_PNCU.hpp"
#include "Engine/Core/Vertex_PCUTBN.hpp"
#include <vector>
#include <fstream>      // std::ifstream
#include <sstream>      // std::stringstream
class D12Surface;
// #DXR
struct AccelerationStructureBuffers
{
	ComPtr<ID3D12Resource> pScratch;      // Scratch memory for AS builder
	ComPtr<ID3D12Resource> pResult;       // Where the AS is
	ComPtr<ID3D12Resource> pInstanceDesc; // Hold the matrices of the instances
};


namespace GlobalRootSignatureParams {
	enum Value {
		OutputViewSlot = 0,
		AccelerationStructureSlot,
		CameraConstant,
		PaddingFloat,
		Count
	};
}
namespace ModelParams {
	enum ModelIndex {
		Sphere = 0,
		AccelerationStructureSlot,
		CameraConstant,
		PaddingFloat,
		Count
	};
}
struct  ModelResource
{
public:
	ModelResource() = default;
	ModelResource(char const* xmlFilePath) 
	{
		bool result = LoadXMLFile(xmlFilePath);
		GUARANTEE_OR_DIE((result), "XML file loading failed!");
	}
	bool LoadXMLFile(char const* xmlFilePath) 
	{
		XmlDocument modelXml;
		XmlResult result = modelXml.LoadFile(xmlFilePath);
		if (result != tinyxml2::XML_SUCCESS)
		{
			return false;
		}
		XmlElement* rootElement = modelXml.RootElement();

		m_modelPath = ParseXmlAttribute(*rootElement, "path", m_modelPath);

		XmlElement* transElement = rootElement->FirstChildElement();
		Vec3 _transformX = Vec3(0.f, 0.f, 0.f);
		Vec3 _transformY = Vec3(0.f, 0.f, 0.f);
		Vec3 _transformZ = Vec3(0.f, 0.f, 0.f);
		Vec3 _transformW = Vec3(0.f, 0.f, 0.f);

		_transformX = ParseXmlAttribute(*transElement, "x", _transformX);
		_transformY = ParseXmlAttribute(*transElement, "y", _transformY);
		_transformZ = ParseXmlAttribute(*transElement, "z", _transformZ);
		_transformW = ParseXmlAttribute(*transElement, "t", _transformW);
		float scale = 0.f;
		scale = ParseXmlAttribute(*transElement, "scale", scale);

		Mat44 fixMat44;
		fixMat44.SetIJKT3D(_transformX, _transformY, _transformZ, _transformW);
		fixMat44.AppendScaleUniform3D(scale);
		m_fixMat44FromXML = fixMat44;
		return true;
	}
	std::string m_modelPath = "";
	Mat44 m_fixMat44FromXML;
public:
	Mat44 GetModelMatrix()const 
	{
		Mat44 mat44 = Mat44();
		mat44 = m_orientationDegrees.GetAsMatrix_XFwd_YLeft_ZUp();
		mat44.SetTranslation3D(m_position);
		return mat44;
	}
	Vec3 m_position = Vec3(0.f, 0.f, 0.f);
	EulerAngles m_orientationDegrees;
	//Mat44 m_modelMatrix;

public:
	uint32_t m_vertexNum = 0;
	ComPtr<ID3D12Resource> m_vertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView{};

	uint32_t m_indexNum = 0;
	ComPtr<ID3D12Resource> m_indexBuffer;
	D3D12_INDEX_BUFFER_VIEW m_indexBufferView{};
};
class RayTracingHelper
{
public:
	RayTracingHelper(RendererD12* ownerRenderer) :m_ownerRenderer(ownerRenderer)
	{
		GUARANTEE_OR_DIE(m_ownerRenderer, "Renderer is Null");
	}
	~RayTracingHelper();
	void ReleaseRayResources();
	void LoadContextAssets(D12GPUMesh* gpuMesh);
	void LoadModelAssets(ModelResource* modelResource);
	bool Initialize();
	void InitialPipelineStates();

	void AddTriangleResource();
	void AddSphereModelResource();
	void AddTeapotModelResource();
	void AddSphereToModelResource(ModelResource* modelResource, Vec3 const& position, float radius = 1.f);
	void AddCubeToModelResource(ModelResource* modelResource, Vec3 const& position);
	void LoadTextureAsset();
	void UpdateTopLevelAS();

/// Create the acceleration structure of an instance
///
/// \param     vVertexBuffers : pair of buffer and vertex count
/// \return    AccelerationStructureBuffers for TLAS
	AccelerationStructureBuffers CreateBottomLevelAS(
		std::vector<std::pair<ComPtr<ID3D12Resource>, uint32_t>> vVertexBuffers,
		std::vector<std::pair<ComPtr<ID3D12Resource>, uint32_t>> vIndexBuffers =
		{});

/// Create the main acceleration structure that holds
/// all instances of the scene
/// \param     instances : pair of BLAS and transform

	void CreateTopLevelAS(
		const std::vector<std::pair<ComPtr<ID3D12Resource>, DirectX::XMMATRIX>>
		& instances,
		bool updateOnly = false);

	/// Create all acceleration structures, bottom and top
	void CreateAccelerationStructures();

	//V Advice
	//Global Root Signature
	void SerializeAndCreateRaytracingRootSignature(D3D12_ROOT_SIGNATURE_DESC& desc, ComPtr<ID3D12RootSignature>* rootSig);
	//void CreateGlobalRootSignatures();
	//ComPtr<IDxcBlob> m_rayTracingLibrary;
	//void CreateRaytracingPipelineWithGlobalRootSig();
	Microsoft::WRL::ComPtr<ID3D12RootSignature> CreateEmptyLocalSignatureForShaders();
	//void CreateShaderResourceHeapWithGlobalRootSig();
	//void CreateShaderBindingTableWithGlobalRootSig();


	//PBR Light Constant
	void CreateLightAndModelBuffer();
	void UpdateLightBuffer(ConstantsD12 const& lightConst);
	ComPtr< ID3D12Resource > m_constBuffer;
	ComPtr< ID3D12DescriptorHeap > m_constBufferHeap;
	uint32_t m_constBufferSize = 0;

	// #DXR Extra: Perspective Camera
	void CreateCameraBuffer();
	void UpdateCameraBuffer(CameraConstantsD12 const& cameraConst);
	ComPtr< ID3D12Resource > m_cameraBuffer;
	ComPtr< ID3D12DescriptorHeap > m_constHeap;
	uint32_t m_cameraBufferSize = 0;

	// #DXR Extra: Per-Instance Data
	ComPtr<ID3D12Resource> m_planeBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_planeBufferView;
	void CreatePlaneVB();


	// #DXR Extra - Another ray type
	ComPtr<IDxcBlob> m_shadowLibrary;
	ComPtr<ID3D12RootSignature> m_shadowSignature;

	// #DXR
	ComPtr<ID3D12RootSignature> CreateRayGenSignature();
	ComPtr<ID3D12RootSignature> CreateMissSignature();//Empty
	ComPtr<ID3D12RootSignature> CreateHitSignature();
	void CreateRaytracingPipeline();
	//void CreateRayTracingPipelineWithGlobalRootSig();


	ComPtr<IDxcBlob> m_rayGenLibrary;
	ComPtr<IDxcBlob> m_hitLibrary;
	ComPtr<IDxcBlob> m_missLibrary;

	ComPtr<ID3D12RootSignature> m_rayGenSignature;
	ComPtr<ID3D12RootSignature> m_hitSignature;
	ComPtr<ID3D12RootSignature> m_missSignature;

	// Ray tracing pipeline state
	ComPtr<ID3D12StateObject> m_rtStateObject;
	// Ray tracing pipeline state properties, retaining the shader identifiers
	// to use in the Shader Binding Table
	ComPtr<ID3D12StateObjectProperties> m_rtStateObjectProps;


	ComPtr<ID3D12RootSignature> m_raytracingGlobalRootSignature;
	ComPtr<ID3D12RootSignature> m_cameraRootSignature;
	ComPtr<ID3D12PipelineState> m_pipelineState;
	// #DXR
	void CreateRaytracingOutputBuffer();
	void CreateShaderResourceHeap();
	

	ComPtr<ID3D12Resource> m_outputResource;

	// #DXR
	void CreateShaderBindingTable();
	ShaderBindingTableGenerator m_sbtHelper;
	ComPtr<ID3D12Resource> m_sbtStorage;

	void OnRender();
	void PopulateCommandList();
	void TransitResource(D12Surface* surface);
private:
	ID3D12Resource* CreateBuffer(id3d12Device* device, uint64_t size,
		D3D12_RESOURCE_FLAGS flags, D3D12_RESOURCE_STATES initState,
		const D3D12_HEAP_PROPERTIES& heapProps);
//--------------------------------------------------------------------------------------------------
//
//
	ID3D12DescriptorHeap* CreateDescriptorHeap(ID3D12Device* device, uint32_t count,
		D3D12_DESCRIPTOR_HEAP_TYPE type, bool shaderVisible)
	{
		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		desc.NumDescriptors = count;
		desc.Type = type;
		desc.Flags =
			shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

		ID3D12DescriptorHeap* pHeap;
		DXCall(device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&pHeap)));
		return pHeap;
	}

//--------------------------------------------------------------------------------------------------
// Compile a HLSL file into a DXIL library
//
	IDxcBlob* CompileShaderLibrary(LPCWSTR fileName)
	{
		static IDxcCompiler* pCompiler = nullptr;
		static IDxcLibrary* pLibrary = nullptr;
		static IDxcIncludeHandler* dxcIncludeHandler;

		HRESULT hr;

		// Initialize the DXC compiler and compiler helper
		if (!pCompiler)
		{
			DXCall(DxcCreateInstance(CLSID_DxcCompiler, __uuidof(IDxcCompiler), (void**)&pCompiler));
			DXCall(DxcCreateInstance(CLSID_DxcLibrary, __uuidof(IDxcLibrary), (void**)&pLibrary));
			DXCall(pLibrary->CreateIncludeHandler(&dxcIncludeHandler));
		}
		// Open and read the file
		std::ifstream shaderFile(fileName);
		if (shaderFile.good() == false)
		{
			ERROR_AND_DIE("Cannot find shader file");
		}
		std::stringstream strStream;
		strStream << shaderFile.rdbuf();
		std::string sShader = strStream.str();

		// Create blob from the string
		IDxcBlobEncoding* pTextBlob;
		DXCall(pLibrary->CreateBlobWithEncodingFromPinned(
			(LPBYTE)sShader.c_str(), (uint32_t)sShader.size(), 0, &pTextBlob));

		// Compile
		IDxcOperationResult* pResult;
		DXCall(pCompiler->Compile(pTextBlob, fileName, L"", L"lib_6_3", nullptr, 0, nullptr, 0,
			dxcIncludeHandler, &pResult));

		// Verify the result
		HRESULT resultCode;
		DXCall(pResult->GetStatus(&resultCode));
		if (FAILED(resultCode))
		{
			IDxcBlobEncoding* pError;
			hr = pResult->GetErrorBuffer(&pError);
			if (FAILED(hr))
			{
				ERROR_AND_DIE("Failed to get shader compiler error");
			}

			// Convert error blob to a string
			std::vector<char> infoLog(pError->GetBufferSize() + 1);
			memcpy(infoLog.data(), pError->GetBufferPointer(), pError->GetBufferSize());
			infoLog[pError->GetBufferSize()] = 0;

			std::string errorMsg = "Shader Compiler Error:\n";
			errorMsg.append(infoLog.data());

			MessageBoxA(nullptr, errorMsg.c_str(), "Error!", MB_OK);
			ERROR_AND_DIE("Failed compile shader");
		}

		IDxcBlob* pBlob;
		DXCall(pResult->GetResult(&pBlob));
		return pBlob;
	}

public:
	//collection of Public ray tracing parameter data

	//std::vector<D3D12_ROOT_PARAMETER> m_rayParameters;
	
	//std::vector<UINT> m_rayRangeLocations;
	id3d12Device* m_device = nullptr;
private:
	RendererD12* m_ownerRenderer = nullptr;

	// Pipeline objects.
	id3d12GraphicsCommandList* m_commandList = nullptr;
	ID3D12CommandQueue* m_cmdQueue = nullptr;

public:
	
	// App resources.
	
	ModelResource m_sphereModel_One;
	ModelResource m_sphereModel_Two;
	ModelResource m_potModel;
	ModelResource m_sphereModelPointLight;
	ModelResource m_sphereModelRed;
	ModelResource m_sphereModelYellow;
private:

	ComPtr<ID3D12Resource> m_vertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView{};
	  ComPtr<ID3D12Resource> m_indexBuffer;
  D3D12_INDEX_BUFFER_VIEW m_indexBufferView{};


// Specifies the default heap. This heap type experiences the most bandwidth for
// the GPU, but cannot provide CPU access.
	const D3D12_HEAP_PROPERTIES m_kDefaultHeapProps = {
		D3D12_HEAP_TYPE_DEFAULT, D3D12_CPU_PAGE_PROPERTY_UNKNOWN, D3D12_MEMORY_POOL_UNKNOWN, 0, 0 };
// Specifies a heap used for uploading. This heap type has CPU access optimized
// for uploading to the GPU.
	const D3D12_HEAP_PROPERTIES m_kUploadHeapProps = {
		D3D12_HEAP_TYPE_UPLOAD, D3D12_CPU_PAGE_PROPERTY_UNKNOWN, D3D12_MEMORY_POOL_UNKNOWN, 0, 0 };


	ComPtr<ID3D12Resource> m_bottomLevelAS; // Storage for the bottom Level AS

	TopLevelASGenerator m_topLevelASGenerator;
	AccelerationStructureBuffers m_topLevelASBuffers;
	std::vector<std::pair<ComPtr<ID3D12Resource>, DirectX::XMMATRIX>> m_instances;
	//std::pair<ComPtr<ID3D12Resource>, DirectX::XMMATRIX> m_instances[1];
};



#endif // ENABLE_D3D12