#include "Engine/Renderer/RayTracingHelper.hpp"
#ifdef ENABLE_D3D12

#include "Engine/Renderer/RendererD12.hpp"
#include "Engine/Renderer/D12Content.hpp"
#include "Engine/Renderer/D12Helpers.hpp"
#include "Engine/Renderer/D12Surface.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/AABB3.hpp"

RayTracingHelper::~RayTracingHelper()
{
	
}

void RayTracingHelper::ReleaseRayResources()
{
	m_raytracingGlobalRootSignature.Reset();
}

void RayTracingHelper::LoadContextAssets(D12GPUMesh* gpuMesh)
{
	GUARANTEE_OR_DIE(gpuMesh, "Mesh is nullptr");
	m_vertexBuffer = gpuMesh->m_vertexMeshBuffer;
	m_vertexBufferView = gpuMesh->m_meshView->m_vertexBufferView;

	m_indexBuffer = gpuMesh->m_indexMeshBuffer;
	m_indexBufferView = gpuMesh->m_meshView->m_indexBufferView;
}

void RayTracingHelper::LoadModelAssets(ModelResource* modelResource)
{
	GUARANTEE_OR_DIE(modelResource, "Mesh is nullptr");
	m_vertexBuffer = modelResource->m_vertexBuffer;
	m_vertexBufferView = modelResource->m_vertexBufferView;

	m_indexBuffer = modelResource->m_indexBuffer;
	m_indexBufferView = modelResource->m_indexBufferView;
}

bool RayTracingHelper::Initialize()
{
	if (!m_device) 
	{
		m_device = m_ownerRenderer->GetDevice();
	}
	m_commandList = m_ownerRenderer->GetCmdListOwner()->GetCommandList();
	m_cmdQueue = m_ownerRenderer->GetCmdListOwner()->GetCommandQueue();
	//InitialPipelineState();

	CreatePlaneVB();
	//AddTeapotModelResource();
	AddCubeToModelResource(&m_potModel, Vec3(-1.f, -1.f, 1.f));
	AddSphereToModelResource(&m_sphereModel_Two, Vec3(0.f, 2.f, 2.f));
	AddSphereToModelResource(&m_sphereModelPointLight, m_ownerRenderer->m_pointLightPositionOne, 0.05f);
	AddSphereToModelResource(&m_sphereModelRed, Vec3(-2.f, 1.f, 2.f));// Vec3(-2.f, 1.f, 2.f)) Vec3(6.f, 0.f, 2.f))

	AddSphereToModelResource(&m_sphereModelYellow, Vec3(3.f, 3.f, 2.f));//Vec3(3.f, 3.f, 2.f)) Vec3(6.f, 0.f, 2.f))
	m_ownerRenderer->GetCmdListOwner()->ExecuteCmdLists(m_pipelineState.Get());
	// Setup the acceleration structures (AS) for raytracing. When setting up
	// geometry, each bottom-level AS has its own transform matrix.
	CreateAccelerationStructures();

	// Create the raytracing pipeline, associating the shader code to symbol names
	// and to their root signatures, and defining the amount of memory carried by
	// rays (ray payload)

	CreateRaytracingPipeline(); // #DXR


	// Allocate the buffer storing the raytracing output, with the same dimensions
	// as the target image
	CreateRaytracingOutputBuffer(); // #DXR

	// Create the buffer containing the raytracing result (always output in a
	// UAV), and create the heap referencing the resources used by the raytracing,
	// such as the acceleration structure
	//CreateShaderResourceHeapWithGlobalRootSig();
	CreateCameraBuffer();
	CreateLightAndModelBuffer();
	CreateShaderResourceHeap(); // #DXR
	LoadTextureAsset();
	DXCall(m_commandList->Close());
	// Create the shader binding table and indicating which shaders
	// are invoked for each instance in the  AS
	CreateShaderBindingTable();
	return true;
}

void RayTracingHelper::InitialPipelineStates()
{

	CD3DX12_ROOT_PARAMETER constantParameter;
	CD3DX12_DESCRIPTOR_RANGE range;
	range.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
	constantParameter.InitAsDescriptorTable(1, &range, D3D12_SHADER_VISIBILITY_ALL);

	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init(1, &constantParameter, 0, nullptr,
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	SerializeAndCreateRaytracingRootSignature(rootSignatureDesc, &m_cameraRootSignature);

	D12PipelineStateSubobjectStream psoStream = D12PipelineStateSubobjectStream();

	D3D12_RT_FORMAT_ARRAY rt_array{};
	rt_array.NumRenderTargets = 1;
	rt_array.RTFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoStream.renderTargetFormats = rt_array;
	//Create Input Layout
	D3D12_INPUT_ELEMENT_DESC inputElementDesc[] = {
		{"POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT,		0,		0,								D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{"COLOR",		0, DXGI_FORMAT_R8G8B8A8_UNORM,		0,		D3D12_APPEND_ALIGNED_ELEMENT,	D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		//{"COLOR",		0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0,		D3D12_APPEND_ALIGNED_ELEMENT,	D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{"TEXCOORD",	0, DXGI_FORMAT_R32G32_FLOAT,		0,		D3D12_APPEND_ALIGNED_ELEMENT,	D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{"TANGENT",		0, DXGI_FORMAT_R32G32B32_FLOAT,		0,		D3D12_APPEND_ALIGNED_ELEMENT,	D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{"BINORMAL",	0, DXGI_FORMAT_R32G32B32_FLOAT,		0,		D3D12_APPEND_ALIGNED_ELEMENT,	D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{"NORMAL",		0, DXGI_FORMAT_R32G32B32_FLOAT,		0,		D3D12_APPEND_ALIGNED_ELEMENT,	D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
	};
	std::vector<D3D12_INPUT_ELEMENT_DESC> inputLayout(inputElementDesc, inputElementDesc + 6);
	psoStream.inputLayout = { inputLayout.data() , (UINT)inputLayout.size() };
	psoStream.vs = m_ownerRenderer->GetShaders()->GetCompiledShaderByID(EngineShader::UNLIT_VS);
	psoStream.ps = m_ownerRenderer->GetShaders()->GetCompiledShaderByID(EngineShader::UNLIT_PS);
	psoStream.rootSignature = m_cameraRootSignature.Get();
	psoStream.rasterizer = rasterizerState.noCull;
	psoStream.blend = blendState.alphaBlend;
	psoStream.depthStencil1 = depthState.disabled;
	psoStream.primitiveTopology = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	m_pipelineState = m_ownerRenderer->GetHelper()->CreatePipelineState(&psoStream, sizeof(psoStream));
}

void RayTracingHelper::AddTriangleResource()
{
	// Create the vertex buffer.
	{
		// Define the geometry for a triangle.
		Vertex_PCUTBN one = Vertex_PCUTBN(Vertex_PNCU(Vec3(0.0f, 0.25f, 0.0f), Vec3::Z_AXIS, Rgba8::WHITE));
		Vertex_PCUTBN three = Vertex_PCUTBN(Vertex_PNCU(Vec3(-0.25f, -0.25f, 0.0f), Vec3::Z_AXIS, Rgba8::WHITE));
		Vertex_PCUTBN two = Vertex_PCUTBN(Vertex_PNCU(Vec3(0.25f, -0.25f, 0.0f), Vec3::Z_AXIS, Rgba8::WHITE));

		Vertex_PCUTBN triangleVertices[] = { one, two, three };

		const UINT vertexBufferSize = sizeof(triangleVertices);

		// Note: using upload heaps to transfer static data like vert buffers is not
		// recommended. Every time the GPU needs it, the upload heap will be
		// marshalled over. 
		m_vertexBuffer = CreateBuffer(m_device, vertexBufferSize, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ, m_kUploadHeapProps);

		
		// Copy the triangle data to the vertex buffer.
		UINT8* pVertexDataBegin;
		D3D12_RANGE readRange{ 0,0 };

		DXCall(m_vertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
		memcpy(pVertexDataBegin, triangleVertices, vertexBufferSize);
		m_vertexBuffer->Unmap(0, nullptr);

		// Initialize the vertex buffer view.
		m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
		m_vertexBufferView.StrideInBytes = sizeof(Vertex_PCUTBN);
		m_vertexBufferView.SizeInBytes = vertexBufferSize;

	}
}

void RayTracingHelper::AddSphereModelResource()
{
	std::vector<Vertex_PCUTBN> verts;
	std::vector<unsigned int> indexes;
	//AddVertsForUVSphereZ3D(verts, indexes, Vec3::Z_AXIS, 1.f, 256.f, 128.f);
	AddVertsForUVSphereZ3D(verts, indexes, Vec3::WORLD_ORIGIN, 1.f, 256.f, 128.f);
	//AddVertsForUVSphereZ3D(verts, indexes, Vec3::Z_AXIS, 1.f, 256.f, 128.f);
	FillTangentSpaceVectorForMesh(verts, indexes);

	m_sphereModel_One.m_position = Vec3(3.f, 0.f, 2.f);
	//m_sphereModel_One.m_orientationDegrees.m_pitchDegrees = 90.f;
	m_sphereModel_One.m_vertexNum = (uint32_t)verts.size();
	m_sphereModel_One.m_indexNum = (uint32_t)indexes.size();
	const UINT vertexBufferSize = sizeof(Vertex_PCUTBN) * (UINT)m_sphereModel_One.m_vertexNum;
	const UINT indexBufferSize = sizeof(unsigned int) * (UINT)m_sphereModel_One.m_indexNum;
	m_sphereModel_One.m_vertexBuffer = CreateBuffer(m_device, vertexBufferSize, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ, m_kUploadHeapProps);

	// Copy the triangle data to the vertex buffer.
	UINT8* pVertexDataBegin;
	D3D12_RANGE readRange{ 0,0 };

	DXCall(m_sphereModel_One.m_vertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
	memcpy(pVertexDataBegin, verts.data(), vertexBufferSize);
	m_sphereModel_One.m_vertexBuffer->Unmap(0, nullptr);

	// Initialize the vertex buffer view.
	m_sphereModel_One. m_vertexBufferView.BufferLocation = m_sphereModel_One.m_vertexBuffer->GetGPUVirtualAddress();
	m_sphereModel_One.m_vertexBufferView.StrideInBytes = sizeof(Vertex_PCUTBN);
	m_sphereModel_One.m_vertexBufferView.SizeInBytes = vertexBufferSize;

	m_sphereModel_One.m_indexBuffer = CreateBuffer(m_device, indexBufferSize, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ, m_kUploadHeapProps);

	// Copy the triangle data to the index buffer.
	UINT8* pIndexDataBegin;
	DXCall(m_sphereModel_One.m_indexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pIndexDataBegin)));
	memcpy(pIndexDataBegin, indexes.data(), indexBufferSize);
	m_sphereModel_One.m_indexBuffer->Unmap(0, nullptr);

	// Initialize the vertex buffer view.
	m_sphereModel_One.m_indexBufferView.BufferLocation = m_sphereModel_One.m_indexBuffer->GetGPUVirtualAddress();
	m_sphereModel_One.m_indexBufferView.SizeInBytes = indexBufferSize;
	m_sphereModel_One.m_indexBufferView.Format = DXGI_FORMAT_R32_UINT;

}

void RayTracingHelper::AddTeapotModelResource()
{
	std::vector<Vertex_PCUTBN> verts;
	std::vector<unsigned int> indexes;
	m_potModel.m_position = Vec3(0.f, 0.f, 0.f);
	
	std::string modelPath = "Data/Models/Pot1.xml";
	m_potModel = ModelResource(modelPath.c_str());
	OBJLoader::LoadOBJFile(m_potModel.m_modelPath, m_potModel.m_fixMat44FromXML, verts, indexes);
	FillTangentSpaceVectorForMesh(verts, indexes);
	m_potModel.m_vertexNum = (uint32_t)verts.size();
	m_potModel.m_indexNum = (uint32_t)indexes.size();
	const UINT vertexBufferSize = sizeof(Vertex_PCUTBN) * (UINT)m_potModel.m_vertexNum;
	const UINT indexBufferSize = sizeof(unsigned int) * (UINT)m_potModel.m_indexNum;
	m_potModel.m_vertexBuffer = CreateBuffer(m_device, vertexBufferSize, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ, m_kUploadHeapProps);

	// Copy the triangle data to the vertex buffer.
	UINT8* pVertexDataBegin;
	D3D12_RANGE readRange{ 0,0 };

	DXCall(m_potModel.m_vertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
	memcpy(pVertexDataBegin, verts.data(), vertexBufferSize);
	m_potModel.m_vertexBuffer->Unmap(0, nullptr);

	// Initialize the vertex buffer view.
	m_potModel.m_vertexBufferView.BufferLocation = m_potModel.m_vertexBuffer->GetGPUVirtualAddress();
	m_potModel.m_vertexBufferView.StrideInBytes = sizeof(Vertex_PCUTBN);
	m_potModel.m_vertexBufferView.SizeInBytes = vertexBufferSize;

	m_potModel.m_indexBuffer = CreateBuffer(m_device, indexBufferSize, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ, m_kUploadHeapProps);

	// Copy the triangle data to the index buffer.
	UINT8* pIndexDataBegin;
	DXCall(m_potModel.m_indexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pIndexDataBegin)));
	memcpy(pIndexDataBegin, indexes.data(), indexBufferSize);
	m_potModel.m_indexBuffer->Unmap(0, nullptr);

	// Initialize the vertex buffer view.
	m_potModel.m_indexBufferView.BufferLocation = m_potModel.m_indexBuffer->GetGPUVirtualAddress();
	m_potModel.m_indexBufferView.SizeInBytes = indexBufferSize;
	m_potModel.m_indexBufferView.Format = DXGI_FORMAT_R32_UINT;

}

void RayTracingHelper::AddSphereToModelResource(ModelResource* modelResource, Vec3 const& position, float radius /*=1.f*/)
{
	modelResource->m_position = position;
	//modelResource->m_orientationDegrees.m_pitchDegrees = 90.f;
	std::vector<Vertex_PCUTBN> verts;
	std::vector<unsigned int> indexes;
	AddVertsForUVSphereZ3D(verts, indexes, Vec3::WORLD_ORIGIN, radius, 256.f, 128.f);
	//AddVertsForUVSphereZ3D(verts, indexes, Vec3::WORLD_ORIGIN, radius, 32.f, 16.f);
	FillTangentSpaceVectorForMesh(verts, indexes);
	modelResource->m_vertexNum = (uint32_t)verts.size();
	modelResource->m_indexNum = (uint32_t)indexes.size();
	const UINT vertexBufferSize = sizeof(Vertex_PCUTBN) * (UINT)modelResource->m_vertexNum;
	const UINT indexBufferSize = sizeof(unsigned int) * (UINT)modelResource->m_indexNum;
	modelResource->m_vertexBuffer = CreateBuffer(m_device, vertexBufferSize, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ, m_kUploadHeapProps);

	// Copy the triangle data to the vertex buffer.
	UINT8* pVertexDataBegin;
	D3D12_RANGE readRange{ 0,0 };

	DXCall(modelResource->m_vertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
	memcpy(pVertexDataBegin, verts.data(), vertexBufferSize);
	modelResource->m_vertexBuffer->Unmap(0, nullptr);

	// Initialize the vertex buffer view.
	modelResource->m_vertexBufferView.BufferLocation = modelResource->m_vertexBuffer->GetGPUVirtualAddress();
	modelResource->m_vertexBufferView.StrideInBytes = sizeof(Vertex_PCUTBN);
	modelResource->m_vertexBufferView.SizeInBytes = vertexBufferSize;

	modelResource->m_indexBuffer = CreateBuffer(m_device, indexBufferSize, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ, m_kUploadHeapProps);

	// Copy the triangle data to the index buffer.
	UINT8* pIndexDataBegin;
	DXCall(modelResource->m_indexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pIndexDataBegin)));
	memcpy(pIndexDataBegin, indexes.data(), indexBufferSize);
	modelResource->m_indexBuffer->Unmap(0, nullptr);

	// Initialize the vertex buffer view.
	modelResource->m_indexBufferView.BufferLocation = modelResource->m_indexBuffer->GetGPUVirtualAddress();
	modelResource->m_indexBufferView.SizeInBytes = indexBufferSize;
	modelResource->m_indexBufferView.Format = DXGI_FORMAT_R32_UINT;
}


void RayTracingHelper::AddCubeToModelResource(ModelResource* modelResource, Vec3 const& position)
{
	modelResource->m_position = position;
	std::vector<Vertex_PCUTBN> verts;
	std::vector<unsigned int> indexes;
	//AddVertsForUVSphereZ3D(verts, indexes, position, 1.f, 256.f, 128.f);

	float offset = 1.f;
	Vec3 offsetVec3 = Vec3(offset, offset, offset);
	//Vec3 offsetHighZ = Vec3(offset, offset, 10.f);
	AABB3 cubeAABB = AABB3(position - offsetVec3, position + offsetVec3);

	AddVertsForCube3D(verts, indexes, cubeAABB);

	FillTangentSpaceVectorForMesh(verts, indexes);
	modelResource->m_vertexNum = (uint32_t)verts.size();
	modelResource->m_indexNum = (uint32_t)indexes.size();
	const UINT vertexBufferSize = sizeof(Vertex_PCUTBN) * (UINT)modelResource->m_vertexNum;
	const UINT indexBufferSize = sizeof(unsigned int) * (UINT)modelResource->m_indexNum;
	modelResource->m_vertexBuffer = CreateBuffer(m_device, vertexBufferSize, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ, m_kUploadHeapProps);

	// Copy the triangle data to the vertex buffer.
	UINT8* pVertexDataBegin;
	D3D12_RANGE readRange{ 0,0 };

	DXCall(modelResource->m_vertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
	memcpy(pVertexDataBegin, verts.data(), vertexBufferSize);
	modelResource->m_vertexBuffer->Unmap(0, nullptr);

	// Initialize the vertex buffer view.
	modelResource->m_vertexBufferView.BufferLocation = modelResource->m_vertexBuffer->GetGPUVirtualAddress();
	modelResource->m_vertexBufferView.StrideInBytes = sizeof(Vertex_PCUTBN);
	modelResource->m_vertexBufferView.SizeInBytes = vertexBufferSize;

	modelResource->m_indexBuffer = CreateBuffer(m_device, indexBufferSize, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ, m_kUploadHeapProps);

	// Copy the triangle data to the index buffer.
	UINT8* pIndexDataBegin;
	DXCall(modelResource->m_indexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pIndexDataBegin)));
	memcpy(pIndexDataBegin, indexes.data(), indexBufferSize);
	modelResource->m_indexBuffer->Unmap(0, nullptr);

	// Initialize the vertex buffer view.
	modelResource->m_indexBufferView.BufferLocation = modelResource->m_indexBuffer->GetGPUVirtualAddress();
	modelResource->m_indexBufferView.SizeInBytes = indexBufferSize;
	modelResource->m_indexBufferView.Format = DXGI_FORMAT_R32_UINT;
}

void RayTracingHelper::LoadTextureAsset()
{
	// 0->+X
	// 1->-X
	// 2->+Y
	// 3->-Y
	// 4->+Z
	// 5->-Z
	//output_pmrem_negx.dds

	std::vector<char const*> imageSkyboxFilePathList = {
	"Data/Images/SkyboxCubemap/GreenHouse/SkyBox/output_front_posz.png" ,
	"Data/Images/SkyboxCubemap/GreenHouse/SkyBox/output_back_negz.png",
	"Data/Images/SkyboxCubemap/GreenHouse/SkyBox/output_left_negx.png",
	"Data/Images/SkyboxCubemap/GreenHouse/SkyBox/output_right_posx.png",
	"Data/Images/SkyboxCubemap/GreenHouse/SkyBox/output_top_posy.png",
	"Data/Images/SkyboxCubemap/GreenHouse/SkyBox/output_bottom_negy.png"
	};

	//std::vector<char const*> imageFilePathList = {
	//	"Data/Images/SkyboxCubemap/Sea/front.jpg" ,
	//	"Data/Images/SkyboxCubemap/Sea/back.jpg",
	//	"Data/Images/SkyboxCubemap/Sea/left.jpg",
	//	"Data/Images/SkyboxCubemap/Sea/right.jpg",
	//	"Data/Images/SkyboxCubemap/Sea/top.jpg",
	//	"Data/Images/SkyboxCubemap/Sea/bottom.jpg"
	//};

	//std::vector<char const*> imageFilePathList = {
	//"Data/Images/SkyboxCubemap/Field/north.jpg",
	//"Data/Images/SkyboxCubemap/Field/south.jpg",
	//"Data/Images/SkyboxCubemap/Field/up.jpg",
	//"Data/Images/SkyboxCubemap/Field/bottom.jpg",
	//"Data/Images/SkyboxCubemap/Field/east.jpg",
	//"Data/Images/SkyboxCubemap/Field/west.jpg"
	//};

	//std::vector<char const*> imageFilePathList = {
	//"Data/Images/SkyboxCubemap/Field/posx.jpg",
	//"Data/Images/SkyboxCubemap/Field/negx.jpg",
	//"Data/Images/SkyboxCubemap/Field/negy.jpg",
	//"Data/Images/SkyboxCubemap/Field/posy.jpg",
	//"Data/Images/SkyboxCubemap/Field/posz.jpg" ,
	//"Data/Images/SkyboxCubemap/Field/negz.jpg"
	//};
	m_ownerRenderer->CreateTextureCubeFromFile(imageSkyboxFilePathList);

	std::vector<char const*> imageIrradianceFilePathList = {
	"Data/Images/SkyboxCubemap/GreenHouse/Irradiance/output_iem_front_posz.png" ,
	"Data/Images/SkyboxCubemap/GreenHouse/Irradiance/output_iem_back_negz.png",
	"Data/Images/SkyboxCubemap/GreenHouse/Irradiance/output_iem_left_negx.png",
	"Data/Images/SkyboxCubemap/GreenHouse/Irradiance/output_iem_right_posx.png",
	"Data/Images/SkyboxCubemap/GreenHouse/Irradiance/output_iem_top_posy.png",
	"Data/Images/SkyboxCubemap/GreenHouse/Irradiance/output_iem_bottom_negy.png"
	};

	m_ownerRenderer->CreateTextureCubeFromFile(imageIrradianceFilePathList);
	m_ownerRenderer->CreateTextureFromFile("Data/Images/SkyboxCubemap/GreenHouse/brdfLUT.png");

	std::vector<char const*> imageRadianceTopFilePathList = {
	"Data/Images/SkyboxCubemap/GreenHouse/Radiance/output_pmrem_256_front_posz.png" ,
	"Data/Images/SkyboxCubemap/GreenHouse/Radiance/output_pmrem_256_back_negz.png",
	"Data/Images/SkyboxCubemap/GreenHouse/Radiance/output_pmrem_256_left_negx.png",
	"Data/Images/SkyboxCubemap/GreenHouse/Radiance/output_pmrem_256_right_posx.png",
	"Data/Images/SkyboxCubemap/GreenHouse/Radiance/output_pmrem_256_top_posy.png",
	"Data/Images/SkyboxCubemap/GreenHouse/Radiance/output_pmrem_256_bottom_negy.png"
	};
	
	m_ownerRenderer->CreateTextureCubeFromFile(imageRadianceTopFilePathList);

	//std::vector<char const*> imageRadianceTopFilePathList = {
	//"Data/Images/SkyboxCubemap/GreenHouse/Radiance/output_pmrem_256_front_posz.png" ,
	//"Data/Images/SkyboxCubemap/GreenHouse/Radiance/output_pmrem_256_back_negz.png",
	//"Data/Images/SkyboxCubemap/GreenHouse/Radiance/output_pmrem_256_left_negx.png",
	//"Data/Images/SkyboxCubemap/GreenHouse/Radiance/output_pmrem_256_right_posx.png",
	//"Data/Images/SkyboxCubemap/GreenHouse/Radiance/output_pmrem_256_top_posy.png",
	//"Data/Images/SkyboxCubemap/GreenHouse/Radiance/output_pmrem_256_bottom_negy.png"
	//};
	//
	//m_ownerRenderer->CreateTextureCubeFromFile(imageRadianceTopFilePathList);

	//std::vector<char const*> imageIrradianceTopFilePathList = {
	//"Data/Images/SkyboxCubemap/GreenHouse/Radiance/output_pmrem_256_front_posz_blurred.png" ,
	//"Data/Images/SkyboxCubemap/GreenHouse/Radiance/output_pmrem_256_back_negz_blurred.png",
	//"Data/Images/SkyboxCubemap/GreenHouse/Radiance/output_pmrem_256_left_negx_blurred.png",
	//"Data/Images/SkyboxCubemap/GreenHouse/Radiance/output_pmrem_256_right_posx_blurred.png",
	//"Data/Images/SkyboxCubemap/GreenHouse/Radiance/output_pmrem_256_top_posy_blurred.png",
	//"Data/Images/SkyboxCubemap/GreenHouse/Radiance/output_pmrem_256_bottom_negy_blurred.png"
	//};
	//
	//m_ownerRenderer->CreateTextureCubeFromFile(imageRadianceTopFilePathList);

	//StreetDusk.png
	//m_ownerRenderer->CreateTextureFromFile("Data/Images/Manhattan2.png");

	//Dented Metal Material
	m_ownerRenderer->CreateTextureFromFile("Data/Images/PBRMaterials/DentedMetal/dented-metal_albedo.png");
	m_ownerRenderer->CreateTextureFromFile("Data/Images/PBRMaterials/DentedMetal/dented-metal_normal-ogl.png");
	m_ownerRenderer->CreateTextureFromFile("Data/Images/PBRMaterials/DentedMetal/dented-metal_roughness.png");
	m_ownerRenderer->CreateTextureFromFile("Data/Images/PBRMaterials/DentedMetal/dented-metal_metallic.png");
	m_ownerRenderer->CreateTextureFromFile("Data/Images/PBRMaterials/DentedMetal/dented-metal_ao.png");

	//Rusty Panels Material
	//m_ownerRenderer->CreateTextureFromFile("Data/Images/PBRMaterials/RustedPanels/rusted-panels_albedo.png");
	//m_ownerRenderer->CreateTextureFromFile("Data/Images/PBRMaterials/RustedPanels/rusted-panels_normal-ogl.png");
	//m_ownerRenderer->CreateTextureFromFile("Data/Images/PBRMaterials/RustedPanels/rusted-panels_roughness.png");
	//m_ownerRenderer->CreateTextureFromFile("Data/Images/PBRMaterials/RustedPanels/rusted-panels_metallic.png");
	//m_ownerRenderer->CreateTextureFromFile("Data/Images/PBRMaterials/RustedPanels/rusted-panels_ao.png");

	//m_ownerRenderer->CreateTextureFromFile("Data/Images/PBRMaterials/WornShinyMetal/worn-shiny-metal-albedo.png");
	//m_ownerRenderer->CreateTextureFromFile("Data/Images/PBRMaterials/WornShinyMetal/worn-shiny-metal-Normal-ogl.png");
	//m_ownerRenderer->CreateTextureFromFile("Data/Images/PBRMaterials/WornShinyMetal/worn-shiny-metal-Roughness.png");
	//m_ownerRenderer->CreateTextureFromFile("Data/Images/PBRMaterials/WornShinyMetal/worn-shiny-metal-Metallic.png");
	//m_ownerRenderer->CreateTextureFromFile("Data/Images/PBRMaterials/WornShinyMetal/worn-shiny-metal-ao.png");

	//Metal Studs
	m_ownerRenderer->CreateTextureFromFile("Data/Images/PBRMaterials/MetalStuds/metal-studs_albedo.png");
	m_ownerRenderer->CreateTextureFromFile("Data/Images/PBRMaterials/MetalStuds/metal-studs_normal-ogl.png");
	m_ownerRenderer->CreateTextureFromFile("Data/Images/PBRMaterials/MetalStuds/metal-studs_roughness.png");
	m_ownerRenderer->CreateTextureFromFile("Data/Images/PBRMaterials/MetalStuds/metal-studs_metallic.png");
	m_ownerRenderer->CreateTextureFromFile("Data/Images/PBRMaterials/MetalStuds/metal-studs_ao.png");

	//Four textures materials

	//Iced Field
	//m_ownerRenderer->CreateTextureFromFile("Data/Images/PBRMaterials/IceField/ice_field_albedo.png");
	//m_ownerRenderer->CreateTextureFromFile("Data/Images/PBRMaterials/IceField/ice_field_normal-dx.png");
	//m_ownerRenderer->CreateTextureFromFile("Data/Images/PBRMaterials/IceField/ice_field_roughness.png");
	//m_ownerRenderer->CreateTextureFromFile("Data/Images/PBRMaterials/IceField/ice_field_metallic.png");
	//m_ownerRenderer->CreateTextureFromFile("Data/Images/PBRMaterials/IceField/ice_field_ao.png");

	//Scuffed Titanium Material
	//m_ownerRenderer->CreateTextureFromFile("Data/Images/PBRMaterials/Titanium/Titanium-Scuffed_basecolor.png");
	//m_ownerRenderer->CreateTextureFromFile("Data/Images/PBRMaterials/Titanium/Titanium-Scuffed_normal.png");
	//m_ownerRenderer->CreateTextureFromFile("Data/Images/PBRMaterials/Titanium/Titanium-Scuffed_roughness.png");
	//m_ownerRenderer->CreateTextureFromFile("Data/Images/PBRMaterials/Titanium/Titanium-Scuffed_metallic.png");
	
	//Scuffed Gold Material
	//m_ownerRenderer->CreateTextureFromFile("Data/Images/PBRMaterials/Gold/gold-scuffed_basecolor-boosted.png");
	//m_ownerRenderer->CreateTextureFromFile("Data/Images/PBRMaterials/Gold/gold-scuffed_normal.png");
	//m_ownerRenderer->CreateTextureFromFile("Data/Images/PBRMaterials/Gold/gold-scuffed_roughness.png");
	//m_ownerRenderer->CreateTextureFromFile("Data/Images/PBRMaterials/Gold/gold-scuffed_metallic.png");

	//Rusty Metal Material
	m_ownerRenderer->CreateTextureFromFile("Data/Images/PBRMaterials/RustyMetal/rusty-metal_albedo.png");
	m_ownerRenderer->CreateTextureFromFile("Data/Images/PBRMaterials/RustyMetal/rusty-metal_normal-ogl.png");
	m_ownerRenderer->CreateTextureFromFile("Data/Images/PBRMaterials/RustyMetal/rusty-metal_roughness.png");
	m_ownerRenderer->CreateTextureFromFile("Data/Images/PBRMaterials/RustyMetal/rusty-metal_metallic.png");
	m_ownerRenderer->CreateTextureFromFile("Data/Images/PBRMaterials/RustyMetal/rusty-metal_ao.png");

	//Brown Floor Material
	//m_ownerRenderer->CreateTextureFromFile("Data/Images/PBRMaterials/Floor/laminate-flooring-brown_albedo.png");
	//m_ownerRenderer->CreateTextureFromFile("Data/Images/PBRMaterials/Floor/laminate-flooring-brown_normal-ogl.png");
	//m_ownerRenderer->CreateTextureFromFile("Data/Images/PBRMaterials/Floor/laminate-flooring-brown_roughness.png");
	//m_ownerRenderer->CreateTextureFromFile("Data/Images/PBRMaterials/Floor/laminate-flooring-brown_metallic.png");
	//m_ownerRenderer->CreateTextureFromFile("Data/Images/PBRMaterials/Floor/laminate-flooring-brown_ao.png");
}


void RayTracingHelper::UpdateTopLevelAS()
{
	Mat44 mat44 = Mat44();
	DirectX::XMMATRIX xmat44Identity = DirectX::XMMATRIX(mat44.m_values);

	Mat44 mat44SphereOne = m_sphereModel_One.GetModelMatrix();
	DirectX::XMMATRIX xmat44SphereOne = DirectX::XMMATRIX(mat44SphereOne.m_values);

	Mat44 mat44SphereTwo = m_sphereModel_Two.GetModelMatrix();
	DirectX::XMMATRIX xmat44SphereTwo = DirectX::XMMATRIX(mat44SphereTwo.m_values);

	Mat44 mat44Pot = m_potModel.GetModelMatrix();
	DirectX::XMMATRIX xmat44Pot = DirectX::XMMATRIX(mat44Pot.m_values);

	m_sphereModelPointLight.m_position = m_ownerRenderer->m_pointLightPositionOne;
	//DebuggerPrintf("(%1f, %1f, %1f)\n", m_sphereModelPointLight.m_position.x, m_sphereModelPointLight.m_position.y, m_sphereModelPointLight.m_position.z);
	Mat44 mat44SpherePointLight = m_sphereModelPointLight.GetModelMatrix();
	DirectX::XMMATRIX xmat44SpherePointLight = DirectX::XMMATRIX(mat44SpherePointLight.m_values);

	Mat44 mat44SphereRed = m_sphereModelRed.GetModelMatrix();
	DirectX::XMMATRIX xmat44SphereRed = DirectX::XMMATRIX(mat44SphereRed.m_values);

	Mat44 mat44SphereYellow = m_sphereModelYellow.GetModelMatrix();
	DirectX::XMMATRIX xmat44SphereYellow = DirectX::XMMATRIX(mat44SphereYellow.m_values);

	m_instances[0].second = xmat44SphereOne;			// First Sphere
	m_instances[1].second = xmat44Identity;				// Plane
	m_instances[2].second = xmat44Pot;					// Pot Model
	m_instances[3].second = xmat44SphereTwo;			// Second Sphere
	m_instances[4].second = xmat44SphereRed;			// Red Sphere
	m_instances[5].second = xmat44SphereYellow;			// Yellow Sphere
	m_instances[6].second = xmat44SpherePointLight;		// Light Sphere

	CreateTopLevelAS(m_instances, true);
	
}

AccelerationStructureBuffers RayTracingHelper::CreateBottomLevelAS(std::vector<std::pair<ComPtr<ID3D12Resource>, uint32_t>> vVertexBuffers, std::vector<std::pair<ComPtr<ID3D12Resource>, uint32_t>> vIndexBuffers /*= {}*/)
{
	BottomLevelASGenerator bottomLevelAS;
	// Adding all vertex buffers and not transforming their position.
	for (size_t i = 0; i < vVertexBuffers.size(); i++) {
		// for (const auto &buffer : vVertexBuffers) {
		if (i < vIndexBuffers.size() && vIndexBuffers[i].second > 0)
			bottomLevelAS.AddVertexBuffer(vVertexBuffers[i].first.Get(), 0,
				vVertexBuffers[i].second, sizeof(Vertex_PCUTBN),
				vIndexBuffers[i].first.Get(), 0,
				vIndexBuffers[i].second, nullptr, 0, true);

		else
			bottomLevelAS.AddVertexBuffer(vVertexBuffers[i].first.Get(), 0,
				vVertexBuffers[i].second, sizeof(Vertex_PCUTBN), 0,
				0);
	}

	// The AS build requires some scratch space to store temporary information.
// The amount of scratch memory is dependent on the scene complexity.
	UINT64 scratchSizeInBytes = 0;
	// The final AS also needs to be stored in addition to the existing vertex
	// buffers. It size is also dependent on the scene complexity.
	UINT64 resultSizeInBytes = 0;

	bottomLevelAS.ComputeASBufferSizes(m_device, false, &scratchSizeInBytes,
		&resultSizeInBytes);

	// Once the sizes are obtained, the application is responsible for allocating
	// the necessary buffers. Since the entire generation will be done on the GPU,
	// we can directly allocate those on the default heap
	AccelerationStructureBuffers buffers;
	buffers.pScratch = CreateBuffer(
		m_device, scratchSizeInBytes,
		D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COMMON,
		m_kDefaultHeapProps);
	buffers.pResult = CreateBuffer(
		m_device, resultSizeInBytes,
		D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
		D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE,
		m_kDefaultHeapProps);

	// Build the acceleration structure. Note that this call integrates a barrier
	// on the generated AS, so that it can be used to compute a top-level AS right
	// after this method.
	bottomLevelAS.Generate(m_commandList, buffers.pScratch.Get(),
		buffers.pResult.Get(), false, nullptr);

	return buffers;
}
//-----------------------------------------------------------------------------
// Create the main acceleration structure that holds all instances of the scene.
// Similarly to the bottom-level AS generation, it is done in 3 steps: gathering
// the instances, computing the memory requirements for the AS, and building the
// AS itself

void RayTracingHelper::CreateTopLevelAS( const std::vector<std::pair<ComPtr<ID3D12Resource>, DirectX::XMMATRIX>> & instances, 
	// pair of bottom level AS and matrix of the instance
// #DXR Extra - Refitting
bool updateOnly // If true the top-level AS will only be refitted and not
				// rebuilt from scratch
) {
	if (!updateOnly) 
	{
		// Gather all the instances into the builder helper
		for (size_t i = 0; i < instances.size(); i++) {
			m_topLevelASGenerator.AddInstance(
				instances[i].first.Get(),
				instances[i].second,
				static_cast<UINT>(i),
				static_cast<UINT>(2 * i) /*2 hit groups per instance*/);
		}
		UINT64 scratchSize, resultSize, instanceDescsSize;

		m_topLevelASGenerator.ComputeASBufferSizes(m_device, true, &scratchSize,
			&resultSize, &instanceDescsSize);

		// Create the scratch and result buffers. Since the build is all done on
		// GPU, those can be allocated on the default heap

		m_topLevelASBuffers.pScratch = CreateBuffer(
			m_device, scratchSize, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COMMON,
			m_kDefaultHeapProps);
		m_ownerRenderer->GetResourceBarrier()->Add(m_topLevelASBuffers.pScratch.Get(), D3D12_RESOURCE_STATE_COMMON,
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		m_ownerRenderer->GetResourceBarrier()->Apply(m_commandList);

		m_topLevelASBuffers.pResult = CreateBuffer(
			m_device, resultSize, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE,
			m_kDefaultHeapProps);

		// The buffer describing the instances: ID, shader binding information,
	// matrices ... Those will be copied into the buffer by the helper through
	// mapping, so the buffer has to be allocated on the upload heap.
		m_topLevelASBuffers.pInstanceDesc = CreateBuffer(
			m_device, instanceDescsSize, D3D12_RESOURCE_FLAG_NONE,
			D3D12_RESOURCE_STATE_GENERIC_READ, m_kUploadHeapProps);
	}


	// After all the buffers are allocated, or if only an update is required, we
	// can build the acceleration structure. Note that in the case of the update
	// we also pass the existing AS as the 'previous' AS, so that it can be
	// refitted in place.
	m_topLevelASGenerator.Generate(m_commandList,
		m_topLevelASBuffers.pScratch.Get(),
		m_topLevelASBuffers.pResult.Get(),
		m_topLevelASBuffers.pInstanceDesc.Get(),
		updateOnly, m_topLevelASBuffers.pResult.Get());
}

void RayTracingHelper::CreateAccelerationStructures()
{

	AccelerationStructureBuffers spheresBottomLevelBuffers = CreateBottomLevelAS(
		{ {m_sphereModel_One.m_vertexBuffer.Get(), m_sphereModel_One.m_vertexNum} }, { {m_sphereModel_One.m_indexBuffer.Get(), m_sphereModel_One.m_indexNum} });

	AccelerationStructureBuffers potBottomLevelBuffers = CreateBottomLevelAS(
		{ {m_potModel.m_vertexBuffer.Get(), m_potModel.m_vertexNum} }, { {m_potModel.m_indexBuffer.Get(), m_potModel.m_indexNum} });
	
	AccelerationStructureBuffers sphereTwoBottomLevelBuffers = CreateBottomLevelAS(
		{ {m_sphereModel_Two.m_vertexBuffer.Get(), m_sphereModel_Two.m_vertexNum} }, { {m_sphereModel_Two.m_indexBuffer.Get(), m_sphereModel_Two.m_indexNum} });

	AccelerationStructureBuffers sphereRedBottomLevelBuffers = CreateBottomLevelAS(
		{ {m_sphereModelRed.m_vertexBuffer.Get(), m_sphereModelRed.m_vertexNum} }, { {m_sphereModelRed.m_indexBuffer.Get(), m_sphereModelRed.m_indexNum} });

	AccelerationStructureBuffers sphereYellowBottomLevelBuffers = CreateBottomLevelAS(
		{ {m_sphereModelYellow.m_vertexBuffer.Get(), m_sphereModelYellow.m_vertexNum} }, { {m_sphereModelYellow.m_indexBuffer.Get(), m_sphereModelYellow.m_indexNum} });

	AccelerationStructureBuffers sphereLightBottomLevelBuffers = CreateBottomLevelAS(
		{ {m_sphereModelPointLight.m_vertexBuffer.Get(), m_sphereModelPointLight.m_vertexNum} }, { {m_sphereModelPointLight.m_indexBuffer.Get(), m_sphereModelPointLight.m_indexNum} });

	AccelerationStructureBuffers planeBottomLevelBuffers = CreateBottomLevelAS({ {m_planeBuffer.Get(), 6} });

	Mat44 mat44 = Mat44();
	DirectX::XMMATRIX xmat44Identity = DirectX::XMMATRIX(mat44.m_values);

	Mat44 mat44SphereOne = m_sphereModel_One.GetModelMatrix();
	DirectX::XMMATRIX xmat44SphereOne = DirectX::XMMATRIX(mat44SphereOne.m_values);

	Mat44 mat44SphereTwo = m_sphereModel_Two.GetModelMatrix();
	DirectX::XMMATRIX xmat44SphereTwo = DirectX::XMMATRIX(mat44SphereTwo.m_values);


	Mat44 mat44Pot = m_potModel.GetModelMatrix();
	DirectX::XMMATRIX xmat44Pot = DirectX::XMMATRIX(mat44Pot.m_values);

	Mat44 mat44SpherePointLight = m_sphereModelPointLight.GetModelMatrix();
	DirectX::XMMATRIX xmat44SpherePointLight = DirectX::XMMATRIX(mat44SpherePointLight.m_values);

	Mat44 mat44SphereRed = m_sphereModelRed.GetModelMatrix();
	DirectX::XMMATRIX xmat44SphereRed = DirectX::XMMATRIX(mat44SphereRed.m_values);

	Mat44 mat44SphereYe = m_sphereModelYellow.GetModelMatrix();
	DirectX::XMMATRIX xmat44SphereYellow = DirectX::XMMATRIX(mat44SphereYe.m_values);

	std::pair<ComPtr<ID3D12Resource>, DirectX::XMMATRIX> instanceSphere;
	instanceSphere.first = spheresBottomLevelBuffers.pResult;
	instanceSphere.second = xmat44SphereOne;
	//instanceSphere.second = xmat44Identity;
	m_instances.push_back(instanceSphere);

	std::pair<ComPtr<ID3D12Resource>, DirectX::XMMATRIX> instancePlane;
	instancePlane.first = planeBottomLevelBuffers.pResult;
	instancePlane.second = xmat44Identity;
	m_instances.push_back(instancePlane); 

	std::pair<ComPtr<ID3D12Resource>, DirectX::XMMATRIX> instancePot;
	instancePot.first = potBottomLevelBuffers.pResult;
	instancePot.second = xmat44Pot;
	m_instances.push_back(instancePot);

	std::pair<ComPtr<ID3D12Resource>, DirectX::XMMATRIX> instanceSphereTwo;
	instanceSphereTwo.first = sphereTwoBottomLevelBuffers.pResult;
	instanceSphereTwo.second = xmat44SphereTwo;
	m_instances.push_back(instanceSphereTwo);

	std::pair<ComPtr<ID3D12Resource>, DirectX::XMMATRIX> instanceSphereRed;
	instanceSphereRed.first = sphereRedBottomLevelBuffers.pResult;
	instanceSphereRed.second = xmat44SphereRed;
	m_instances.push_back(instanceSphereRed);

	std::pair<ComPtr<ID3D12Resource>, DirectX::XMMATRIX> instanceSphereYellow;
	instanceSphereYellow.first = sphereYellowBottomLevelBuffers.pResult;
	instanceSphereYellow.second = xmat44SphereYellow;
	m_instances.push_back(instanceSphereYellow);

	std::pair<ComPtr<ID3D12Resource>, DirectX::XMMATRIX> instanceSphereLight;
	instanceSphereLight.first = sphereLightBottomLevelBuffers.pResult;
	instanceSphereLight.second = xmat44SpherePointLight;
	m_instances.push_back(instanceSphereLight);


	CreateTopLevelAS(m_instances);
	m_ownerRenderer->GetCmdListOwner()->ExecuteCmdLists(m_pipelineState.Get());

	// Store the AS buffers. The rest of the buffers will be released once we exit
	// the function
	m_bottomLevelAS = spheresBottomLevelBuffers.pResult;

}

void RayTracingHelper::SerializeAndCreateRaytracingRootSignature(D3D12_ROOT_SIGNATURE_DESC& desc, ComPtr<ID3D12RootSignature>* rootSig)
{

	ID3DBlob* pSigBlob;
	ID3DBlob* pErrorBlob;

	HRESULT hr = D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1, &pSigBlob, &pErrorBlob);
	if (FAILED(hr))
	{
		ERROR_AND_DIE("Cannot serialize root signature");
	}
	hr = m_device->CreateRootSignature(1, pSigBlob->GetBufferPointer(), pSigBlob->GetBufferSize(), IID_PPV_ARGS(&(*rootSig)));
	if (FAILED(hr))
	{
		ERROR_AND_DIE("Cannot create root signature");
	}
	else
	{
		NAME_D3D12_OBJECT((*rootSig).Get(), L"The Global Root Signature");
	}
}

Microsoft::WRL::ComPtr<ID3D12RootSignature> RayTracingHelper::CreateEmptyLocalSignatureForShaders()
{
	RootSignatureGenerator rsc;
	rsc.AddRootParameter(D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS, 2, 0, 8);
	return rsc.Generate(m_device, true);
}


void RayTracingHelper::CreateLightAndModelBuffer()
{
	m_constBufferSize = sizeof(ConstantsD12);

	// Create the constant buffer for all matrices
	m_constBuffer = CreateBuffer(
		m_device, m_constBufferSize, D3D12_RESOURCE_FLAG_NONE,
		D3D12_RESOURCE_STATE_GENERIC_READ, m_kUploadHeapProps);

	// Create a descriptor heap that will be used by the rasterization shaders
	m_constBufferHeap = CreateDescriptorHeap(
		m_device, 1, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, true);

	// Describe and create the constant buffer view.
	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
	cbvDesc.BufferLocation = m_constBuffer->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = m_constBufferSize;
	
	// Get a handle to the heap memory on the CPU side, to be able to write the
	// descriptors directly
	D3D12_CPU_DESCRIPTOR_HANDLE srvHandle = m_constBufferHeap->GetCPUDescriptorHandleForHeapStart();
	m_device->CreateConstantBufferView(&cbvDesc, srvHandle);
}

void RayTracingHelper::UpdateLightBuffer(ConstantsD12 const& lightConst)
{

	// Copy the light contents
	uint8_t* pData;
	DXCall(m_constBuffer->Map(0, nullptr, (void**)&pData));
	memcpy(pData, &lightConst, m_constBufferSize);
	m_constBuffer->Unmap(0, nullptr);
}

//----------------------------------------------------------------------------------
//
// The camera buffer is a constant buffer that stores the transform matrices of
// the camera, for use by both the rasterization and raytracing. This method
// allocates the buffer where the matrices will be copied. For the sake of code
// clarity, it also creates a heap containing only this buffer, to use in the
// rasterization path.
//
// #DXR Extra: Perspective Camera
void RayTracingHelper::CreateCameraBuffer()
{
	m_cameraBufferSize = sizeof(CameraConstantsD12);

	// Create the constant buffer for all matrices
	m_cameraBuffer = CreateBuffer(
		m_device, m_cameraBufferSize, D3D12_RESOURCE_FLAG_NONE,
		D3D12_RESOURCE_STATE_GENERIC_READ, m_kUploadHeapProps);

	// Create a descriptor heap that will be used by the rasterization shaders
	m_constHeap = CreateDescriptorHeap(
		m_device, 1, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, true);

	// Describe and create the constant buffer view.
	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
	cbvDesc.BufferLocation = m_cameraBuffer->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = m_cameraBufferSize;

	// Get a handle to the heap memory on the CPU side, to be able to write the
	// descriptors directly
	D3D12_CPU_DESCRIPTOR_HANDLE srvHandle = m_constHeap->GetCPUDescriptorHandleForHeapStart();
	m_device->CreateConstantBufferView(&cbvDesc, srvHandle);
}

// #DXR Extra: Perspective Camera
//--------------------------------------------------------------------------------
// Create and copies the view model and perspective matrices of the camera
//

void RayTracingHelper::UpdateCameraBuffer(CameraConstantsD12 const& cameraConst)
{
	//#ToDo: Map the UAV heap memory, not the 
	// Copy the matrix contents
	uint8_t* pData;
	DXCall(m_cameraBuffer->Map(0, nullptr, (void**)&pData));
	memcpy(pData, &cameraConst, m_cameraBufferSize);
	m_cameraBuffer->Unmap(0, nullptr);
}
//-----------------------------------------------------------------------------
//
// Create a vertex buffer for the plane
//
// #DXR Extra: Per-Instance Data
void RayTracingHelper::CreatePlaneVB()
{
	// Define the geometry for a plane.

	float offset = 8.0f;
	Vec3 BL = Vec3(-offset, -offset, 0.f);
	Vec3 BR = Vec3(offset, -offset, 0.f);
	Vec3 TR = Vec3(offset, offset, 0.f);
	Vec3 TL = Vec3(-offset, offset, 0.f);
	std::vector<Vertex_PCUTBN> planeVerts;

	//AddVertsForQuadNormalOnly3D(planeVerts, BL, BR, TR, TL, Rgba8::WHITE);
	AddVertsForQuadNormalOnly3DPCUTBN(planeVerts, BL, BR, TR, TL, Rgba8::WHITE);
	size_t size = planeVerts.size() * (size_t)sizeof(Vertex_PCUTBN);
	const UINT planeBufferSize = (UINT)size;

	CD3DX12_HEAP_PROPERTIES heapProperty = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	CD3DX12_RESOURCE_DESC bufferResource = CD3DX12_RESOURCE_DESC::Buffer(planeBufferSize);
	DXCall(m_device->CreateCommittedResource(&heapProperty, D3D12_HEAP_FLAG_NONE, &bufferResource, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_planeBuffer)));

	// Copy the triangle data to the vertex buffer.

	UINT8* pVertexDataBegin;
	CD3DX12_RANGE readRange(
		0, 0); // We do not intend to read from this resource on the CPU.
	DXCall(m_planeBuffer->Map(
		0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
	memcpy(pVertexDataBegin, planeVerts.data(), size);
	m_planeBuffer->Unmap(0, nullptr);

	// Initialize the vertex buffer view.
	m_planeBufferView.BufferLocation = m_planeBuffer->GetGPUVirtualAddress();
	m_planeBufferView.StrideInBytes = sizeof(Vertex_PCUTBN);
	m_planeBufferView.SizeInBytes = planeBufferSize;
}

//-----------------------------------------------------------------------------
// The ray generation shader needs to access 2 resources: the ray-tracing output
// and the top-level acceleration structure
//
Microsoft::WRL::ComPtr<ID3D12RootSignature> RayTracingHelper::CreateRayGenSignature()
{
	RootSignatureGenerator rsc;
	rsc.AddHeapRangesParameter(
		{	{0 /*u0*/, 1 /*1 descriptor */, 0 /*use the implicit register space 0*/,
			D3D12_DESCRIPTOR_RANGE_TYPE_UAV /* UAV representing the output buffer*/,
			0 /*heap slot where the UAV is defined*/},
			{0 /*t0*/, 1, 0,
			D3D12_DESCRIPTOR_RANGE_TYPE_SRV /*Top-level acceleration structure*/,
			1},
			{0 /*b0*/, 1, 0,
			D3D12_DESCRIPTOR_RANGE_TYPE_CBV /*Camera Constant*/,
			2 }
		}
	);

	return rsc.Generate(m_device, true);
}
//-----------------------------------------------------------------------------
// The hit shader communicates only through the ray payload, and therefore does
// not require any resources
//
Microsoft::WRL::ComPtr<ID3D12RootSignature> RayTracingHelper::CreateHitSignature()
{
	RootSignatureGenerator rsc;
	rsc.AddRootParameter(D3D12_ROOT_PARAMETER_TYPE_SRV, 0/*t0*/);			// vertices
	// Add a single range pointing to the TLAS in the heap
	
	const UINT heapSlotIndex = 7;
	
	UINT fiveMatrialRegisterIndex = 4;//4+0, 4+1, 4+2, 4+3, 4+4
	UINT matelStudMatrialRegisterIndex = 9;//9+0, 9+1, 9+2, 9+3 9+4
	UINT RustedMetalMatrialRegisterIndex = 14;//14+0, 14+1, 14+2, 14+3, 14+4
	//UINT glossyMatrialRegisterIndex = 19;//19+0, 19+1, 19+2, 19+3
	rsc.AddHeapRangesParameter({
		{1 /*t1*/, 1, 0, D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
		 1 /*2nd slot of the heap*/},
		 {1 /*b1*/, 1, 0, D3D12_DESCRIPTOR_RANGE_TYPE_CBV,// Light Constants
		 3 /*4th slot of the heap*/},

		 
		 {3 /*t3*/, 1, 0, D3D12_DESCRIPTOR_RANGE_TYPE_SRV,// Environment texture
		 4 /*5nd slot of the heap*/},
		{3 /*t3*/, 1, 1/*space1*/, D3D12_DESCRIPTOR_RANGE_TYPE_SRV,// Irradiance texture
		5 /*6nd slot of the heap*/},
		{3 /*t3*/, 1, 2/*space2*/, D3D12_DESCRIPTOR_RANGE_TYPE_SRV,// BRDF Lookup texture
		6 /*7nd slot of the heap*/},
		{3 /*t3*/, 1, 3/*space3*/, D3D12_DESCRIPTOR_RANGE_TYPE_SRV,// Radiance texture
		7 /*8nd slot of the heap*/},

		 //Textures PBR 5+4+4
		 {fiveMatrialRegisterIndex + 0 /*t4*/, 1, 0, D3D12_DESCRIPTOR_RANGE_TYPE_SRV,// Albedo floor texture
		 heapSlotIndex + 1 /*9th slot of the heap*/},
		 {fiveMatrialRegisterIndex + 1 /*t5*/, 1, 0, D3D12_DESCRIPTOR_RANGE_TYPE_SRV,// Normal floor texture
		 heapSlotIndex + 2 /*10th slot of the heap*/},
		 {fiveMatrialRegisterIndex + 2 /*t6*/, 1, 0, D3D12_DESCRIPTOR_RANGE_TYPE_SRV,// Roughness floor texture
		  heapSlotIndex + 3 /*11th slot of the heap*/},
		 {fiveMatrialRegisterIndex + 3 /*t7*/, 1, 0, D3D12_DESCRIPTOR_RANGE_TYPE_SRV,// Metallic floor texture
		  heapSlotIndex + 4 /*12th slot of the heap*/},
		 {fiveMatrialRegisterIndex + 4 /*t8*/, 1, 0, D3D12_DESCRIPTOR_RANGE_TYPE_SRV,// AO floor texture
		  heapSlotIndex + 5 /*13th slot of the heap*/},

		 {matelStudMatrialRegisterIndex + 0 /*t9*/, 1, 0, D3D12_DESCRIPTOR_RANGE_TYPE_SRV,// Albedo floor texture
		 heapSlotIndex + 6 /*14th slot of the heap*/},
		 {matelStudMatrialRegisterIndex + 1 /*t10*/, 1, 0, D3D12_DESCRIPTOR_RANGE_TYPE_SRV,// Normal floor texture
		  heapSlotIndex + 7 /*15th slot of the heap*/},
		 {matelStudMatrialRegisterIndex + 2 /*t11*/, 1, 0, D3D12_DESCRIPTOR_RANGE_TYPE_SRV,// Roughness floor texture
		  heapSlotIndex + 8 /*16th slot of the heap*/},
		 {matelStudMatrialRegisterIndex + 3 /*t12*/, 1, 0, D3D12_DESCRIPTOR_RANGE_TYPE_SRV,// Metallic floor texture
		  heapSlotIndex + 9 /*17th slot of the heap*/},
		 {matelStudMatrialRegisterIndex + 4 /*t13*/, 1, 0, D3D12_DESCRIPTOR_RANGE_TYPE_SRV,// Metallic floor texture
		  heapSlotIndex + 10 /*18th slot of the heap*/},

		 {RustedMetalMatrialRegisterIndex + 0 /*t14*/, 1, 0, D3D12_DESCRIPTOR_RANGE_TYPE_SRV,// Albedo floor texture
		 heapSlotIndex + 11 /*19th slot of the heap*/},
		 {RustedMetalMatrialRegisterIndex + 1 /*t15*/, 1, 0, D3D12_DESCRIPTOR_RANGE_TYPE_SRV,// Normal floor texture
		  heapSlotIndex + 12 /*20th slot of the heap*/},
		 {RustedMetalMatrialRegisterIndex + 2 /*t16*/, 1, 0, D3D12_DESCRIPTOR_RANGE_TYPE_SRV,// Roughness floor texture
		  heapSlotIndex + 13 /*21th slot of the heap*/},
		 {RustedMetalMatrialRegisterIndex + 3 /*t17*/, 1, 0, D3D12_DESCRIPTOR_RANGE_TYPE_SRV,// Metallic floor texture
		  heapSlotIndex + 14 /*22th slot of the heap*/},
		 {RustedMetalMatrialRegisterIndex + 4 /*t18*/, 1, 0, D3D12_DESCRIPTOR_RANGE_TYPE_SRV,// AO floor texture
		  heapSlotIndex + 15 /*23th slot of the heap*/},

		 //{glossyMatrialRegisterIndex + 0 /*t19*/, 1, 0, D3D12_DESCRIPTOR_RANGE_TYPE_SRV,// Normal floor texture
		 // 20 /*21th slot of the heap*/},
		 //{glossyMatrialRegisterIndex + 1 /*t20*/, 1, 0, D3D12_DESCRIPTOR_RANGE_TYPE_SRV,// Roughness floor texture
		 // 21 /*22th slot of the heap*/},
		 //{glossyMatrialRegisterIndex + 2 /*t21*/, 1, 0, D3D12_DESCRIPTOR_RANGE_TYPE_SRV,// Metallic floor texture
		 // 22 /*23th slot of the heap*/},
		 //{glossyMatrialRegisterIndex + 3 /*t22*/, 1, 0, D3D12_DESCRIPTOR_RANGE_TYPE_SRV,// AO floor texture
		 // 23 /*24th slot of the heap*/}
		});
	rsc.AddRootParameter(D3D12_ROOT_PARAMETER_TYPE_SRV, 2 /*t2*/); // indices

	return rsc.GenerateWithSampler(m_device, true);
}
//-----------------------------------------------------------------------------
// The miss shader communicates only through the ray payload, and therefore
// does not require any resources
//
Microsoft::WRL::ComPtr<ID3D12RootSignature> RayTracingHelper::CreateMissSignature()
{
	RootSignatureGenerator  rsc;
	rsc.AddHeapRangesParameter({
	 {1 /*b1*/, 1, 0, D3D12_DESCRIPTOR_RANGE_TYPE_CBV,// Constants Buffer
	3 /*4th slot of the heap*/},
	 {3 /*t3*/, 1, 0, D3D12_DESCRIPTOR_RANGE_TYPE_SRV,// Environment Sky box texture
	 4 /*5nd slot of the heap*/}
	 ,
	{3 /*t3*/, 1, 1/*space1*/, D3D12_DESCRIPTOR_RANGE_TYPE_SRV,// Radiance texture
 	 5 /*6nd slot of the heap*/},
	{3 /*t3*/, 1, 2/*space2*/, D3D12_DESCRIPTOR_RANGE_TYPE_SRV,// BRDF Lookup texture
	 6 /*7nd slot of the heap*/}
		});
	return rsc.GenerateWithSampler(m_device, true);
}

void RayTracingHelper::CreateRaytracingPipeline()
{
	RayTracingPipelineGenerator pipeline(m_device);

// The pipeline contains the DXIL code of all the shaders potentially executed
// during the ray-tracing process. This section compiles the HLSL code into a
// set of DXIL libraries. We chose to separate the code in several libraries
// by semantic (ray generation, hit, miss) for clarity. Any code layout can be
// used.
	m_rayGenLibrary = CompileShaderLibrary(L"Data/Shaders/RayGen.hlsl");
	m_missLibrary = CompileShaderLibrary(L"Data/Shaders/Miss.hlsl");
	m_hitLibrary = CompileShaderLibrary(L"Data/Shaders/Hit.hlsl");

	m_shadowLibrary = CompileShaderLibrary(L"Data/Shaders/ShadowRay.hlsl");
	pipeline.AddLibrary(m_shadowLibrary.Get(), { L"ShadowClosestHit", L"ShadowMiss" });
	m_shadowSignature = CreateHitSignature();


// In a way similar to DLLs, each library is associated with a number of
// exported symbols. This
// has to be done explicitly in the lines below. Note that a single library
// can contain an arbitrary number of symbols, whose semantic is given in HLSL
// using the [shader("xxx")] syntax
	pipeline.AddLibrary(m_rayGenLibrary.Get(), { L"RayGen" });
	pipeline.AddLibrary(m_missLibrary.Get(), { L"Miss" });
	// #DXR Extra: Per-Instance Data
	pipeline.AddLibrary(m_hitLibrary.Get(), { L"ClosestHit", L"PlaneClosestHit", L"PureColorClosestHit" });




// To be used, each DX12 shader needs a root signature defining which
// parameters and buffers will be accessed.
	m_rayGenSignature = CreateRayGenSignature();
	m_missSignature = CreateMissSignature();
	m_hitSignature = CreateHitSignature();
	
	NAME_D3D12_OBJECT(m_rayGenSignature.Get(), L"Ray Gen Signature");
	if (m_missSignature.Get()) 
	{
		NAME_D3D12_OBJECT(m_missSignature.Get(), L"Miss Signature");
	}
	
	NAME_D3D12_OBJECT(m_hitSignature.Get(), L"Hit Signature");


// 3 different shaders can be invoked to obtain an intersection: an
// intersection shader is called
// when hitting the bounding box of non-triangular geometry. This is beyond
// the scope of this tutorial. An any-hit shader is called on potential
// intersections. This shader can, for example, perform alpha-testing and
// discard some intersections. Finally, the closest-hit program is invoked on
// the intersection point closest to the ray origin. Those 3 shaders are bound
// together into a hit group.
// Note that for triangular geometry the intersection shader is built-in. An
// empty any-hit shader is also defined by default, so in our simple case each
// hit group contains only the closest hit shader. Note that since the
// exported symbols are defined above the shaders can be simply referred to by
// name.


	pipeline.AddHitGroup(L"HitGroup", L"ClosestHit");
	pipeline.AddHitGroup(L"PotHitGroup", L"ClosestHit");
	pipeline.AddHitGroup(L"SphereSecondHitGroup", L"ClosestHit");
	// #DXR Extra - Another ray type
	// Hit group for all geometry when hit by a shadow ray
	pipeline.AddHitGroup(L"PlaneHitGroup", L"PlaneClosestHit");
	pipeline.AddHitGroup(L"ShadowHitGroup", L"ShadowClosestHit");

	pipeline.AddHitGroup(L"SphereLightHitGroup", L"PureColorClosestHit");
	pipeline.AddHitGroup(L"SphereRedHitGroup", L"ClosestHit");
	pipeline.AddHitGroup(L"SphereYellowHitGroup", L"ClosestHit");
  // The following section associates the root signature to each shader. Note
  // that we can explicitly show that some shaders share the same root signature
  // (example: Miss and ShadowMiss). Note that the hit shaders are now only referred
  // to as hit groups, meaning that the underlying intersection, any-hit and
  // closest-hit shaders share the same root signature.
	pipeline.AddRootSignatureAssociation(m_rayGenSignature.Get(), { L"RayGen" });
	pipeline.AddRootSignatureAssociation(m_missSignature.Get(), { L"Miss", L"ShadowMiss" });
	//pipeline.AddRootSignatureAssociation(m_hitSignature.Get(), { L"HitGroup" });
	// #DXR Extra - Another ray type
	pipeline.AddRootSignatureAssociation(m_shadowSignature.Get(), { L"ShadowHitGroup" });
	// #DXR Extra - Another ray type

	// #DXR Extra: Per-Instance Data
	pipeline.AddRootSignatureAssociation(m_hitSignature.Get(),
		{ L"HitGroup",L"PotHitGroup",L"SphereSecondHitGroup",  L"PlaneHitGroup" ,L"SphereRedHitGroup" ,L"SphereYellowHitGroup" });


	// The payload size defines the maximum size of the data carried by the rays,
	// example the the data
	// exchanged between shaders, such as the HitInfo structure in the HLSL code.
	// It is important to keep this value as low as possible as a too high value
	// would result in unnecessary memory consumption and cache trashing.
	pipeline.SetMaxPayloadSize(4 * sizeof(float)); // RGB + distance

	// Upon hitting a surface, DXR can provide several attributes to the hit. In
	// our sample we just use the barycentric coordinates defined by the weights
	// u,v of the last two vertices of the triangle. The actual barycentric can
	// be obtained using float3 barycentrics = float3(1.f-u-v, u, v);
	pipeline.SetMaxAttributeSize(2 * sizeof(float)); // barycentric coordinates

	// The raytracing process can shoot rays from existing hit points, resulting
	// in nested TraceRay calls. Our sample code traces only primary rays, which
	// then requires a trace depth of 1. Note that this recursion depth should be
	// kept to a minimum for best performance. Path tracing algorithms can be
	// easily flattened into a simple loop in the ray generation.
	// #DXR Extra - Another ray type
	pipeline.SetMaxRecursionDepth(2);

	// Compile the pipeline for execution on the GPU
	m_rtStateObject = pipeline.Generate();

	// Cast the state object into a properties object, allowing to later access
	// the shader pointers by name
	DXCall(m_rtStateObject->QueryInterface(IID_PPV_ARGS(&m_rtStateObjectProps)));

}

// void RayTracingHelper::CreateRayTracingPipelineWithGlobalRootSig()
// {
// 	m_rayTracingLibrary = CompileShaderLibrary(L"Data/Shaders/RayTracing.hlsl");
// 	CD3DX12_STATE_OBJECT_DESC raytracingPipeline{ D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE };
// 	auto lib = raytracingPipeline.CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();
// 	D3D12_SHADER_BYTECODE libdxil = CD3DX12_SHADER_BYTECODE((void*)m_rayTracingLibrary->GetBufferPointer(), m_rayTracingLibrary->GetBufferSize());
// 	lib->SetDXILLibrary(&libdxil);
// 
// 	DXCall(m_device->CreateStateObject(raytracingPipeline, IID_PPV_ARGS(&m_rtStateObject)), L"Couldn't create DirectX Raytracing state object.\n");
// }

//-----------------------------------------------------------------------------
//
// Allocate the buffer holding the raytracing output, with the same size as the
// output image
//
void RayTracingHelper::CreateRaytracingOutputBuffer()
{
	const IntVec2 windowSize = m_ownerRenderer->GetConfig().m_window->GetClientDimensions();
	D3D12_RESOURCE_DESC resDesc = {};
	resDesc.DepthOrArraySize = 1;
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

	resDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	resDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
	resDesc.Width = windowSize.x;
	resDesc.Height = windowSize.y;
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resDesc.MipLevels = 1;
	resDesc.SampleDesc.Count = 1;
	DXCall(m_device->CreateCommittedResource(&m_kDefaultHeapProps, D3D12_HEAP_FLAG_NONE, 
		&resDesc, D3D12_RESOURCE_STATE_COPY_SOURCE, nullptr, IID_PPV_ARGS(&m_outputResource)));
}

void RayTracingHelper::CreateShaderResourceHeap()
{

	// Create a SRV/UAV/CBV descriptor heap. We need 2 entries - 1 UAV for the
	// raytracing output and 1 SRV for the TLAS

	// Get a handle to the heap memory on the CPU side, to be able to write the
	// descriptors directly

	// Create the UAV. Based on the root signature we created it is the first
	// entry. The Create*View methods write the view information directly into
	// srvHandle

	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
	D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = m_ownerRenderer->GetUAVHeap().Allocate().m_cpu;
	m_device->CreateUnorderedAccessView(m_outputResource.Get(), nullptr, &uavDesc, cpuHandle);

	// Add the Top Level AS SRV right after the raytracing output buffer
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.RaytracingAccelerationStructure.Location = m_topLevelASBuffers.pResult->GetGPUVirtualAddress();
	// Write the acceleration structure view in the heap

	cpuHandle = m_ownerRenderer->GetUAVHeap().Allocate().m_cpu;
	m_device->CreateShaderResourceView(nullptr, &srvDesc, cpuHandle);


// Add the constant buffer for the camera after the TLAS

	// Describe and create a constant buffer view for the camera
	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
	cbvDesc.BufferLocation = m_cameraBuffer->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = m_cameraBufferSize;
	cpuHandle = m_ownerRenderer->GetUAVHeap().Allocate().m_cpu;
	m_device->CreateConstantBufferView(&cbvDesc, cpuHandle);

	D3D12_CONSTANT_BUFFER_VIEW_DESC litCbvDesc = {};
	litCbvDesc.BufferLocation = m_constBuffer->GetGPUVirtualAddress();
	litCbvDesc.SizeInBytes = m_constBufferSize;
	cpuHandle = m_ownerRenderer->GetUAVHeap().Allocate().m_cpu;
	m_device->CreateConstantBufferView(&litCbvDesc, cpuHandle);
}

void RayTracingHelper::CreateShaderBindingTable()
{
	// The SBT helper class collects calls to Add*Program.  If called several
// times, the helper must be emptied before re-adding shaders.
	m_sbtHelper.Reset();

	// The pointer to the beginning of the heap is the only parameter required by
	// shaders without root parameters
	D3D12_GPU_DESCRIPTOR_HANDLE srvUavHeapHandle = m_ownerRenderer->GetUAVHeap().GetHeap()->GetGPUDescriptorHandleForHeapStart();

	// The helper treats both root parameter pointers and heap pointers as void*,
	// while DX12 uses the
	// D3D12_GPU_DESCRIPTOR_HANDLE to define heap pointers. The pointer in this
	// struct is a UINT64, which then has to be reinterpreted as a pointer.
	auto heapPointer = reinterpret_cast<UINT64*>(srvUavHeapHandle.ptr);

	// The ray generation only uses heap data
	m_sbtHelper.AddRayGenerationProgram(L"RayGen", { heapPointer });



	// The miss and hit shaders do not access any external resources: instead they
	// communicate their results through the ray payload

	m_sbtHelper.AddMissProgram(L"Miss", { heapPointer });
	m_sbtHelper.AddMissProgram(L"ShadowMiss", { heapPointer });

	//Sphere hit group
	m_sbtHelper.AddHitGroup(L"HitGroup", { 
		(void*)(m_sphereModel_One.m_vertexBuffer->GetGPUVirtualAddress()),
		heapPointer,
		(void*)(m_sphereModel_One.m_indexBuffer->GetGPUVirtualAddress()) });
	m_sbtHelper.AddHitGroup(L"ShadowHitGroup", { heapPointer });


	m_sbtHelper.AddHitGroup(L"PlaneHitGroup", { (void*)(m_planeBuffer->GetGPUVirtualAddress()), heapPointer });
	// #DXR Extra - Another ray type
	m_sbtHelper.AddHitGroup(L"ShadowHitGroup", { heapPointer });

	//Pot hit group
	m_sbtHelper.AddHitGroup(L"PotHitGroup", {
		(void*)(m_potModel.m_vertexBuffer->GetGPUVirtualAddress()),
		heapPointer,
		(void*)(m_potModel.m_indexBuffer->GetGPUVirtualAddress()) });
	m_sbtHelper.AddHitGroup(L"ShadowHitGroup", { heapPointer });
	//Second Sphere
	m_sbtHelper.AddHitGroup(L"SphereSecondHitGroup", {
	(void*)(m_sphereModel_Two.m_vertexBuffer->GetGPUVirtualAddress()),
	heapPointer,
	(void*)(m_sphereModel_Two.m_indexBuffer->GetGPUVirtualAddress()) });
	m_sbtHelper.AddHitGroup(L"ShadowHitGroup", { heapPointer });

	m_sbtHelper.AddHitGroup(L"SphereRedHitGroup", {
	(void*)(m_sphereModelRed.m_vertexBuffer->GetGPUVirtualAddress()),
	heapPointer,
	(void*)(m_sphereModelRed.m_indexBuffer->GetGPUVirtualAddress()) });
	m_sbtHelper.AddHitGroup(L"ShadowHitGroup", { heapPointer });

	m_sbtHelper.AddHitGroup(L"SphereYellowHitGroup", {
	(void*)(m_sphereModelYellow.m_vertexBuffer->GetGPUVirtualAddress()),
	heapPointer,
	(void*)(m_sphereModelYellow.m_indexBuffer->GetGPUVirtualAddress()) });
	m_sbtHelper.AddHitGroup(L"ShadowHitGroup", { heapPointer });


	m_sbtHelper.AddHitGroup(L"SphereLightHitGroup", {
	(void*)(m_sphereModelPointLight.m_vertexBuffer->GetGPUVirtualAddress()),
	heapPointer,
	(void*)(m_sphereModelPointLight.m_indexBuffer->GetGPUVirtualAddress()) });



	// Compute the size of the SBT given the number of shaders and their
	// parameters
	uint32_t sbtSize = m_sbtHelper.ComputeSBTSize();

	// Create the SBT on the upload heap. This is required as the helper will use
	// mapping to write the SBT contents. After the SBT compilation it could be
	// copied to the default heap for performance.
	m_sbtStorage = CreateBuffer(m_device, sbtSize, D3D12_RESOURCE_FLAG_NONE,
		D3D12_RESOURCE_STATE_GENERIC_READ, m_kUploadHeapProps);
	if (!m_sbtStorage) {
		ERROR_AND_DIE("Could not allocate the shader binding table");
	}
	// Compile the SBT from the shader and parameters info
	m_sbtHelper.Generate(m_sbtStorage.Get(), m_rtStateObjectProps.Get());
}

void RayTracingHelper::OnRender()
{
	PopulateCommandList();
}

void RayTracingHelper::PopulateCommandList()
{
	// #DXR
	// Bind the descriptor heap giving access to the top-level acceleration
	// structure, as well as the raytracing output
	m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());
	if (m_raytracingGlobalRootSignature) 
	{
		m_commandList->SetGraphicsRootSignature(m_rayGenSignature.Get());
		m_commandList->SetGraphicsRoot32BitConstant(0, 0, 0);

		m_commandList->SetComputeRootSignature(m_raytracingGlobalRootSignature.Get());
	}

	
	std::vector<ID3D12DescriptorHeap*> heaps = { m_ownerRenderer->GetUAVHeap().GetHeap() };
	m_commandList->SetDescriptorHeaps(static_cast<UINT>(heaps.size()), heaps.data());
	m_ownerRenderer->GetResourceBarrier()->Add(m_outputResource.Get(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	m_ownerRenderer->GetResourceBarrier()->Apply(m_commandList);


	// Setup the raytracing task
	D3D12_DISPATCH_RAYS_DESC desc = {};
	// The layout of the SBT is as follows: ray generation shader, miss
	// shaders, hit groups. As described in the CreateShaderBindingTable method,
	// all SBT entries of a given type have the same size to allow a fixed
	// stride.

		// The ray generation shaders are always at the beginning of the SBT.
	uint32_t rayGenerationSectionSizeInBytes = m_sbtHelper.GetRayGenSectionSize();
	desc.RayGenerationShaderRecord.StartAddress = m_sbtStorage->GetGPUVirtualAddress();
	desc.RayGenerationShaderRecord.SizeInBytes = rayGenerationSectionSizeInBytes;

	// The miss shaders are in the second SBT section, right after the ray
	// generation shader. We have one miss shader for the camera rays and one
	// for the shadow rays, so this section has a size of 2*m_sbtEntrySize. We
	// also indicate the stride between the two miss shaders, which is the size
	// of a SBT entry
	uint32_t missSectionSizeInBytes = m_sbtHelper.GetMissSectionSize();
	desc.MissShaderTable.StartAddress = m_sbtStorage->GetGPUVirtualAddress() + rayGenerationSectionSizeInBytes;
	desc.MissShaderTable.SizeInBytes = missSectionSizeInBytes;
	desc.MissShaderTable.StrideInBytes = m_sbtHelper.GetMissEntrySize();

	// The hit groups section start after the miss shaders. In this sample we
	// have one 1 hit group for the triangle
	uint32_t hitGroupsSectionSize = m_sbtHelper.GetHitGroupSectionSize();
	desc.HitGroupTable.StartAddress = m_sbtStorage->GetGPUVirtualAddress() + rayGenerationSectionSizeInBytes + missSectionSizeInBytes;
	//Another half of 32 will be shadow ray miss shader
	desc.HitGroupTable.SizeInBytes = hitGroupsSectionSize;
	desc.HitGroupTable.StrideInBytes = m_sbtHelper.GetHitGroupEntrySize();

	// Dimensions of the image to render, identical to a kernel launch dimension
	const IntVec2 windowSize = m_ownerRenderer->GetConfig().m_window->GetClientDimensions();
	desc.Width = windowSize.x;
	desc.Height = windowSize.y;
	desc.Depth = 1;

	// Bind the raytracing pipeline
	m_commandList->SetPipelineState1(m_rtStateObject.Get());
	// Dispatch the rays and write to the raytracing output
	m_commandList->DispatchRays(&desc);

	TransitResource(m_ownerRenderer->GetSurface());

}

void RayTracingHelper::TransitResource(D12Surface* surface)
{
	GUARANTEE_OR_DIE(surface, "surface is nullptr");
	// The raytracing output needs to be copied to the actual render target used
// for display. For this, we need to transition the raytracing output from a
// UAV to a copy source, and the render target buffer to a copy destination.
// We can then do the actual copy, before transitioning the render target
// buffer into a render target, that will be then used to display the image
	m_ownerRenderer->GetResourceBarrier()->Add(m_outputResource.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE);
	m_ownerRenderer->GetResourceBarrier()->Apply(m_commandList);
	
	m_ownerRenderer->GetResourceBarrier()->Add(surface->GetBackBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_DEST);
	m_ownerRenderer->GetResourceBarrier()->Apply(m_commandList);

	m_commandList->CopyResource(surface->GetBackBuffer(), m_outputResource.Get());

	m_ownerRenderer->GetResourceBarrier()->Add(surface->GetBackBuffer(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_RENDER_TARGET);
	m_ownerRenderer->GetResourceBarrier()->Apply(m_commandList);

}

ID3D12Resource* RayTracingHelper::CreateBuffer(id3d12Device* device, uint64_t size, D3D12_RESOURCE_FLAGS flags, D3D12_RESOURCE_STATES initState, const D3D12_HEAP_PROPERTIES& heapProps)
{
	D3D12_RESOURCE_DESC bufDesc = {};
	bufDesc.Alignment = 0;
	bufDesc.DepthOrArraySize = 1;
	bufDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	bufDesc.Flags = flags;
	bufDesc.Format = DXGI_FORMAT_UNKNOWN;
	bufDesc.Height = 1;
	bufDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	bufDesc.MipLevels = 1;
	bufDesc.SampleDesc.Count = 1;
	bufDesc.SampleDesc.Quality = 0;
	bufDesc.Width = size;

	ID3D12Resource* pBuffer;
	DXCall(device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &bufDesc,
		initState, nullptr, IID_PPV_ARGS(&pBuffer)));
	return pBuffer;
}


//--------------------------------------------------------------------------------------------------
//
// Store data related to a DXIL library: the library itself, the exported symbols, and the
// associated descriptors


#endif // ENABLE_D3D12