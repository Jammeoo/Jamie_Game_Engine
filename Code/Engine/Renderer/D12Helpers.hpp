#pragma once
#include "Engine/Renderer/D12CommonHeader.hpp"

#ifdef ENABLE_D3D12
class RendererD12;
struct 
{
	const D3D12_HEAP_PROPERTIES defaultHeap{
		D3D12_HEAP_TYPE_DEFAULT,
		D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
		D3D12_MEMORY_POOL_UNKNOWN,
		0,
		0
	};
	const D3D12_HEAP_PROPERTIES uploadHeap{
	D3D12_HEAP_TYPE_UPLOAD,
	D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
	D3D12_MEMORY_POOL_UNKNOWN,
	0,
	0
	};
}HeapProperties;

constexpr struct 
{
	const D3D12_RASTERIZER_DESC noCull
	{
		D3D12_FILL_MODE_SOLID,					    //FillMode
		D3D12_CULL_MODE_NONE,						//CullMode
		0,											//FrontCounterClockwise
		0,											//DepthBias
		0,											//DepthBiasClamp
		0,											//SlopeScaledDepthBias
		1,											//DepthClipEnable
		1,											//MultisampleEnable
		0,											//AntialiasedLineEnable
		0,											//ForcedSampleCount
		D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF	//ConservativeRaster
	};

	const D3D12_RASTERIZER_DESC backfaceCull
	{
		D3D12_FILL_MODE_SOLID,					    //FillMode
		D3D12_CULL_MODE_BACK,						//CullMode
		0,											//FrontCounterClockwise
		0,											//DepthBias
		0,											//DepthBiasClamp
		0,											//SlopeScaledDepthBias
		1,											//DepthClipEnable
		1,											//MultisampleEnable
		0,											//AntialiasedLineEnable
		0,											//ForcedSampleCount
		D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF	//ConservativeRaster
	};

	const D3D12_RASTERIZER_DESC frontfaceCull
	{
		D3D12_FILL_MODE_SOLID,					    //FillMode
		D3D12_CULL_MODE_FRONT,						//CullMode
		0,											//FrontCounterClockwise
		0,											//DepthBias
		0,											//DepthBiasClamp
		0,											//SlopeScaledDepthBias
		1,											//DepthClipEnable
		1,											//MultisampleEnable
		0,											//AntialiasedLineEnable
		0,											//ForcedSampleCount
		D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF	//ConservativeRaster
	};

	const D3D12_RASTERIZER_DESC wireframe
	{
		D3D12_FILL_MODE_WIREFRAME,					//FillMode
		D3D12_CULL_MODE_NONE,						//CullMode
		0,											//FrontCounterClockwise
		0,											//DepthBias
		0,											//DepthBiasClamp
		0,											//SlopeScaledDepthBias
		1,											//DepthClipEnable
		1,											//MultisampleEnable
		0,											//AntialiasedLineEnable
		0,											//ForcedSampleCount
		D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF	//ConservativeRaster
	};
}rasterizerState;

constexpr struct 
{
	const D3D12_DEPTH_STENCIL_DESC1 disabled
	{
		0,	//DepthEnable
		D3D12_DEPTH_WRITE_MASK_ZERO,	//DepthWriteMask
		D3D12_COMPARISON_FUNC_LESS_EQUAL,	//DepthFunc
		0,	//StencilEnable
		0,	//StencilReadMask
		0,	//StencilWriteMask
		{},	//FrontFace
		{},	//BackFace
		0	//DepthBoundsTestEnable
	};

	const D3D12_DEPTH_STENCIL_DESC1 enabled
	{
		1,											//DepthEnable
		D3D12_DEPTH_WRITE_MASK_ALL,					//DepthWriteMask
		D3D12_COMPARISON_FUNC_LESS_EQUAL,			//DepthFunc
		0,											//StencilEnable
		0,											//StencilReadMask
		0,											//StencilWriteMask
		{},											//FrontFace
		{},											//BackFace
		0											//DepthBoundsTestEnable
	};
	const D3D12_DEPTH_STENCIL_DESC1 enabledReadonly
	{
		1,											//DepthEnable
		D3D12_DEPTH_WRITE_MASK_ZERO,				//DepthWriteMask
		D3D12_COMPARISON_FUNC_LESS_EQUAL,			//DepthFunc
		0,											//StencilEnable
		0,											//StencilReadMask
		0,											//StencilWriteMask
		{},											//FrontFace
		{},											//BackFace
		0											//DepthBoundsTestEnable
	};

	const D3D12_DEPTH_STENCIL_DESC1 reversed
	{
		1,											//DepthEnable
		D3D12_DEPTH_WRITE_MASK_ALL,					//DepthWriteMask
		D3D12_COMPARISON_FUNC_GREATER_EQUAL,		//DepthFunc
		0,											//StencilEnable
		0,											//StencilReadMask
		0,											//StencilWriteMask
		{},											//FrontFace
		{},											//BackFace
		0											//DepthBoundsTestEnable
	};
	const D3D12_DEPTH_STENCIL_DESC1 reversedReadOnly
	{
		1,											//DepthEnable
		D3D12_DEPTH_WRITE_MASK_ZERO,					//DepthWriteMask
		D3D12_COMPARISON_FUNC_GREATER_EQUAL,		//DepthFunc
		0,											//StencilEnable
		0,											//StencilReadMask
		0,											//StencilWriteMask
		{},											//FrontFace
		{},											//BackFace
		0											//DepthBoundsTestEnable
	};
}depthState;

constexpr struct {
	const D3D12_BLEND_DESC disabled{
		0,                                              // AlphaToCoverageEnable
		0,                                              // IndependentBlendEnable
		{
			{
			   0,                                       // BlendEnable
			   0,                                       // LogicOpEnable
			   D3D12_BLEND_SRC_ALPHA,                   // SrcBlend
			   D3D12_BLEND_INV_SRC_ALPHA,               // DestBlend
			   D3D12_BLEND_OP_ADD,                      // BlendOp
			   D3D12_BLEND_ONE,                         // SrcBlendAlpha
			   D3D12_BLEND_ONE,                         // DestBlendAlpha
			   D3D12_BLEND_OP_ADD,                      // BlendOpAlpha
			   D3D12_LOGIC_OP_NOOP,                     // LogicOp
			   D3D12_COLOR_WRITE_ENABLE_ALL,            // RenderTargetWriteMask

			},
			{},{},{},{},{},{},{}
		}
	};

	const D3D12_BLEND_DESC alphaBlend{
		0,                                              // AlphaToCoverageEnable
		0,                                              // IndependentBlendEnable
		{
			{
			   1,                                       // BlendEnable
			   0,                                       // LogicOpEnable
			   D3D12_BLEND_SRC_ALPHA,                   // SrcBlend
			   D3D12_BLEND_INV_SRC_ALPHA,               // DestBlend
			   D3D12_BLEND_OP_ADD,                      // BlendOp
			   D3D12_BLEND_ONE,                         // SrcBlendAlpha
			   D3D12_BLEND_ONE,                         // DestBlendAlpha
			   D3D12_BLEND_OP_ADD,                      // BlendOpAlpha
			   D3D12_LOGIC_OP_NOOP,                     // LogicOp
			   D3D12_COLOR_WRITE_ENABLE_ALL,            // RenderTargetWriteMask

			},
			{},{},{},{},{},{},{}
		}
	};

	const D3D12_BLEND_DESC additive{
		0,                                              // AlphaToCoverageEnable
		0,                                              // IndependentBlendEnable
		{
			{
			   1,                                       // BlendEnable
			   0,                                       // LogicOpEnable
			   D3D12_BLEND_ONE,                         // SrcBlend
			   D3D12_BLEND_ONE,                         // DestBlend
			   D3D12_BLEND_OP_ADD,                      // BlendOp
			   D3D12_BLEND_ONE,                         // SrcBlendAlpha
			   D3D12_BLEND_ONE,                         // DestBlendAlpha
			   D3D12_BLEND_OP_ADD,                      // BlendOpAlpha
			   D3D12_LOGIC_OP_NOOP,                     // LogicOp
			   D3D12_COLOR_WRITE_ENABLE_ALL,            // RenderTargetWriteMask

			},
			{},{},{},{},{},{},{}
		}
	};

	const D3D12_BLEND_DESC premultiplied{
		0,                                              // AlphaToCoverageEnable
		0,                                              // IndependentBlendEnable
		{
			{
			   0,                                       // BlendEnable
			   0,                                       // LogicOpEnable
			   D3D12_BLEND_ONE,                         // SrcBlend
			   D3D12_BLEND_INV_SRC_ALPHA,               // DestBlend
			   D3D12_BLEND_OP_ADD,                      // BlendOp
			   D3D12_BLEND_ONE,                         // SrcBlendAlpha
			   D3D12_BLEND_ONE,                         // DestBlendAlpha
			   D3D12_BLEND_OP_ADD,                      // BlendOpAlpha
			   D3D12_LOGIC_OP_NOOP,                     // LogicOp
			   D3D12_COLOR_WRITE_ENABLE_ALL,            // RenderTargetWriteMask

			},
			{},{},{},{},{},{},{}
		}
	};
} blendState;


class D12ResourceBarrier 
{
public:
	constexpr static u32 m_maxResourceBarriers = 32;
	//Add a transition barrier to the list of barriers
	constexpr void Add(ID3D12Resource* resource, 
		D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after,
		D3D12_RESOURCE_BARRIER_FLAGS flags = D3D12_RESOURCE_BARRIER_FLAG_NONE, 
		u32 subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES) 
	{
		assert(resource);
		assert(m_offset < m_maxResourceBarriers);
		D3D12_RESOURCE_BARRIER& barrier{ m_barriers[m_offset] };
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = flags;
		barrier.Transition.pResource = resource;
		barrier.Transition.StateBefore = before;
		barrier.Transition.StateAfter = after;
		barrier.Transition.Subresource = subresource;

		++m_offset;
	}
	//Add a UAV barrier to the list barriers.
	constexpr void Add(ID3D12Resource* resource,
		D3D12_RESOURCE_BARRIER_FLAGS flags = D3D12_RESOURCE_BARRIER_FLAG_NONE) 
	{
		assert(resource);
		assert(m_offset < m_maxResourceBarriers);
		D3D12_RESOURCE_BARRIER& barrier{ m_barriers[m_offset] };
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
		barrier.Flags = flags;
		barrier.UAV.pResource = resource;
		++m_offset;

	}
	//Add An aliasing barrier to the list of barriers
	constexpr void Add(ID3D12Resource* resourceBefore, ID3D12Resource* resourceAfter,
		D3D12_RESOURCE_BARRIER_FLAGS flags = D3D12_RESOURCE_BARRIER_FLAG_NONE)
	{
		assert(resourceBefore);
		assert(resourceAfter);

		assert(m_offset < m_maxResourceBarriers);
		D3D12_RESOURCE_BARRIER& barrier{ m_barriers[m_offset] };
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_ALIASING;
		barrier.Flags = flags;
		barrier.Aliasing.pResourceBefore = resourceBefore;
		barrier.Aliasing.pResourceAfter = resourceAfter;


		++m_offset;
	}

	void Apply(id3d12GraphicsCommandList* cmdList) 
	{
		assert(m_offset);
		cmdList->ResourceBarrier(m_offset, m_barriers);
		m_offset = 0;
	}

private:
	u32 m_offset = 0;
	D3D12_RESOURCE_BARRIER m_barriers[m_maxResourceBarriers]{};
};

class D12Helpers
{
public:
	D12Helpers(RendererD12* renderer) :m_ownerRenderer(renderer)
	{

	};
	~D12Helpers();
	ID3D12RootSignature* CreateRootSignature(const D3D12_ROOT_SIGNATURE_DESC1& desc);

	//ID3D12RootSignature* CreateRootSignature(const D3D12_ROOT_SIGNATURE_DESC& desc);

	ID3D12PipelineState* CreatePipelineState(void* stream, u32 streamSize);
	ID3D12PipelineState* CreatePipelineState(D3D12_PIPELINE_STATE_STREAM_DESC desc);
	void TransitionResource(id3d12GraphicsCommandList* cmdList, ID3D12Resource* resource, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after,
		D3D12_RESOURCE_BARRIER_FLAGS flags = D3D12_RESOURCE_BARRIER_FLAG_NONE, u32 subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);

	ID3D12Resource* CreateBuffer(const void* data, u32 bufferSize, bool isCPUAccessible = false,
		D3D12_RESOURCE_STATES state = D3D12_RESOURCE_STATE_COMMON,
		D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE,
		ID3D12Heap* heap = nullptr, u64 heapOffset = 0);
	u64 AlignSizeForConstantBuffer(u64 size);

	u64 AlignSizeForTextureBuffer(u64 size);

private:
	RendererD12* m_ownerRenderer = nullptr;
};



// struct D12DescriptorRange:public D3D12_DESCRIPTOR_RANGE 
// {
// 	constexpr explicit D12DescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE rangeType, u32 descriptorCount, u32 shaderRegister, u32 space = 0,
// 		D3D12_DESCRIPTOR_RANGE_FLAGS flags = D3D12_DESCRIPTOR_RANGE_FLAG_NONE,
// 		/*u32 offsetFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND*/)
// 		:D3D12_DESCRIPTOR_RANGE{ rangeType,descriptorCount,shaderRegister,space,flags } 
// 	{
// 
// 	}
// };

struct D12DescriptorRange :public D3D12_DESCRIPTOR_RANGE1
{
	D12DescriptorRange() = default;
	constexpr explicit D12DescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE rangeType, u32 descriptorCount, u32 shaderRegister, u32 space = 0,
		D3D12_DESCRIPTOR_RANGE_FLAGS flags = D3D12_DESCRIPTOR_RANGE_FLAG_NONE,
		u32 offsetFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND)
		:D3D12_DESCRIPTOR_RANGE1{ rangeType,descriptorCount,shaderRegister,space,flags,offsetFromTableStart }
	{

	}
};

struct D12RootParameter:public D3D12_ROOT_PARAMETER1 
{
	constexpr void AsConstants(u32 NumConstants, D3D12_SHADER_VISIBILITY visibility,
		u32 shaderRegister, u32 space = 0) 
	{
		ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
		ShaderVisibility = visibility;
		Constants.Num32BitValues = NumConstants;
		Constants.ShaderRegister = shaderRegister;
		Constants.RegisterSpace = space;
	}
	constexpr void AsCBV(D3D12_SHADER_VISIBILITY visibility,
		u32 shaderRegister, u32 space = 0, 
		D3D12_ROOT_DESCRIPTOR_FLAGS flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE) 
	{
		AsDescriptor(D3D12_ROOT_PARAMETER_TYPE_CBV, visibility, shaderRegister, space, flags);
	}
	constexpr void AsSRV(D3D12_SHADER_VISIBILITY visibility,
		u32 shaderRegister, u32 space = 0, 
		D3D12_ROOT_DESCRIPTOR_FLAGS flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE)
	{
		AsDescriptor(D3D12_ROOT_PARAMETER_TYPE_SRV, visibility, shaderRegister, space, flags);
	}
	constexpr void AsUAV(D3D12_SHADER_VISIBILITY visibility,
		u32 shaderRegister, u32 space = 0, 
		D3D12_ROOT_DESCRIPTOR_FLAGS flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE)
	{
		AsDescriptor(D3D12_ROOT_PARAMETER_TYPE_UAV, visibility, shaderRegister, space, flags);
	}
	constexpr void AsDescriptorTable(D3D12_SHADER_VISIBILITY visibility, const D12DescriptorRange* ranges, u32 rangeCount) 
	{
		ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		ShaderVisibility = visibility;
		DescriptorTable.NumDescriptorRanges = rangeCount;
		DescriptorTable.pDescriptorRanges = ranges;
	}
private:
	constexpr void AsDescriptor(D3D12_ROOT_PARAMETER_TYPE type, D3D12_SHADER_VISIBILITY visibility,
		u32 shaderRegister, u32 space, D3D12_ROOT_DESCRIPTOR_FLAGS flags) 
	{
		ParameterType = type;
		ShaderVisibility = visibility;
		Descriptor.ShaderRegister = shaderRegister;
		Descriptor.RegisterSpace = space;
		Descriptor.Flags = flags;

	}
};

struct D12RootSignatureDesc :public D3D12_ROOT_SIGNATURE_DESC1 
{
	constexpr static D3D12_ROOT_SIGNATURE_FLAGS defaultFlags{
		D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_AMPLIFICATION_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_MESH_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED |
		D3D12_ROOT_SIGNATURE_FLAG_SAMPLER_HEAP_DIRECTLY_INDEXED
	};

	constexpr explicit D12RootSignatureDesc(D12Helpers* helper, const D12RootParameter* parameters,
		u32 parameterCount, D3D12_ROOT_SIGNATURE_FLAGS flags = defaultFlags, 
		const D3D12_STATIC_SAMPLER_DESC* staticSapmler = nullptr,
		u32 samplerCount = 0
		) 
		:D3D12_ROOT_SIGNATURE_DESC1{parameterCount,parameters,samplerCount,staticSapmler,flags}
	{
		m_helper = helper;
	}

	ID3D12RootSignature* Create()const 
	{
		return m_helper->CreateRootSignature(*this);
	}
private:
	D12Helpers* m_helper = nullptr;
};

#pragma warning(push)
#pragma warning(disable : 4324)	//disable padding warning

template<D3D12_PIPELINE_STATE_SUBOBJECT_TYPE type, typename T>
class alignas(void*) D12PipelineStateSubobject
//class D12PipelineStateSubobject
{
public:
	D12PipelineStateSubobject() = default;
	constexpr explicit D12PipelineStateSubobject(T subobject) :m_type(type), m_subobject(subobject) {}
	D12PipelineStateSubobject& operator=(const T& subobject) {
		m_subobject = subobject;
		return *this;
	}


private:
	const D3D12_PIPELINE_STATE_SUBOBJECT_TYPE m_type = type;
	T m_subobject{};

};

#pragma warning(pop)
//Pipeline State Sub-object (PSS) macro
#define PSS(name, ...) using D12PipelineStateSubobject##name = D12PipelineStateSubobject<__VA_ARGS__>;
PSS(rootSignature, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_ROOT_SIGNATURE, ID3D12RootSignature*);

PSS(vs, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_VS, D3D12_SHADER_BYTECODE);
PSS(ps, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_PS, D3D12_SHADER_BYTECODE);
PSS(ds, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_DS, D3D12_SHADER_BYTECODE);
PSS(hs, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_HS, D3D12_SHADER_BYTECODE);
PSS(gs, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_GS, D3D12_SHADER_BYTECODE);
PSS(cs, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_CS, D3D12_SHADER_BYTECODE);
PSS(streamOutput, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_STREAM_OUTPUT, D3D12_STREAM_OUTPUT_DESC);
PSS(blend, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_BLEND, D3D12_BLEND_DESC);
PSS(sampleMask, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_SAMPLE_MASK, u32);
PSS(rasterizer, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_RASTERIZER, D3D12_RASTERIZER_DESC);
PSS(depthSencil, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_DEPTH_STENCIL, D3D12_DEPTH_STENCIL_DESC);
PSS(inputLayout, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_INPUT_LAYOUT, D3D12_INPUT_LAYOUT_DESC);
PSS(ibStripCutValue, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_IB_STRIP_CUT_VALUE, D3D12_INDEX_BUFFER_STRIP_CUT_VALUE);
PSS(primitiveTopology, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_PRIMITIVE_TOPOLOGY, D3D12_PRIMITIVE_TOPOLOGY_TYPE);
PSS(renderTargetFormats, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_RENDER_TARGET_FORMATS, D3D12_RT_FORMAT_ARRAY);
PSS(depthStencilFormat, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_DEPTH_STENCIL_FORMAT, DXGI_FORMAT);
PSS(sampleDesc, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_SAMPLE_DESC, DXGI_SAMPLE_DESC);
PSS(nodeMask, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_NODE_MASK, u32);
PSS(cachedPSO, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_CACHED_PSO, D3D12_CACHED_PIPELINE_STATE);
PSS(flags, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_FLAGS, D3D12_PIPELINE_STATE_FLAGS);
PSS(depthStencil1, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_DEPTH_STENCIL1, D3D12_DEPTH_STENCIL_DESC1);
PSS(viewInstancing, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_VIEW_INSTANCING, D3D12_VIEW_INSTANCING_DESC);
PSS(as, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_AS, D3D12_SHADER_BYTECODE);
PSS(ms ,D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_MS, D3D12_SHADER_BYTECODE);
#undef PSS

struct D12PipelineStateSubobjectStream 
{
	D12PipelineStateSubobjectrootSignature rootSignature{ nullptr };
	D12PipelineStateSubobjectvs									vs{};
	D12PipelineStateSubobjectps									ps{};
	D12PipelineStateSubobjectds									ds{};
	D12PipelineStateSubobjecths									hs{};
	D12PipelineStateSubobjectgs									gs{};
	D12PipelineStateSubobjectcs									cs{};
	D12PipelineStateSubobjectstreamOutput						streamOutput{};
	D12PipelineStateSubobjectblend								blend{blendState.disabled};
	D12PipelineStateSubobjectsampleMask							sampleMask{ UINT_MAX };
	D12PipelineStateSubobjectrasterizer							rasterizer{ rasterizerState.noCull };
	D12PipelineStateSubobjectinputLayout						inputLayout{};
	D12PipelineStateSubobjectibStripCutValue					ibStripCutValue{};
	D12PipelineStateSubobjectprimitiveTopology					primitiveTopology{ D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE };
	D12PipelineStateSubobjectrenderTargetFormats				renderTargetFormats{};
	D12PipelineStateSubobjectdepthStencilFormat					depthStencilFormat{};
	D12PipelineStateSubobjectsampleDesc							sampleDesc{ {1,0} };
	D12PipelineStateSubobjectnodeMask							nodeMask{};
	D12PipelineStateSubobjectcachedPSO							cachedpso{};
	D12PipelineStateSubobjectflags								flags{};
	D12PipelineStateSubobjectdepthStencil1						depthStencil1{ depthState.disabled };
	D12PipelineStateSubobjectviewInstancing						viewInstancing{};
	D12PipelineStateSubobjectas									as{};
	D12PipelineStateSubobjectms									ms{};
};							 




#endif // ENABLE_D3D12