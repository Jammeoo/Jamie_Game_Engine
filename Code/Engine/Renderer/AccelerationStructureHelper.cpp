#include "Engine/Renderer/AccelerationStructureHelper.hpp"

#ifdef ENABLE_D3D12

#include <unordered_set>




//--------------------------------------------------------------------------------------------------
//
// Add an instance to the top-level acceleration structure. The instance is
// represented by a bottom-level AS, a transform, an instance ID and the index
// of the hit group indicating which shaders are executed upon hitting any
// geometry within the instance
void TopLevelASGenerator::AddInstance(
	ID3D12Resource* bottomLevelAS,      // Bottom-level acceleration structure containing the
										// actual geometric data of the instance
	const DirectX::XMMATRIX& transform, // Transform matrix to apply to the instance, allowing the
										// same bottom-level AS to be used at several world-space
										// positions
	UINT instanceID,                    // Instance ID, which can be used in the shaders to
										// identify this specific instance
	UINT hitGroupIndex                  // Hit group index, corresponding the the index of the
										// hit group in the Shader Binding Table that will be
										// invocated upon hitting the geometry
)
{
	//float const* trans = transform.r[0].m128_f32;
	//Mat44 mat;
	//
	//for (int i = 0; i < 16; i++) 
	//{
	//	mat.m_values[i] = trans[i];
	//}
 	m_instances.emplace_back(Instance(bottomLevelAS, transform, instanceID, hitGroupIndex));
}

//--------------------------------------------------------------------------------------------------
//
// Compute the size of the scratch space required to build the acceleration
// structure, as well as the size of the resulting structure. The allocation of
// the buffers is then left to the application
void TopLevelASGenerator::ComputeASBufferSizes(
	id3d12Device* device, // Device on which the build will be performed
	bool allowUpdate,                        // If true, the resulting acceleration structure will
											 // allow iterative updates
	UINT64* scratchSizeInBytes,              // Required scratch memory on the GPU to build
											 // the acceleration structure
	UINT64* resultSizeInBytes,               // Required GPU memory to store the acceleration
											 // structure
	UINT64* descriptorsSizeInBytes           // Required GPU memory to store instance
											 // descriptors, containing the matrices,
											 // indices etc.
)
{
// The generated AS can support iterative updates. This may change the final
// size of the AS as well as the temporary memory requirements, and hence has
// to be set before the actual build
	m_flags = allowUpdate ? D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE
		: D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE;

	// Describe the work being requested, in this case the construction of a
	// (possibly dynamic) top-level hierarchy, with the given instance descriptors
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS
		prebuildDesc = {};
	prebuildDesc.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
	prebuildDesc.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	prebuildDesc.NumDescs = static_cast<UINT>(m_instances.size());
	prebuildDesc.Flags = m_flags;

	// This structure is used to hold the sizes of the required scratch memory and
	// resulting AS
	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO info = {};

	// Building the acceleration structure (AS) requires some scratch space, as
	// well as space to store the resulting structure This function computes a
	// conservative estimate of the memory requirements for both, based on the
	// number of bottom-level instances.
	device->GetRaytracingAccelerationStructurePrebuildInfo(&prebuildDesc, &info);

	// Buffer sizes need to be 256-byte-aligned
	info.ResultDataMaxSizeInBytes =
		ROUND_UP(info.ResultDataMaxSizeInBytes, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
	info.ScratchDataSizeInBytes =
		ROUND_UP(info.ScratchDataSizeInBytes, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);

	m_resultSizeInBytes = info.ResultDataMaxSizeInBytes;
	m_scratchSizeInBytes = info.ScratchDataSizeInBytes;
	// The instance descriptors are stored as-is in GPU memory, so we can deduce
	// the required size from the instance count
	m_instanceDescsSizeInBytes =
		ROUND_UP(sizeof(D3D12_RAYTRACING_INSTANCE_DESC) * static_cast<UINT64>(m_instances.size()),
			D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);

	*scratchSizeInBytes = m_scratchSizeInBytes;
	*resultSizeInBytes = m_resultSizeInBytes;
	*descriptorsSizeInBytes = m_instanceDescsSizeInBytes;
}




//--------------------------------------------------------------------------------------------------
//
// Enqueue the construction of the acceleration structure on a command list,
// using application-provided buffers and possibly a pointer to the previous
// acceleration structure in case of iterative updates. Note that the update can
// be done in place: the result and previousResult pointers can be the same.
void TopLevelASGenerator::Generate(
	id3d12GraphicsCommandList* commandList, // Command list on which the build will be enqueued
	ID3D12Resource* scratchBuffer,     // Scratch buffer used by the builder to
									   // store temporary data
	ID3D12Resource* resultBuffer,      // Result buffer storing the acceleration structure
	ID3D12Resource* descriptorsBuffer, // Auxiliary result buffer containing the instance
									   // descriptors, has to be in upload heap
	bool updateOnly /*= false*/,       // If true, simply refit the existing
									   // acceleration structure
	ID3D12Resource* previousResult /*= nullptr*/ // Optional previous acceleration
												 // structure, used if an iterative update
												 // is requested
)
{
	// Copy the descriptors in the target descriptor buffer
	D3D12_RAYTRACING_INSTANCE_DESC* instanceDescs;
	descriptorsBuffer->Map(0, nullptr, reinterpret_cast<void**>(&instanceDescs));
	if (!instanceDescs)
	{
		ERROR_AND_DIE("Cannot map the instance descriptor buffer - is it in the upload heap?");
	}

	auto instanceCount = static_cast<UINT>(m_instances.size());

	// Initialize the memory to zero on the first time only
	if (!updateOnly)
	{
		ZeroMemory(instanceDescs, m_instanceDescsSizeInBytes);
	}

	// Create the description for each instance
	for (uint32_t i = 0; i < instanceCount; i++)
	{
		// Instance ID visible in the shader in InstanceID()
		instanceDescs[i].InstanceID = m_instances[i].instanceID;
		// Index of the hit group invoked upon intersection
		instanceDescs[i].InstanceContributionToHitGroupIndex = m_instances[i].hitGroupIndex;
		// Instance flags, including back-face culling, winding, etc - TODO: should
		// be accessible from outside
		instanceDescs[i].Flags = D3D12_RAYTRACING_INSTANCE_FLAG_NONE;
		// Instance transform matrix
		//Mat44 matTransposed = m_instances[i].m_transform;
		//matTransposed.Transpose();
		DirectX::XMMATRIX m = XMMatrixTranspose(
			m_instances[i].transform); // GLM is column major, the INSTANCE_DESC is row major

		memcpy(instanceDescs[i].Transform, &m, sizeof(instanceDescs[i].Transform));
		// Get access to the bottom level
		instanceDescs[i].AccelerationStructure = m_instances[i].bottomLevelAS->GetGPUVirtualAddress();
		// Visibility mask, always visible here - TODO: should be accessible from
		// outside
		instanceDescs[i].InstanceMask = 0xFF;
	}

	descriptorsBuffer->Unmap(0, nullptr);

	// If this in an update operation we need to provide the source buffer
	D3D12_GPU_VIRTUAL_ADDRESS pSourceAS = updateOnly ? previousResult->GetGPUVirtualAddress() : 0;

	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS flags = m_flags;
	// The stored flags represent whether the AS has been built for updates or
	// not. If yes and an update is requested, the builder is told to only update
	// the AS instead of fully rebuilding it
	if (flags == D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE && updateOnly)
	{
		flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PERFORM_UPDATE;
	}

	// Sanity checks
	if (m_flags != D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE && updateOnly)
	{
		ERROR_AND_DIE("Cannot update a top-level AS not originally built for updates");
	}
	if (updateOnly && previousResult == nullptr)
	{
		ERROR_AND_DIE("Top-level hierarchy update requires the previous hierarchy");
	}

	// Create a descriptor of the requested builder work, to generate a top-level
	// AS from the input parameters
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC buildDesc = {};
	buildDesc.Inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
	buildDesc.Inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	buildDesc.Inputs.InstanceDescs = descriptorsBuffer->GetGPUVirtualAddress();
	buildDesc.Inputs.NumDescs = instanceCount;
	buildDesc.DestAccelerationStructureData = { resultBuffer->GetGPUVirtualAddress()
	};
	buildDesc.ScratchAccelerationStructureData = { scratchBuffer->GetGPUVirtualAddress()
	};
	buildDesc.SourceAccelerationStructureData = pSourceAS;
	buildDesc.Inputs.Flags = flags;

	// Build the top-level AS
	commandList->BuildRaytracingAccelerationStructure(&buildDesc, 0, nullptr);

	// Wait for the builder to complete by setting a barrier on the resulting
	// buffer. This can be important in case the rendering is triggered
	// immediately afterwards, without executing the command list
	D3D12_RESOURCE_BARRIER uavBarrier;
	uavBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
	uavBarrier.UAV.pResource = resultBuffer;
	uavBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	commandList->ResourceBarrier(1, &uavBarrier);
}


TopLevelASGenerator::Instance::Instance(ID3D12Resource* blAS, const DirectX::XMMATRIX& tr, UINT iID,
	UINT hgId)
	: bottomLevelAS(blAS), transform(tr), instanceID(iID), hitGroupIndex(hgId)
{
}

//--------------------------------------------------------------------------------------------------
// Add a vertex buffer in GPU memory into the acceleration structure. The
// vertices are supposed to be represented by 3 float32 value
void BottomLevelASGenerator::AddVertexBuffer(
	ID3D12Resource* vertexBuffer, // Buffer containing the vertex coordinates,
								  // possibly interleaved with other vertex data
	UINT64
	vertexOffsetInBytes, // Offset of the first vertex in the vertex buffer
	uint32_t vertexCount,    // Number of vertices to consider in the buffer
	UINT vertexSizeInBytes,  // Size of a vertex including all its other data,
							 // used to stride in the buffer
	ID3D12Resource* transformBuffer, // Buffer containing a 4x4 transform matrix
									 // in GPU memory, to be applied to the
									 // vertices. This buffer cannot be nullptr
	UINT64 transformOffsetInBytes,   // Offset of the transform matrix in the
									 // transform buffer
	bool isOpaque /* = true */ // If true, the geometry is considered opaque,
							   // optimizing the search for a closest hit
) {
	AddVertexBuffer(vertexBuffer, vertexOffsetInBytes, vertexCount,
		vertexSizeInBytes, nullptr, 0, 0, transformBuffer,
		transformOffsetInBytes, isOpaque);
}

//--------------------------------------------------------------------------------------------------
// Add a vertex buffer along with its index buffer in GPU memory into the
// acceleration structure. The vertices are supposed to be represented by 3
// float32 value. This implementation limits the original flexibility of the
// API:
//   - triangles (no custom intersector support)
//   - 3xfloat32 format
//   - 32-bit indices
void BottomLevelASGenerator::AddVertexBuffer(
	ID3D12Resource* vertexBuffer, // Buffer containing the vertex coordinates,
								  // possibly interleaved with other vertex data
	UINT64
	vertexOffsetInBytes, // Offset of the first vertex in the vertex buffer
	uint32_t vertexCount,    // Number of vertices to consider in the buffer
	UINT vertexSizeInBytes,  // Size of a vertex including all its other data,
							 // used to stride in the buffer
	ID3D12Resource* indexBuffer, // Buffer containing the vertex indices
								 // describing the triangles
	UINT64 indexOffsetInBytes, // Offset of the first index in the index buffer
	uint32_t indexCount,       // Number of indices to consider in the buffer
	ID3D12Resource* transformBuffer, // Buffer containing a 4x4 transform matrix
									 // in GPU memory, to be applied to the
									 // vertices. This buffer cannot be nullptr
	UINT64 transformOffsetInBytes,   // Offset of the transform matrix in the
									 // transform buffer
	bool isOpaque /* = true */ // If true, the geometry is considered opaque,
							   // optimizing the search for a closest hit
) {
	// Create the DX12 descriptor representing the input data, assumed to be
	// opaque triangles, with 3xf32 vertex coordinates and 32-bit indices
	D3D12_RAYTRACING_GEOMETRY_DESC descriptor = {};
	descriptor.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
	descriptor.Triangles.VertexBuffer.StartAddress =
		vertexBuffer->GetGPUVirtualAddress() + vertexOffsetInBytes;
	descriptor.Triangles.VertexBuffer.StrideInBytes = vertexSizeInBytes;
	descriptor.Triangles.VertexCount = vertexCount;
	descriptor.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
	descriptor.Triangles.IndexBuffer =
		indexBuffer ? (indexBuffer->GetGPUVirtualAddress() + indexOffsetInBytes)
		: 0;
	descriptor.Triangles.IndexFormat =
		indexBuffer ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_UNKNOWN;
	descriptor.Triangles.IndexCount = indexCount;
	descriptor.Triangles.Transform3x4 =
		transformBuffer
		? (transformBuffer->GetGPUVirtualAddress() + transformOffsetInBytes)
		: 0;
	descriptor.Flags = isOpaque ? D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE
		: D3D12_RAYTRACING_GEOMETRY_FLAG_NONE;

	m_vertexBuffers.push_back(descriptor);
}


//--------------------------------------------------------------------------------------------------
// Compute the size of the scratch space required to build the acceleration
// structure, as well as the size of the resulting structure. The allocation of
// the buffers is then left to the application
void BottomLevelASGenerator::ComputeASBufferSizes(
	id3d12Device* device, // Device on which the build will be performed
	bool allowUpdate,     // If true, the resulting acceleration structure will
						  // allow iterative updates
	UINT64* scratchSizeInBytes, // Required scratch memory on the GPU to build
								// the acceleration structure
	UINT64* resultSizeInBytes   // Required GPU memory to store the acceleration
								// structure
) {
	// The generated AS can support iterative updates. This may change the final
	// size of the AS as well as the temporary memory requirements, and hence has
	// to be set before the actual build
	m_flags =
		allowUpdate
		? D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE
		: D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE;

	// Describe the work being requested, in this case the construction of a
	// (possibly dynamic) bottom-level hierarchy, with the given vertex buffers

	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS prebuildDesc;
	prebuildDesc.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
	prebuildDesc.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	prebuildDesc.NumDescs = static_cast<UINT>(m_vertexBuffers.size());
	prebuildDesc.pGeometryDescs = m_vertexBuffers.data();
	prebuildDesc.Flags = m_flags;

	// This structure is used to hold the sizes of the required scratch memory and
	// resulting AS
	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO info = {};

	// Building the acceleration structure (AS) requires some scratch space, as
	// well as space to store the resulting structure This function computes a
	// conservative estimate of the memory requirements for both, based on the
	// geometry size.
	device->GetRaytracingAccelerationStructurePrebuildInfo(&prebuildDesc, &info);

	// Buffer sizes need to be 256-byte-aligned
	*scratchSizeInBytes =
		ROUND_UP(info.ScratchDataSizeInBytes,
			D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
	*resultSizeInBytes = ROUND_UP(info.ResultDataMaxSizeInBytes,
		D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);

	// Store the memory requirements for use during build
	m_scratchSizeInBytes = *scratchSizeInBytes;
	m_resultSizeInBytes = *resultSizeInBytes;
}


//--------------------------------------------------------------------------------------------------
// Enqueue the construction of the acceleration structure on a command list,
// using application-provided buffers and possibly a pointer to the previous
// acceleration structure in case of iterative updates. Note that the update can
// be done in place: the result and previousResult pointers can be the same.
void BottomLevelASGenerator::Generate(
	id3d12GraphicsCommandList
	* commandList, // Command list on which the build will be enqueued
	ID3D12Resource* scratchBuffer, // Scratch buffer used by the builder to
								   // store temporary data
	ID3D12Resource
	* resultBuffer, // Result buffer storing the acceleration structure
	bool updateOnly,   // If true, simply refit the existing
					   // acceleration structure
	ID3D12Resource* previousResult // Optional previous acceleration
								   // structure, used if an iterative update
								   // is requested
) {

	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS flags = m_flags;
	// The stored flags represent whether the AS has been built for updates or
	// not. If yes and an update is requested, the builder is told to only update
	// the AS instead of fully rebuilding it
	if (flags ==
		D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE &&
		updateOnly) {
		flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PERFORM_UPDATE;
	}

	// Sanity checks
	if (m_flags !=
		D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE &&
		updateOnly) {
		ERROR_AND_DIE("Cannot update a bottom-level AS not originally built for updates");
	}
	if (updateOnly && previousResult == nullptr) {
		ERROR_AND_DIE("Bottom-level hierarchy update requires the previous hierarchy");
	}

	if (m_resultSizeInBytes == 0 || m_scratchSizeInBytes == 0) {
		ERROR_AND_DIE("Invalid scratch and result buffer sizes - ComputeASBufferSizes needs to be called before Build");
	}
	// Create a descriptor of the requested builder work, to generate a
	// bottom-level AS from the input parameters
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC buildDesc;
	buildDesc.Inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
	buildDesc.Inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	buildDesc.Inputs.NumDescs = static_cast<UINT>(m_vertexBuffers.size());
	buildDesc.Inputs.pGeometryDescs = m_vertexBuffers.data();
	buildDesc.DestAccelerationStructureData = {
		resultBuffer->GetGPUVirtualAddress() };
	buildDesc.ScratchAccelerationStructureData = {
		scratchBuffer->GetGPUVirtualAddress() };
	buildDesc.SourceAccelerationStructureData =
		previousResult ? previousResult->GetGPUVirtualAddress() : 0;
	buildDesc.Inputs.Flags = flags;

	// Build the AS
	commandList->BuildRaytracingAccelerationStructure(&buildDesc, 0, nullptr);

	// Wait for the builder to complete by setting a barrier on the resulting
	// buffer. This is particularly important as the construction of the top-level
	// hierarchy may be called right afterwards, before executing the command
	// list.
	D3D12_RESOURCE_BARRIER uavBarrier;
	uavBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
	uavBarrier.UAV.pResource = resultBuffer;
	uavBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	commandList->ResourceBarrier(1, &uavBarrier);
}


RayTracingPipelineGenerator::RayTracingPipelineGenerator(id3d12Device* device) : m_device(device)
{
// The pipeline creation requires having at least one empty global and local root signatures, so
// we systematically create both, as this does not incur any overhead
	CreateDummyRootSignatures();
	
}

void RayTracingPipelineGenerator::AddLibrary(IDxcBlob* dxilLibrary, const std::vector<std::wstring>& symbolExports)
{
	m_libraries.emplace_back(Library(dxilLibrary, symbolExports));
}

//--------------------------------------------------------------------------------------------------
//
// In DXR the hit-related shaders are grouped into hit groups. Such shaders are:
// - The intersection shader, which can be used to intersect custom geometry, and is called upon
//   hitting the bounding box the the object. A default one exists to intersect triangles
// - The any hit shader, called on each intersection, which can be used to perform early
//   alpha-testing and allow the ray to continue if needed. Default is a pass-through.
// - The closest hit shader, invoked on the hit point closest to the ray start.
// The shaders in a hit group share the same root signature, and are only referred to by the
// hit group name in other places of the program.
void RayTracingPipelineGenerator::AddHitGroup(const std::wstring& hitGroupName,
	const std::wstring& closestHitSymbol,
	const std::wstring& anyHitSymbol /*= L""*/,
	const std::wstring& intersectionSymbol /*= L""*/)
{
	m_hitGroups.emplace_back(
		HitGroup(hitGroupName, closestHitSymbol, anyHitSymbol, intersectionSymbol));
}

void RayTracingPipelineGenerator::AddRootSignatureAssociation(ID3D12RootSignature* rootSignature, const std::vector<std::wstring>& symbols)
{
	m_rootSignatureAssociations.emplace_back(RootSignatureAssociation(rootSignature, symbols));
}
//--------------------------------------------------------------------------------------------------
//
// The payload is the way hit or miss shaders can exchange data with the shader that called
// TraceRay. When several ray types are used (e.g. primary and shadow rays), this value must be
// the largest possible payload size. Note that to optimize performance, this size must be kept
// as low as possible.
void RayTracingPipelineGenerator::SetMaxPayloadSize(UINT sizeInBytes)
{
	m_maxPayLoadSizeInBytes = sizeInBytes;
}
//--------------------------------------------------------------------------------------------------
//
// When hitting geometry, a number of surface attributes can be generated by the intersector.
// Using the built-in triangle intersector the attributes are the barycentric coordinates, with a
// size 2*sizeof(float).
void RayTracingPipelineGenerator::SetMaxAttributeSize(UINT sizeInBytes)
{
	m_maxAttributeSizeInBytes = sizeInBytes;
}

void RayTracingPipelineGenerator::SetMaxRecursionDepth(UINT maxDepth)
{
	m_maxRecursionDepth = maxDepth;
}
//--------------------------------------------------------------------------------------------------
//
// Compiles the ray-tracing state object
ID3D12StateObject* RayTracingPipelineGenerator::Generate()
{
	// The pipeline is made of a set of sub-objects, representing the DXIL libraries, hit group
// declarations, root signature associations, plus some configuration objects
	UINT64 subobjectCount =
		m_libraries.size() +                     // DXIL libraries
		m_hitGroups.size() +                     // Hit group declarations
		1 +                                      // Shader configuration
		1 +                                      // Shader payload
		2 * m_rootSignatureAssociations.size() + // Root signature declaration + association
		2 +                                      // Empty global and local root signatures
		1;                                       // Final pipeline subobject

	// Initialize a vector with the target object count. It is necessary to make the allocation before
	// adding subobjects as some subobjects reference other subobjects by pointer. Using push_back may
	// reallocate the array and invalidate those pointers.
	std::vector<D3D12_STATE_SUBOBJECT> subobjects(subobjectCount);

	UINT currentIndex = 0;

	// Add all the DXIL libraries
	for (const Library& lib : m_libraries)
	{
		D3D12_STATE_SUBOBJECT libSubobject = {};
		libSubobject.Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY;
		libSubobject.pDesc = &lib.m_libDesc;

		subobjects[currentIndex++] = libSubobject;
	}

	// Add all the hit group declarations
	for (const HitGroup& group : m_hitGroups)
	{
		D3D12_STATE_SUBOBJECT hitGroup = {};
		hitGroup.Type = D3D12_STATE_SUBOBJECT_TYPE_HIT_GROUP;
		hitGroup.pDesc = &group.m_desc;

		subobjects[currentIndex++] = hitGroup;
	}

	// Add a subobject for the shader payload configuration
	D3D12_RAYTRACING_SHADER_CONFIG shaderDesc = {};
	shaderDesc.MaxPayloadSizeInBytes = m_maxPayLoadSizeInBytes;
	shaderDesc.MaxAttributeSizeInBytes = m_maxAttributeSizeInBytes;

	D3D12_STATE_SUBOBJECT shaderConfigObject = {};
	shaderConfigObject.Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG;
	shaderConfigObject.pDesc = &shaderDesc;

	subobjects[currentIndex++] = shaderConfigObject;
	// Build a list of all the symbols for ray generation, miss and hit groups
// Those shaders have to be associated with the payload definition
	std::vector<std::wstring> exportedSymbols = {};
	std::vector<LPCWSTR> exportedSymbolPointers = {};
	BuildShaderExportList(exportedSymbols);

	// Build an array of the string pointers
	exportedSymbolPointers.reserve(exportedSymbols.size());
	for (const auto& name : exportedSymbols)
	{
		exportedSymbolPointers.push_back(name.c_str());
	}
	const WCHAR** shaderExports = exportedSymbolPointers.data();

	// Add a subobject for the association between shaders and the payload
	D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION shaderPayloadAssociation = {};
	shaderPayloadAssociation.NumExports = static_cast<UINT>(exportedSymbols.size());
	shaderPayloadAssociation.pExports = shaderExports;

	// Associate the set of shaders with the payload defined in the previous subobject
	shaderPayloadAssociation.pSubobjectToAssociate = &subobjects[(currentIndex - 1)];

	// Create and store the payload association object
	D3D12_STATE_SUBOBJECT shaderPayloadAssociationObject = {};
	shaderPayloadAssociationObject.Type = D3D12_STATE_SUBOBJECT_TYPE_SUBOBJECT_TO_EXPORTS_ASSOCIATION;
	shaderPayloadAssociationObject.pDesc = &shaderPayloadAssociation;
	subobjects[currentIndex++] = shaderPayloadAssociationObject;

	// The root signature association requires two objects for each: one to declare the root
	// signature, and another to associate that root signature to a set of symbols
	for (RootSignatureAssociation& assoc : m_rootSignatureAssociations)
	{

		// Add a subobject to declare the root signature
		D3D12_STATE_SUBOBJECT rootSigObject = {};
		rootSigObject.Type = D3D12_STATE_SUBOBJECT_TYPE_LOCAL_ROOT_SIGNATURE;
		rootSigObject.pDesc = &assoc.m_rootSignature;

		subobjects[currentIndex++] = rootSigObject;

		// Add a subobject for the association between the exported shader symbols and the root
		// signature
		assoc.m_association.NumExports = static_cast<UINT>(assoc.m_symbolPointers.size());
		assoc.m_association.pExports = assoc.m_symbolPointers.data();
		assoc.m_association.pSubobjectToAssociate = &subobjects[(currentIndex - 1)];

		D3D12_STATE_SUBOBJECT rootSigAssociationObject = {};
		rootSigAssociationObject.Type = D3D12_STATE_SUBOBJECT_TYPE_SUBOBJECT_TO_EXPORTS_ASSOCIATION;
		rootSigAssociationObject.pDesc = &assoc.m_association;

		subobjects[currentIndex++] = rootSigAssociationObject;
	}

	// The pipeline construction always requires an empty global root signature
	D3D12_STATE_SUBOBJECT globalRootSig;
	globalRootSig.Type = D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_ROOT_SIGNATURE;
	ID3D12RootSignature* dgSig = m_dummyGlobalRootSignature;
	globalRootSig.pDesc = &dgSig;


	subobjects[currentIndex++] = globalRootSig;

	// The pipeline construction always requires an empty local root signature
	D3D12_STATE_SUBOBJECT dummyLocalRootSig;
	dummyLocalRootSig.Type = D3D12_STATE_SUBOBJECT_TYPE_LOCAL_ROOT_SIGNATURE;
	ID3D12RootSignature* dlSig = m_dummyLocalRootSignature;
	dummyLocalRootSig.pDesc = &dlSig;
	subobjects[currentIndex++] = dummyLocalRootSig;

	// Add a subobject for the ray tracing pipeline configuration
	D3D12_RAYTRACING_PIPELINE_CONFIG pipelineConfig = {};
	pipelineConfig.MaxTraceRecursionDepth = m_maxRecursionDepth;

	D3D12_STATE_SUBOBJECT pipelineConfigObject = {};
	pipelineConfigObject.Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG;
	pipelineConfigObject.pDesc = &pipelineConfig;

	subobjects[currentIndex++] = pipelineConfigObject;

	// Describe the ray tracing pipeline state object
	D3D12_STATE_OBJECT_DESC pipelineDesc = {};
	pipelineDesc.Type = D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE;
	pipelineDesc.NumSubobjects = currentIndex; // static_cast<UINT>(subobjects.size());
	pipelineDesc.pSubobjects = subobjects.data();

	ID3D12StateObject* rtStateObject = nullptr;

	// Create the state object
	HRESULT hr = m_device->CreateStateObject(&pipelineDesc, IID_PPV_ARGS(&rtStateObject));
	if (FAILED(hr))
	{
		ERROR_AND_DIE("Could not create the raytracing state object");
	}
	return rtStateObject;

}


ID3D12StateObject* RayTracingPipelineGenerator::GenerateWithGlobal()
{
	// The pipeline is made of a set of sub-objects, representing the DXIL libraries, hit group
// declarations, root signature associations, plus some configuration objects
	UINT64 subobjectCount =
		m_libraries.size() +                     // DXIL libraries
		m_hitGroups.size() +                     // Hit group declarations
		1 +                                      // Shader configuration
		1 +                                      // Shader payload
		2 * m_rootSignatureAssociations.size() + // Root signature declaration + association
		2 +                                      // Empty global and local root signatures
		1;                                       // Final pipeline subobject

	// Initialize a vector with the target object count. It is necessary to make the allocation before
	// adding subobjects as some subobjects reference other subobjects by pointer. Using push_back may
	// reallocate the array and invalidate those pointers.
	std::vector<D3D12_STATE_SUBOBJECT> subobjects(subobjectCount);

	UINT currentIndex = 0;

	// Add all the DXIL libraries
	for (const Library& lib : m_libraries)
	{
		D3D12_STATE_SUBOBJECT libSubobject = {};
		libSubobject.Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY;
		libSubobject.pDesc = &lib.m_libDesc;

		subobjects[currentIndex++] = libSubobject;
	}

	// Add all the hit group declarations
	for (const HitGroup& group : m_hitGroups)
	{
		D3D12_STATE_SUBOBJECT hitGroup = {};
		hitGroup.Type = D3D12_STATE_SUBOBJECT_TYPE_HIT_GROUP;
		hitGroup.pDesc = &group.m_desc;

		subobjects[currentIndex++] = hitGroup;
	}

	// Add a subobject for the shader payload configuration
	D3D12_RAYTRACING_SHADER_CONFIG shaderDesc = {};
	shaderDesc.MaxPayloadSizeInBytes = m_maxPayLoadSizeInBytes;
	shaderDesc.MaxAttributeSizeInBytes = m_maxAttributeSizeInBytes;

	D3D12_STATE_SUBOBJECT shaderConfigObject = {};
	shaderConfigObject.Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG;
	shaderConfigObject.pDesc = &shaderDesc;

	subobjects[currentIndex++] = shaderConfigObject;
	// Build a list of all the symbols for ray generation, miss and hit groups
// Those shaders have to be associated with the payload definition
	std::vector<std::wstring> exportedSymbols = {};
	std::vector<LPCWSTR> exportedSymbolPointers = {};
	BuildShaderExportList(exportedSymbols);

	// Build an array of the string pointers
	exportedSymbolPointers.reserve(exportedSymbols.size());
	for (const auto& name : exportedSymbols)
	{
		exportedSymbolPointers.push_back(name.c_str());
	}
	const WCHAR** shaderExports = exportedSymbolPointers.data();

	// Add a subobject for the association between shaders and the payload
	D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION shaderPayloadAssociation = {};
	shaderPayloadAssociation.NumExports = static_cast<UINT>(exportedSymbols.size());
	shaderPayloadAssociation.pExports = shaderExports;

	// Associate the set of shaders with the payload defined in the previous subobject
	shaderPayloadAssociation.pSubobjectToAssociate = &subobjects[(currentIndex - 1)];

	// Create and store the payload association object
	D3D12_STATE_SUBOBJECT shaderPayloadAssociationObject = {};
	shaderPayloadAssociationObject.Type = D3D12_STATE_SUBOBJECT_TYPE_SUBOBJECT_TO_EXPORTS_ASSOCIATION;
	shaderPayloadAssociationObject.pDesc = &shaderPayloadAssociation;
	subobjects[currentIndex++] = shaderPayloadAssociationObject;

	// The root signature association requires two objects for each: one to declare the root
	// signature, and another to associate that root signature to a set of symbols
	for (RootSignatureAssociation& assoc : m_rootSignatureAssociations)
	{

		// Add a subobject to declare the root signature
		D3D12_STATE_SUBOBJECT rootSigObject = {};
		rootSigObject.Type = D3D12_STATE_SUBOBJECT_TYPE_LOCAL_ROOT_SIGNATURE;
		rootSigObject.pDesc = &assoc.m_rootSignature;

		subobjects[currentIndex++] = rootSigObject;

		// Add a subobject for the association between the exported shader symbols and the root
		// signature
		assoc.m_association.NumExports = static_cast<UINT>(assoc.m_symbolPointers.size());
		assoc.m_association.pExports = assoc.m_symbolPointers.data();
		assoc.m_association.pSubobjectToAssociate = &subobjects[(currentIndex - 1)];

		D3D12_STATE_SUBOBJECT rootSigAssociationObject = {};
		rootSigAssociationObject.Type = D3D12_STATE_SUBOBJECT_TYPE_SUBOBJECT_TO_EXPORTS_ASSOCIATION;
		rootSigAssociationObject.pDesc = &assoc.m_association;

		subobjects[currentIndex++] = rootSigAssociationObject;
	}

	// The pipeline construction always requires an empty global root signature
	
	D3D12_STATE_SUBOBJECT globalRootSig;
	globalRootSig.Type = D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_ROOT_SIGNATURE;
	//ID3D12RootSignature* dgSig = m_dummyGlobalRootSignature;
	ID3D12RootSignature* dgSig = m_globalRootSignature;
	globalRootSig.pDesc = &dgSig;
	

	subobjects[currentIndex++] = globalRootSig;

	// The pipeline construction always requires an empty local root signature
	D3D12_STATE_SUBOBJECT dummyLocalRootSig;
	dummyLocalRootSig.Type = D3D12_STATE_SUBOBJECT_TYPE_LOCAL_ROOT_SIGNATURE;
	ID3D12RootSignature* dlSig = m_dummyLocalRootSignature;
	dummyLocalRootSig.pDesc = &dlSig;
	subobjects[currentIndex++] = dummyLocalRootSig;

	// Add a subobject for the ray tracing pipeline configuration
	D3D12_RAYTRACING_PIPELINE_CONFIG pipelineConfig = {};
	pipelineConfig.MaxTraceRecursionDepth = m_maxRecursionDepth;

	D3D12_STATE_SUBOBJECT pipelineConfigObject = {};
	pipelineConfigObject.Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG;
	pipelineConfigObject.pDesc = &pipelineConfig;

	subobjects[currentIndex++] = pipelineConfigObject;

	// Describe the ray tracing pipeline state object
	D3D12_STATE_OBJECT_DESC pipelineDesc = {};
	pipelineDesc.Type = D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE;
	pipelineDesc.NumSubobjects = currentIndex; // static_cast<UINT>(subobjects.size());
	pipelineDesc.pSubobjects = subobjects.data();

	ID3D12StateObject* rtStateObject = nullptr;

	// Create the state object
	HRESULT hr = m_device->CreateStateObject(&pipelineDesc, IID_PPV_ARGS(&rtStateObject));
	if (FAILED(hr))
	{
		ERROR_AND_DIE("Could not create the raytracing state object");
	}
	return rtStateObject;
}


void RayTracingPipelineGenerator::SetGlobalRootSig(ID3D12RootSignature* globalSig)
{
	m_globalRootSignature = globalSig;
}

void RayTracingPipelineGenerator::CreateDummyRootSignatures()
{
	// Creation of the global root signature
	D3D12_ROOT_SIGNATURE_DESC rootDesc = {};
	rootDesc.NumParameters = 0;
	rootDesc.pParameters = nullptr;
	// A global root signature is the default, hence this flag
	rootDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;

	HRESULT hr = 0;

	ID3DBlob* serializedRootSignature;
	ID3DBlob* error;

	// Create the empty global root signature
	hr = D3D12SerializeRootSignature(&rootDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		&serializedRootSignature, &error);
	if (FAILED(hr))
	{
		ERROR_AND_DIE("Could not serialize the global root signature");
	}
	hr = m_device->CreateRootSignature(0, serializedRootSignature->GetBufferPointer(),
		serializedRootSignature->GetBufferSize(),
		IID_PPV_ARGS(&m_dummyGlobalRootSignature));

	serializedRootSignature->Release();
	if (FAILED(hr))
	{
		ERROR_AND_DIE("Could not create the global root signature");
	}
	else
	{
		NAME_D3D12_OBJECT(m_dummyGlobalRootSignature, L"Dummy Global Root Sig");
	}

	// Create the local root signature, reusing the same descriptor but altering the creation flag
	rootDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE;
	hr = D3D12SerializeRootSignature(&rootDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		&serializedRootSignature, &error);
	if (FAILED(hr))
	{
		ERROR_AND_DIE("Could not serialize the local root signature");
	}
	hr = m_device->CreateRootSignature(0, serializedRootSignature->GetBufferPointer(),
		serializedRootSignature->GetBufferSize(),
		IID_PPV_ARGS(&m_dummyLocalRootSignature));

	serializedRootSignature->Release();
	if (FAILED(hr))
	{
		ERROR_AND_DIE("Could not create the local root signature");
	}
	else
	{
		NAME_D3D12_OBJECT(m_dummyLocalRootSignature, L"Dummy Local Root Sig");
	}

}

// void RayTracingPipelineGenerator::ModifyContentRootSignature()
// {
// 
// }

//--------------------------------------------------------------------------------------------------
//
// Build a list containing the export symbols for the ray generation shaders, miss shaders, and
// hit group names
void RayTracingPipelineGenerator::BuildShaderExportList(std::vector<std::wstring>& exportedSymbols)
{
	// Get all names from libraries
	// Get names associated to hit groups
	// Return list of libraries+hit group names - shaders in hit groups

	std::unordered_set<std::wstring> exports;

	// Add all the symbols exported by the libraries
	for (const Library& lib : m_libraries)
	{
		for (const auto& exportName : lib.m_exportedSymbols)
		{
#ifdef _DEBUG
			// Sanity check in debug mode: check that no name is exported more than once
			if (exports.find(exportName) != exports.end())
			{
				ERROR_AND_DIE("Multiple definition of a symbol in the imported DXIL libraries");
			}
#endif
			exports.insert(exportName);
		}
	}

#ifdef _DEBUG
	// Sanity check in debug mode: verify that the hit groups do not reference an unknown shader name
	std::unordered_set<std::wstring> all_exports = exports;

	for (const auto& hitGroup : m_hitGroups)
	{
		if (!hitGroup.m_anyHitSymbol.empty() && exports.find(hitGroup.m_anyHitSymbol) == exports.end())
		{
			ERROR_AND_DIE("Any hit symbol not found in the imported DXIL libraries");
		}

		if (!hitGroup.m_closestHitSymbol.empty() &&
			exports.find(hitGroup.m_closestHitSymbol) == exports.end())
		{
			ERROR_AND_DIE("Closest hit symbol not found in the imported DXIL libraries");
		}

		if (!hitGroup.m_intersectionSymbol.empty() &&
			exports.find(hitGroup.m_intersectionSymbol) == exports.end())
		{
			ERROR_AND_DIE("Intersection symbol not found in the imported DXIL libraries");
		}

		all_exports.insert(hitGroup.m_hitGroupName);
	}

	// Sanity check in debug mode: verify that the root signature associations do not reference an
	// unknown shader or hit group name
	for (const auto& assoc : m_rootSignatureAssociations)
	{
		for (const auto& symb : assoc.m_symbols)
		{
			if (!symb.empty() && all_exports.find(symb) == all_exports.end())
			{
				ERROR_AND_DIE("Root association symbol not found in the imported DXIL libraries and hit group names");
			}
		}
	}
#endif

	// Go through all hit groups and remove the symbols corresponding to intersection, any hit and
	// closest hit shaders from the symbol set
	for (const auto& hitGroup : m_hitGroups)
	{
		if (!hitGroup.m_anyHitSymbol.empty())
		{
			exports.erase(hitGroup.m_anyHitSymbol);
		}
		if (!hitGroup.m_closestHitSymbol.empty())
		{
			exports.erase(hitGroup.m_closestHitSymbol);
		}
		if (!hitGroup.m_intersectionSymbol.empty())
		{
			exports.erase(hitGroup.m_intersectionSymbol);
		}
		exports.insert(hitGroup.m_hitGroupName);
	}

	// Finally build a vector containing ray generation and miss shaders, plus the hit group names
	for (const auto& name : exports)
	{
		exportedSymbols.push_back(name);
	}

}

//--------------------------------------------------------------------------------------------------
//
// Store data related to a DXIL library: the library itself, the exported symbols, and the
// associated descriptors
RayTracingPipelineGenerator::Library::Library(IDxcBlob* dxil, const std::vector<std::wstring>& exportedSymbols)
	: m_dxil(dxil), m_exportedSymbols(exportedSymbols), m_exports(exportedSymbols.size())
{
	// Create one export descriptor per symbol
	for (size_t i = 0; i < m_exportedSymbols.size(); i++)
	{
		m_exports[i] = {};
		m_exports[i].Name = m_exportedSymbols[i].c_str();
		m_exports[i].ExportToRename = nullptr;
		m_exports[i].Flags = D3D12_EXPORT_FLAG_NONE;
	}

	// Create a library descriptor combining the DXIL code and the export names
	m_libDesc.DXILLibrary.BytecodeLength = dxil->GetBufferSize();
	m_libDesc.DXILLibrary.pShaderBytecode = dxil->GetBufferPointer();
	m_libDesc.NumExports = static_cast<UINT>(m_exportedSymbols.size());
	m_libDesc.pExports = m_exports.data();
}
//--------------------------------------------------------------------------------------------------
//
// This copy constructor has to be defined so that the export descriptors are set correctly. Using
// the default constructor would copy the string pointers of the symbols into the descriptors, which
// would cause issues when the original Library object gets out of scope
RayTracingPipelineGenerator::Library::Library(const Library& source) 
	: Library(source.m_dxil, source.m_exportedSymbols)
{

}











//--------------------------------------------------------------------------------------------------
//
// Create a hit group descriptor from the input hit group name and shader symbols
RayTracingPipelineGenerator::HitGroup::HitGroup(std::wstring hitGroupName,
	std::wstring closestHitSymbol,
	std::wstring anyHitSymbol /*= L""*/,
	std::wstring intersectionSymbol /*= L""*/)
	: m_hitGroupName(std::move(hitGroupName)), m_closestHitSymbol(std::move(closestHitSymbol)),
	m_anyHitSymbol(std::move(anyHitSymbol)), m_intersectionSymbol(std::move(intersectionSymbol))
{
	// Indicate which shader program is used for closest hit, leave the other
	// ones undefined (default behavior), export the name of the group
	m_desc.HitGroupExport = m_hitGroupName.c_str();
	m_desc.ClosestHitShaderImport = m_closestHitSymbol.empty() ? nullptr : m_closestHitSymbol.c_str();
	m_desc.AnyHitShaderImport = m_anyHitSymbol.empty() ? nullptr : m_anyHitSymbol.c_str();
	m_desc.IntersectionShaderImport =
		m_intersectionSymbol.empty() ? nullptr : m_intersectionSymbol.c_str();
}

//--------------------------------------------------------------------------------------------------
//
// This copy constructor has to be defined so that the export descriptors are set correctly. Using
// the default constructor would copy the string pointers of the symbols into the descriptors, which
// would cause issues when the original HitGroup object gets out of scope
RayTracingPipelineGenerator::HitGroup::HitGroup(const HitGroup& source)
	: HitGroup(source.m_hitGroupName, source.m_closestHitSymbol, source.m_anyHitSymbol,
		source.m_intersectionSymbol)
{

}

//--------------------------------------------------------------------------------------------------
//
// Store the association between a set of symbols and a root signature. The associated descriptors
// will be built when compiling the pipeline. We store the symbol pointers directly so that they can
// be used without processing during compilation.
RayTracingPipelineGenerator::RootSignatureAssociation::RootSignatureAssociation(ID3D12RootSignature* rootSignature, const std::vector<std::wstring>& symbols)
	: m_rootSignature(rootSignature), m_symbols(symbols), m_symbolPointers(symbols.size())
{
	for (size_t i = 0; i < m_symbols.size(); i++)
	{
		m_symbolPointers[i] = m_symbols[i].c_str();
	}
	m_rootSignaturePointer = m_rootSignature;
}

RayTracingPipelineGenerator::RootSignatureAssociation::RootSignatureAssociation(const RootSignatureAssociation& source)
	: RootSignatureAssociation(source.m_rootSignature, source.m_symbols)
{
}

//--------------------------------------------------------------------------------------------------
//
// Add a ray generation program by name, with its list of data pointers or values according to
// the layout of its root signature
void ShaderBindingTableGenerator::AddRayGenerationProgram(const std::wstring& entryPoint,
	const std::vector<void*>& inputData)
{
	m_rayGen.emplace_back(SBTEntry(entryPoint, inputData));
}


//--------------------------------------------------------------------------------------------------
//
// Add a miss program by name, with its list of data pointers or values according to
// the layout of its root signature
void ShaderBindingTableGenerator::AddMissProgram(const std::wstring& entryPoint,
	const std::vector<void*>& inputData)
{
	m_miss.emplace_back(SBTEntry(entryPoint, inputData));
}

//--------------------------------------------------------------------------------------------------
//
// Add a hit group by name, with its list of data pointers or values according to
// the layout of its root signature
void ShaderBindingTableGenerator::AddHitGroup(const std::wstring& entryPoint,
	const std::vector<void*>& inputData)
{
	m_hitGroup.emplace_back(SBTEntry(entryPoint, inputData));
}
//--------------------------------------------------------------------------------------------------
//
// Compute the size of the SBT based on the set of programs and hit groups it contains
uint32_t ShaderBindingTableGenerator::ComputeSBTSize()
{
	// Size of a program identifier
	m_progIdSize = D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT;
	// Compute the entry size of each program type depending on the maximum number of parameters in
	// each category
	m_rayGenEntrySize = GetEntrySize(m_rayGen);
	m_missEntrySize = GetEntrySize(m_miss);
	m_hitGroupEntrySize = GetEntrySize(m_hitGroup);

	// The total SBT size is the sum of the entries for ray generation, miss and hit groups, aligned
	// on 256 bytes
	uint32_t sbtSize = ROUND_UP(m_rayGenEntrySize * static_cast<UINT>(m_rayGen.size()) +
		m_missEntrySize * static_cast<UINT>(m_miss.size()) +
		m_hitGroupEntrySize * static_cast<UINT>(m_hitGroup.size()),
		256);
	return sbtSize;
}

//--------------------------------------------------------------------------------------------------
//
// Build the SBT and store it into sbtBuffer, which has to be pre-allocated on the upload heap.
// Access to the raytracing pipeline object is required to fetch program identifiers using their
// names
void ShaderBindingTableGenerator::Generate(ID3D12Resource* sbtBuffer,
	ID3D12StateObjectProperties* raytracingPipeline)
{
	// Map the SBT
	uint8_t* pData;
	HRESULT hr = sbtBuffer->Map(0, nullptr, reinterpret_cast<void**>(&pData));
	if (FAILED(hr))
	{
		ERROR_AND_DIE("Could not map the shader binding table");
	}
	// Copy the shader identifiers followed by their resource pointers or root constants: first the
	// ray generation, then the miss shaders, and finally the set of hit groups
	uint32_t offset = 0;

	offset = CopyShaderData(raytracingPipeline, pData, m_rayGen, m_rayGenEntrySize);
	pData += offset;

	offset = CopyShaderData(raytracingPipeline, pData, m_miss, m_missEntrySize);
	pData += offset;

	offset = CopyShaderData(raytracingPipeline, pData, m_hitGroup, m_hitGroupEntrySize);

	// Unmap the SBT
	sbtBuffer->Unmap(0, nullptr);
}
//--------------------------------------------------------------------------------------------------
//
// Reset the sets of programs and hit groups
void ShaderBindingTableGenerator::Reset()
{
	m_rayGen.clear();
	m_miss.clear();
	m_hitGroup.clear();

	m_rayGenEntrySize = 0;
	m_missEntrySize = 0;
	m_hitGroupEntrySize = 0;
	m_progIdSize = 0;
}
//--------------------------------------------------------------------------------------------------
// The following getters are used to simplify the call to DispatchRays where the offsets of the
// shader programs must be exactly following the SBT layout

//--------------------------------------------------------------------------------------------------
//
// Get the size in bytes of the SBT section dedicated to ray generation programs
UINT ShaderBindingTableGenerator::GetRayGenSectionSize() const
{
	return m_rayGenEntrySize * static_cast<UINT>(m_rayGen.size());
}
//--------------------------------------------------------------------------------------------------
//
// Get the size in bytes of one ray generation program entry in the SBT
UINT ShaderBindingTableGenerator::GetRayGenEntrySize() const
{
	return m_rayGenEntrySize;
}
//--------------------------------------------------------------------------------------------------
//
// Get the size in bytes of the SBT section dedicated to miss programs
UINT ShaderBindingTableGenerator::GetMissSectionSize() const
{
	return m_missEntrySize * static_cast<UINT>(m_miss.size());
}

//--------------------------------------------------------------------------------------------------
//
// Get the size in bytes of one miss program entry in the SBT
UINT ShaderBindingTableGenerator::GetMissEntrySize()
{
	return m_missEntrySize;
}
//--------------------------------------------------------------------------------------------------
//
// Get the size in bytes of the SBT section dedicated to hit groups
UINT ShaderBindingTableGenerator::GetHitGroupSectionSize() const
{
	return m_hitGroupEntrySize * static_cast<UINT>(m_hitGroup.size());
}
//--------------------------------------------------------------------------------------------------
//
// Get the size in bytes of one hit group entry in the SBT
UINT ShaderBindingTableGenerator::GetHitGroupEntrySize() const
{
	return m_hitGroupEntrySize;
}

//--------------------------------------------------------------------------------------------------
//
// For each entry, copy the shader identifier followed by its resource pointers and/or root
// constants in outputData, with a stride in bytes of entrySize, and returns the size in bytes
// actually written to outputData.
uint32_t ShaderBindingTableGenerator::CopyShaderData(
	ID3D12StateObjectProperties* raytracingPipeline, uint8_t* outputData,
	const std::vector<SBTEntry>& shaders, uint32_t entrySize)
{
	uint8_t* pData = outputData;
	for (const auto& shader : shaders)
	{
		// Get the shader identifier, and check whether that identifier is known
		void* id = raytracingPipeline->GetShaderIdentifier(shader.m_entryPoint.c_str());
		if (!id)
		{
			//std::wstring errMsg(std::wstring(L"Unknown shader identifier used in the SBT: ") +
			//	shader.m_entryPoint);
			//throw std::logic_error(std::string(errMsg.begin(), errMsg.end()));
			//std::wstring errMsg(shader.m_entryPoint);
			std::string debugText = "Unknown shader identifier used in the SBT";
			ERROR_AND_DIE(debugText);
		}
		// Copy the shader identifier
		memcpy(pData, id, m_progIdSize);
		// Copy all its resources pointers or values in bulk
		memcpy(pData + m_progIdSize, shader.m_inputData.data(), shader.m_inputData.size() * 8);

		pData += entrySize;
	}
	// Return the number of bytes actually written to the output buffer
	return static_cast<uint32_t>(shaders.size()) * entrySize;
}

//--------------------------------------------------------------------------------------------------
//
// Compute the size of the SBT entries for a set of entries, which is determined by the maximum
// number of parameters of their root signature
uint32_t ShaderBindingTableGenerator::GetEntrySize(const std::vector<SBTEntry>& entries)
{
	// Find the maximum number of parameters used by a single entry
	size_t maxArgs = 0;
	for (const auto& shader : entries)
	{
		maxArgs = max(maxArgs, shader.m_inputData.size());
	}
	// A SBT entry is made of a program ID and a set of parameters, taking 8 bytes each. Those
	// parameters can either be 8-bytes pointers, or 4-bytes constants
	uint32_t entrySize = m_progIdSize + 8 * static_cast<uint32_t>(maxArgs);

	// The entries of the shader binding table must be 16-bytes-aligned
	entrySize = ROUND_UP(entrySize, D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT);

	return entrySize;
}


ShaderBindingTableGenerator::SBTEntry::SBTEntry(std::wstring entryPoint, std::vector<void*> inputData)
	: m_entryPoint(std::move(entryPoint)), m_inputData(std::move(inputData))
{

}



void RootSignatureGenerator::AddHeapRangesParameter(std::vector<std::tuple<UINT,
	/* BaseShaderRegister, */ UINT, /* NumDescriptors */ UINT, 
	/* RegisterSpace */ D3D12_DESCRIPTOR_RANGE_TYPE, /* RangeType */ UINT 
	/* OffsetInDescriptorsFromTableStart */ >> ranges)
{
	// Build and store the set of descriptors for the ranges
	std::vector<D3D12_DESCRIPTOR_RANGE> rangeStorage;
	for (const auto& input : ranges)
	{
		D3D12_DESCRIPTOR_RANGE r = {};
		r.BaseShaderRegister = std::get<RSC_BASE_SHADER_REGISTER>(input);
		r.NumDescriptors = std::get<RSC_NUM_DESCRIPTORS>(input);
		r.RegisterSpace = std::get<RSC_REGISTER_SPACE>(input);
		r.RangeType = std::get<RSC_RANGE_TYPE>(input);
		r.OffsetInDescriptorsFromTableStart =
			std::get<RSC_OFFSET_IN_DESCRIPTORS_FROM_TABLE_START>(input);
		rangeStorage.push_back(r);
	}

	// Add those ranges to the heap parameters
	AddHeapRangesParameter(rangeStorage);
}

void RootSignatureGenerator::AddHeapRangesParameter(const std::vector<D3D12_DESCRIPTOR_RANGE>& ranges)
{
	m_ranges.push_back(ranges);

	// A set of ranges on the heap is a descriptor table parameter
	D3D12_ROOT_PARAMETER param = {};
	param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	param.DescriptorTable.NumDescriptorRanges = static_cast<UINT>(ranges.size());
	// The range pointer is kept null here, and will be resolved when generating the root signature
	// (see explanation of m_rangeLocations below)
	param.DescriptorTable.pDescriptorRanges = nullptr;

	// All parameters (heap ranges and root parameters) are added to the same parameter list to
	// preserve order
	m_parameters.push_back(param);

	// The descriptor table descriptor ranges require a pointer to the descriptor ranges. Since new
	// ranges can be dynamically added in the vector, we separately store the index of the range set.
	// The actual address will be solved when generating the actual root signature
	m_rangeLocations.push_back(static_cast<UINT>(m_ranges.size() - 1));
}

void RootSignatureGenerator::AddRootParameter(D3D12_ROOT_PARAMETER_TYPE type, UINT shaderRegister /*= 0*/, UINT registerSpace /*= 0*/, UINT numRootConstants /*= 1*/)
{
	D3D12_ROOT_PARAMETER param = {};
	param.ParameterType = type;
	// The descriptor is an union, so specific values need to be set in case the parameter is a
	// constant instead of a buffer.
	if (type == D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS)
	{
		param.Constants.Num32BitValues = numRootConstants;
		param.Constants.RegisterSpace = registerSpace;
		param.Constants.ShaderRegister = shaderRegister;
	}
	else
	{
		param.Descriptor.RegisterSpace = registerSpace;
		param.Descriptor.ShaderRegister = shaderRegister;
	}

	// We default the visibility to all shaders
	param.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	// Add the root parameter to the set of parameters,
	m_parameters.push_back(param);
	// and indicate that there will be no range
	// location to indicate since this parameter is not part of the heap
	m_rangeLocations.push_back(~0u);
}

#endif // ENABLE_D3D12