#pragma once
#include "Engine/Core/EngineCommon.hpp"
#ifdef ENABLE_D3D12
#include "Engine/Renderer/D12CommonHeader.hpp"
#include "Engine/Math/Mat44.hpp"

// Helper to compute aligned buffer sizes
#ifndef ROUND_UP
#define ROUND_UP(v, powerOf2Alignment) (((v) + (powerOf2Alignment)-1) & ~((powerOf2Alignment)-1))
#endif

/// Helper class to generate bottom-level acceleration structures for ray-tracing
class BottomLevelASGenerator
{
public:
	/// Add a vertex buffer in GPU memory into the acceleration structure. The
	/// vertices are supposed to be represented by 3 float32 value. Indices are
	/// implicit.
	void AddVertexBuffer(ID3D12Resource* vertexBuffer, /// Buffer containing the vertex coordinates,
													   /// possibly interleaved with other vertex data
		UINT64 vertexOffsetInBytes,   /// Offset of the first vertex in the vertex
									  /// buffer
		uint32_t vertexCount,         /// Number of vertices to consider
									  /// in the buffer
		UINT vertexSizeInBytes,       /// Size of a vertex including all
									  /// its other data, used to stride
									  /// in the buffer
		ID3D12Resource* transformBuffer, /// Buffer containing a 4x4 transform
										 /// matrix in GPU memory, to be applied
										 /// to the vertices. This buffer cannot
										 /// be nullptr
		UINT64 transformOffsetInBytes,   /// Offset of the transform matrix in the
										 /// transform buffer
		bool isOpaque = true /// If true, the geometry is considered opaque,
							 /// optimizing the search for a closest hit
	);

	/// Add a vertex buffer along with its index buffer in GPU memory into the acceleration structure.
	/// The vertices are supposed to be represented by 3 float32 value, and the indices are 32-bit
	/// unsigned ints
	void AddVertexBuffer(ID3D12Resource* vertexBuffer, /// Buffer containing the vertex coordinates,
													   /// possibly interleaved with other vertex data
		UINT64 vertexOffsetInBytes,   /// Offset of the first vertex in the vertex
									  /// buffer
		uint32_t vertexCount,         /// Number of vertices to consider
									  /// in the buffer
		UINT vertexSizeInBytes,       /// Size of a vertex including
									  /// all its other data,
									  /// used to stride in the buffer
		ID3D12Resource* indexBuffer,  /// Buffer containing the vertex indices
									  /// describing the triangles
		UINT64 indexOffsetInBytes,    /// Offset of the first index in
									  /// the index buffer
		uint32_t indexCount,          /// Number of indices to consider in the buffer
		ID3D12Resource* transformBuffer, /// Buffer containing a 4x4 transform
										 /// matrix in GPU memory, to be applied
										 /// to the vertices. This buffer cannot
										 /// be nullptr
		UINT64 transformOffsetInBytes,   /// Offset of the transform matrix in the
										 /// transform buffer
		bool isOpaque = true /// If true, the geometry is considered opaque,
							 /// optimizing the search for a closest hit
	);

	/// Compute the size of the scratch space required to build the acceleration structure, as well as
	/// the size of the resulting structure. The allocation of the buffers is then left to the
	/// application
	void ComputeASBufferSizes(
		id3d12Device* device, /// Device on which the build will be performed
		bool allowUpdate,           /// If true, the resulting acceleration structure will
									/// allow iterative updates
		UINT64* scratchSizeInBytes, /// Required scratch memory on the GPU to
									/// build the acceleration structure
		UINT64* resultSizeInBytes   /// Required GPU memory to store the
									/// acceleration structure
	);

	/// Enqueue the construction of the acceleration structure on a command list, using
	/// application-provided buffers and possibly a pointer to the previous acceleration structure in
	/// case of iterative updates. Note that the update can be done in place: the result and
	/// previousResult pointers can be the same.
	void Generate(
		id3d12GraphicsCommandList* commandList, /// Command list on which the build will be enqueued
		ID3D12Resource* scratchBuffer, /// Scratch buffer used by the builder to
									   /// store temporary data
		ID3D12Resource* resultBuffer,  /// Result buffer storing the acceleration structure
		bool updateOnly = false,       /// If true, simply refit the existing acceleration structure
		ID3D12Resource* previousResult = nullptr /// Optional previous acceleration structure, used
												 /// if an iterative update is requested
	);

private:
	/// Vertex buffer descriptors used to generate the AS
	std::vector<D3D12_RAYTRACING_GEOMETRY_DESC> m_vertexBuffers = {};

	/// Amount of temporary memory required by the builder
	UINT64 m_scratchSizeInBytes = 0;

	/// Amount of memory required to store the AS
	UINT64 m_resultSizeInBytes = 0;

	/// Flags for the builder, specifying whether to allow iterative updates, or
	/// when to perform an update
	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS m_flags;
};


class TopLevelASGenerator
{
public:

/// Add an instance to the top-level acceleration structure. The instance is
/// represented by a bottom-level AS, a transform, an instance ID and the
/// index of the hit group indicating which shaders are executed upon hitting
/// any geometry within the instance
	void AddInstance(ID3D12Resource* bottomLevelAS, /// Bottom-level acceleration structure containing the
												   /// actual geometric data of the instance
			const DirectX::XMMATRIX& transform, /// Transform matrix to apply to the instance,
												/// allowing the same bottom-level AS to be used
												/// at several world-space positions
			UINT instanceID,   /// Instance ID, which can be used in the shaders to
							   /// identify this specific instance
			UINT hitGroupIndex /// Hit group index, corresponding the the index of the
							   /// hit group in the Shader Binding Table that will be
							   /// invocate upon hitting the geometry
		);

	/// Compute the size of the scratch space required to build the acceleration
/// structure, as well as the size of the resulting structure. The allocation
/// of the buffers is then left to the application
	void ComputeASBufferSizes(
		id3d12Device* device, /// Device on which the build will be performed
		bool allowUpdate,              /// If true, the resulting acceleration structure will
									   /// allow iterative updates
		UINT64* scratchSizeInBytes,    /// Required scratch memory on the GPU to
									   /// build the acceleration structure
		UINT64* resultSizeInBytes,     /// Required GPU memory to store the
									   /// acceleration structure
		UINT64* descriptorsSizeInBytes /// Required GPU memory to store instance
									   /// descriptors, containing the matrices,
									   /// indices etc.
	);

/// Enqueue the construction of the acceleration structure on a command list,
/// using application-provided buffers and possibly a pointer to the previous
/// acceleration structure in case of iterative updates. Note that the update
/// can be done in place: the result and previousResult pointers can be the
/// same.
	void Generate(
		id3d12GraphicsCommandList* commandList, /// Command list on which the build will be enqueued
		ID3D12Resource* scratchBuffer,     /// Scratch buffer used by the builder to
										   /// store temporary data
		ID3D12Resource* resultBuffer,      /// Result buffer storing the acceleration structure
		ID3D12Resource* descriptorsBuffer, /// Auxiliary result buffer containing the instance
										   /// descriptors, has to be in upload heap
		bool updateOnly = false, /// If true, simply refit the existing acceleration structure
		ID3D12Resource* previousResult = nullptr /// Optional previous acceleration structure, used
												 /// if an iterative update is requested
	);

private:
	/// Helper struct storing the instance data
	struct Instance
	{
		//Instance(ID3D12Resource* blAS, const Mat44& tr, UINT iID, UINT hgId);
		Instance(ID3D12Resource* blAS, const DirectX::XMMATRIX& tr, UINT iID, UINT hgId);
		/// Bottom-level AS
		ID3D12Resource* bottomLevelAS;
		/// Transform matrix
		//const Mat44& m_transform;
		const DirectX::XMMATRIX& transform;
		/// Instance ID visible in the shader
		UINT instanceID;
		/// Hit group index used to fetch the shaders from the SBT
		UINT hitGroupIndex;
	};

	/// Construction flags, indicating whether the AS supports iterative updates
	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS m_flags;
	/// Instances contained in the top-level AS
	std::vector<Instance> m_instances;

	/// Size of the temporary memory used by the TLAS builder
	UINT64 m_scratchSizeInBytes;
	/// Size of the buffer containing the instance descriptors
	UINT64 m_instanceDescsSizeInBytes;
	/// Size of the buffer containing the TLAS
	UINT64 m_resultSizeInBytes;
};

/// Helper class to create ray-tracing pipelines
class RayTracingPipelineGenerator 
{
public:
	/// The pipeline helper requires access to the device, as well as the
	/// ray-tracing device prior to Windows 10 RS5.

	RayTracingPipelineGenerator(id3d12Device* device);

	/// Add a DXIL library to the pipeline. Note that this library has to be
	/// compiled with DXC, using a lib_6_3 target. The exported symbols must correspond exactly to the
	/// names of the shaders declared in the library, although unused ones can be omitted.
	void AddLibrary(IDxcBlob* dxilLibrary, const std::vector<std::wstring>& symbolExports);


	/// In DXR the hit-related shaders are grouped into hit groups. Such shaders are:
	/// - The intersection shader, which can be used to intersect custom geometry, and is called upon
	///   hitting the bounding box the the object. A default one exists to intersect triangles
	/// - The any hit shader, called on each intersection, which can be used to perform early
	///   alpha-testing and allow the ray to continue if needed. Default is a pass-through.
	/// - The closest hit shader, invoked on the hit point closest to the ray start.
	/// The shaders in a hit group share the same root signature, and are only referred to by the
	/// hit group name in other places of the program.
	void AddHitGroup(const std::wstring& hitGroupName, const std::wstring& closestHitSymbol,
		const std::wstring& anyHitSymbol = L"",
		const std::wstring& intersectionSymbol = L"");

	/// The shaders and hit groups may have various root signatures. This call associates a root
	/// signature to one or more symbols. All imported symbols must be associated to one root
	/// signature.
	void AddRootSignatureAssociation(ID3D12RootSignature* rootSignature,
		const std::vector<std::wstring>& symbols);

	/// The payload is the way hit or miss shaders can exchange data with the shader that called
	/// TraceRay. When several ray types are used (e.g. primary and shadow rays), this value must be
	/// the largest possible payload size. Note that to optimize performance, this size must be kept
	/// as low as possible.
	void SetMaxPayloadSize(UINT sizeInBytes);

	/// When hitting geometry, a number of surface attributes can be generated by the inter-sector.
	/// Using the built-in triangle inter-sector the attributes are the barycentric coordinates, with a
	/// size 2*sizeof(float).
	void SetMaxAttributeSize(UINT sizeInBytes);

	/// Upon hitting a surface, a closest hit shader can issue a new TraceRay call. This parameter
	/// indicates the maximum level of recursion. Note that this depth should be kept as low as
	/// possible, typically 2, to allow hit shaders to trace shadow rays. Recursive ray tracing
	/// algorithms must be flattened to a loop in the ray generation program for best performance.
	void SetMaxRecursionDepth(UINT maxDepth);


	/// Compiles the ray-tracing state object
	ID3D12StateObject* Generate();

	/// Compiles the ray-tracing state object With Global root signature
	ID3D12StateObject* GenerateWithGlobal();

	void SetGlobalRootSig(ID3D12RootSignature* globalSig);
private:
	/// Storage for DXIL libraries and their exported symbols
	struct Library
	{
		Library(IDxcBlob* dxil, const std::vector<std::wstring>& exportedSymbols);

		Library(const Library& source);

		IDxcBlob* m_dxil;
		const std::vector<std::wstring> m_exportedSymbols;

		std::vector<D3D12_EXPORT_DESC> m_exports;
		D3D12_DXIL_LIBRARY_DESC m_libDesc;
	};

	/// Storage for the hit groups, binding the hit group name with the underlying intersection, any
	/// hit and closest hit symbols
	struct HitGroup
	{
		HitGroup(std::wstring hitGroupName, std::wstring closestHitSymbol,
			std::wstring anyHitSymbol = L"", std::wstring intersectionSymbol = L"");

		HitGroup(const HitGroup& source);

		std::wstring m_hitGroupName;
		std::wstring m_closestHitSymbol;
		std::wstring m_anyHitSymbol;
		std::wstring m_intersectionSymbol;
		D3D12_HIT_GROUP_DESC m_desc = {};
	};

	/// Storage for the association between shaders and root signatures
	struct RootSignatureAssociation
	{
		RootSignatureAssociation(ID3D12RootSignature* rootSignature,
			const std::vector<std::wstring>& symbols);

		RootSignatureAssociation(const RootSignatureAssociation& source);

		ID3D12RootSignature* m_rootSignature;
		ID3D12RootSignature* m_rootSignaturePointer;
		std::vector<std::wstring> m_symbols;
		std::vector<LPCWSTR> m_symbolPointers;
		D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION m_association = {};
	};

	/// The pipeline creation requires having at least one empty global and local root signatures, so
	/// we systematically create both
	void CreateDummyRootSignatures();
	//void ModifyContentRootSignature();
	/// Build a list containing the export symbols for the ray generation shaders, miss shaders, and
	/// hit group names
	void BuildShaderExportList(std::vector<std::wstring>& exportedSymbols);

	std::vector<Library> m_libraries = {};
	std::vector<HitGroup> m_hitGroups = {};
	std::vector<RootSignatureAssociation> m_rootSignatureAssociations = {};

	UINT m_maxPayLoadSizeInBytes = 0;
	/// Attribute size, initialized to 2 for the barycentric coordinates used by the built-in triangle
	/// intersection shader
	UINT m_maxAttributeSizeInBytes = 2 * sizeof(float);
	/// Maximum recursion depth, initialized to 1 to at least allow tracing primary rays
	UINT m_maxRecursionDepth = 1;

	id3d12Device* m_device = nullptr;
	ID3D12RootSignature* m_dummyLocalRootSignature = nullptr;
	ID3D12RootSignature* m_dummyGlobalRootSignature = nullptr;

	ID3D12RootSignature* m_globalRootSignature = nullptr;
};

/// Helper class to create and maintain a Shader Binding Table
class ShaderBindingTableGenerator 
{
public:
	/// Add a ray generation program by name, with its list of data pointers or values according to
	/// the layout of its root signature
	void AddRayGenerationProgram(const std::wstring& entryPoint, const std::vector<void*>& inputData);

	/// Add a miss program by name, with its list of data pointers or values according to
	/// the layout of its root signature
	void AddMissProgram(const std::wstring& entryPoint, const std::vector<void*>& inputData);

	/// Add a hit group by name, with its list of data pointers or values according to
	/// the layout of its root signature
	void AddHitGroup(const std::wstring& entryPoint, const std::vector<void*>& inputData);

	/// Compute the size of the SBT based on the set of programs and hit groups it contains
	uint32_t ComputeSBTSize();

	/// Build the SBT and store it into sbtBuffer, which has to be pre-allocated on the upload heap.
	/// Access to the raytracing pipeline object is required to fetch program identifiers using their
	/// names
	void Generate(ID3D12Resource* sbtBuffer, ID3D12StateObjectProperties* raytracingPipeline);

	/// Reset the sets of programs and hit groups
	void Reset();

	/// The following getters are used to simplify the call to DispatchRays where the offsets of the
	/// shader programs must be exactly following the SBT layout
	/// Get the size in bytes of the SBT section dedicated to ray generation programs
	UINT GetRayGenSectionSize() const;
	/// Get the size in bytes of one ray generation program entry in the SBT
	UINT GetRayGenEntrySize() const;

	/// Get the size in bytes of the SBT section dedicated to miss programs
	UINT GetMissSectionSize() const;
	/// Get the size in bytes of one miss program entry in the SBT
	UINT GetMissEntrySize();

	/// Get the size in bytes of the SBT section dedicated to hit groups
	UINT GetHitGroupSectionSize() const;
	/// Get the size in bytes of hit group entry in the SBT
	UINT GetHitGroupEntrySize() const;

private:
	/// Wrapper for SBT entries, each consisting of the name of the program and a list of values,
	/// which can be either pointers or raw 32-bit constants
	struct SBTEntry
	{
		SBTEntry(std::wstring entryPoint, std::vector<void*> inputData);

		const std::wstring m_entryPoint;
		const std::vector<void*> m_inputData;
	};
	/// For each entry, copy the shader identifier followed by its resource pointers and/or root
	/// constants in outputData, with a stride in bytes of entrySize, and returns the size in bytes
	/// actually written to outputData.
	uint32_t CopyShaderData(ID3D12StateObjectProperties* raytracingPipeline,
		uint8_t* outputData, const std::vector<SBTEntry>& shaders,
		uint32_t entrySize);

	/// Compute the size of the SBT entries for a set of entries, which is determined by the maximum
	/// number of parameters of their root signature
	uint32_t GetEntrySize(const std::vector<SBTEntry>& entries);

	std::vector<SBTEntry> m_rayGen;
	std::vector<SBTEntry> m_miss;
	std::vector<SBTEntry> m_hitGroup;

	/// For each category, the size of an entry in the SBT depends on the maximum number of resources
	/// used by the shaders in that category.The helper computes those values automatically in
	/// GetEntrySize()
	uint32_t m_rayGenEntrySize;
	uint32_t m_missEntrySize;
	uint32_t m_hitGroupEntrySize;

	/// The program names are translated into program identifiers.The size in bytes of an identifier
	/// is provided by the device and is the same for all categories.
	UINT m_progIdSize;
};

class RootSignatureGenerator 
{
public:
	/// Add a set of heap range descriptors as a parameter of the root signature.
	void AddHeapRangesParameter(const std::vector<D3D12_DESCRIPTOR_RANGE>& ranges);

	/// Add a set of heap ranges as a parameter of the root signature. Each range
	/// is defined as follows:
	/// - UINT BaseShaderRegister: the first register index in the range, e.g. the
	/// register of a UAV with BaseShaderRegister==0 is defined in the HLSL code
	/// as register(u0)
	/// - UINT NumDescriptors: number of descriptors in the range. Those will be
	/// mapped to BaseShaderRegister, BaseShaderRegister+1 etc. UINT
	/// RegisterSpace: Allows using the same register numbers multiple times by
	/// specifying a space where they are defined, similarly to a namespace. For
	/// example, a UAV with BaseShaderRegister==0 and RegisterSpace==1 is accessed
	/// in HLSL using the syntax register(u0, space1)
	/// - D3D12_DESCRIPTOR_RANGE_TYPE RangeType: The range type, such as
	/// D3D12_DESCRIPTOR_RANGE_TYPE_CBV for a constant buffer
	/// - UINT OffsetInDescriptorsFromTableStart: The first slot in the heap
	/// corresponding to the buffers mapped by the root signature. This can either
	/// be explicit, or implicit using D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND. In
	/// this case the index in the heap is the one directly following the last
	/// parameter range (or 0 if it's the first)
	void AddHeapRangesParameter(std::vector<std::tuple<UINT, // BaseShaderRegister,
		UINT, // NumDescriptors
		UINT, // RegisterSpace
		D3D12_DESCRIPTOR_RANGE_TYPE, // RangeType
		UINT // OffsetInDescriptorsFromTableStart
		>>
		ranges);

	/// Add a root parameter to the shader, defined by its type: constant buffer (CBV), shader
	/// resource (SRV), unordered access (UAV), or root constant (CBV, directly defined by its value
	/// instead of a buffer). The shaderRegister and registerSpace indicate how to access the
	/// parameter in the HLSL code, e.g a SRV with shaderRegister==1 and registerSpace==0 is
	/// accessible via register(t1, space0).
	/// In case of a root constant, the last parameter indicates how many successive 32-bit constants
	/// will be bound.
	void AddRootParameter(D3D12_ROOT_PARAMETER_TYPE type, UINT shaderRegister = 0,
		UINT registerSpace = 0, UINT numRootConstants = 1);


	/// Create the root signature from the set of parameters, in the order of the addition calls
	ID3D12RootSignature* Generate(ID3D12Device* device, bool isLocal)
	{
		// Go through all the parameters, and set the actual addresses of the heap range descriptors based
		// on their indices in the range set array
		for (size_t i = 0; i < m_parameters.size(); i++)
		{
			if (m_parameters[i].ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE)
			{
				m_parameters[i].DescriptorTable.pDescriptorRanges = m_ranges[m_rangeLocations[i]].data();
			}
		}
		// Specify the root signature with its set of parameters
		D3D12_ROOT_SIGNATURE_DESC rootDesc = {};
		rootDesc.NumParameters = static_cast<UINT>(m_parameters.size());
		rootDesc.pParameters = m_parameters.data();
		// Set the flags of the signature. By default root signatures are global, for example for vertex
		// and pixel shaders. For raytracing shaders the root signatures are local.
		rootDesc.Flags =
			isLocal ? D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE : D3D12_ROOT_SIGNATURE_FLAG_NONE;

		// Create the root signature from its descriptor
		ID3DBlob* pSigBlob;
		ID3DBlob* pErrorBlob;
		HRESULT hr = D3D12SerializeRootSignature(&rootDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &pSigBlob,
			&pErrorBlob);
		if (FAILED(hr))
		{
			ERROR_AND_DIE("Cannot serialize root signature");
		}
		ID3D12RootSignature* pRootSig;
		hr = device->CreateRootSignature(0, pSigBlob->GetBufferPointer(), pSigBlob->GetBufferSize(),
			IID_PPV_ARGS(&pRootSig));
		if (FAILED(hr))
		{
			ERROR_AND_DIE("Cannot create root signature");
		}
		return pRootSig;
	}

	/// Create the root signature from the set of parameters, in the order of the addition calls
	ID3D12RootSignature* GenerateWithSampler(ID3D12Device* device, bool isLocal)
	{
		// Go through all the parameters, and set the actual addresses of the heap range descriptors based
		// on their indices in the range set array
		for (size_t i = 0; i < m_parameters.size(); i++)
		{
			if (m_parameters[i].ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE)
			{
				m_parameters[i].DescriptorTable.pDescriptorRanges = m_ranges[m_rangeLocations[i]].data();
			}
		}
		// Specify the root signature with its set of parameters
		D3D12_ROOT_SIGNATURE_DESC rootDesc = {};
		rootDesc.NumParameters = static_cast<UINT>(m_parameters.size());
		rootDesc.pParameters = m_parameters.data();
		// Set the flags of the signature. By default root signatures are global, for example for vertex
		// and pixel shaders. For raytracing shaders the root signatures are local.
		rootDesc.Flags =
			isLocal ? D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE : D3D12_ROOT_SIGNATURE_FLAG_NONE;

		D3D12_STATIC_SAMPLER_DESC staticSamplerDesc = {};
		staticSamplerDesc.Filter = D3D12_FILTER_MIN_LINEAR_MAG_MIP_POINT;
		staticSamplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		staticSamplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		staticSamplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		staticSamplerDesc.MipLODBias = 0.0f;
		staticSamplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
		staticSamplerDesc.MinLOD = 0.0f;
		staticSamplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
		staticSamplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
		staticSamplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		staticSamplerDesc.ShaderRegister = 0;


		rootDesc.NumStaticSamplers = 1;
		rootDesc.pStaticSamplers = &staticSamplerDesc;

		// Create the root signature from its descriptor
		ID3DBlob* pSigBlob;
		ID3DBlob* pErrorBlob;
		HRESULT hr = D3D12SerializeRootSignature(&rootDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &pSigBlob,
			&pErrorBlob);
		if (FAILED(hr))
		{
			ERROR_AND_DIE("Cannot serialize root signature");
		}
		ID3D12RootSignature* pRootSig;
		hr = device->CreateRootSignature(0, pSigBlob->GetBufferPointer(), pSigBlob->GetBufferSize(),
			IID_PPV_ARGS(&pRootSig));
		if (FAILED(hr))
		{
			ERROR_AND_DIE("Cannot create root signature");
		}
		return pRootSig;
	}

private:
	/// Heap range descriptors
	std::vector<std::vector<D3D12_DESCRIPTOR_RANGE>> m_ranges;
	/// Root parameter descriptors
	std::vector<D3D12_ROOT_PARAMETER> m_parameters;

	/// For each entry of m_parameter, indicate the index of the range array in m_ranges, and ~0u if
	/// the parameter is not a heap range descriptor
	std::vector<UINT> m_rangeLocations;

	enum
	{
		RSC_BASE_SHADER_REGISTER = 0,
		RSC_NUM_DESCRIPTORS = 1,
		RSC_REGISTER_SPACE = 2,
		RSC_RANGE_TYPE = 3,
		RSC_OFFSET_IN_DESCRIPTORS_FROM_TABLE_START = 4
	};
};
#endif // ENABLE_D3D12