#include "Engine/Renderer/D12Helpers.hpp"
#ifdef ENABLE_D3D12
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Renderer/D12CommonHeader.hpp"
#include "Engine/Renderer/RendererD12.hpp"
#include "Engine/Renderer/D12UploadContext.hpp"

#define  DX_SAFE_RELEASE(dxObject)				\
{												\
	if ((dxObject)!=nullptr)					\
	{											\
		(dxObject)->Release();					\
		(dxObject) = nullptr;					\
	}											\
}												\



D12Helpers::~D12Helpers()
{

}

ID3D12RootSignature* D12Helpers::CreateRootSignature(const D3D12_ROOT_SIGNATURE_DESC1& desc)
{

	D3D12_VERSIONED_ROOT_SIGNATURE_DESC versionedDesc{};
	versionedDesc.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
	versionedDesc.Desc_1_1 = desc;

	HRESULT hr = S_OK;
	ComPtr<ID3DBlob> signatureBlob = { nullptr };
	ComPtr<ID3DBlob> errorSigBlob = { nullptr };

	hr = D3D12SerializeVersionedRootSignature(&versionedDesc, &signatureBlob, &errorSigBlob);

	if (!SUCCEEDED(hr))
	{
		DebuggerPrintf((const char*)errorSigBlob->GetBufferPointer());
		return nullptr;
	}

	assert(signatureBlob);
	ID3D12RootSignature* signature = nullptr;
	DXCall(hr = m_ownerRenderer->GetDevice()->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&signature)));

	if (!SUCCEEDED(hr)) 
	{
		DX_SAFE_RELEASE(signature);
	}

	return signature;
}

// ID3D12RootSignature* D12Helpers::CreateRootSignature(const D3D12_ROOT_SIGNATURE_DESC& desc)
// {
// 	D3D12_VERSIONED_ROOT_SIGNATURE_DESC versionedDesc{};
// 	versionedDesc.Version = D3D_ROOT_SIGNATURE_VERSION_1_0;
// 	versionedDesc.Desc_1_0 = desc;
// 
// 	HRESULT hr = S_OK;
// 	ComPtr<ID3DBlob> signatureBlob = { nullptr };
// 	ComPtr<ID3DBlob> errorSigBlob = { nullptr };
// 
// 	hr = D3D12SerializeVersionedRootSignature(&versionedDesc, &signatureBlob, &errorSigBlob);
// 
// 	if (!SUCCEEDED(hr))
// 	{
// 		DebuggerPrintf((const char*)errorSigBlob->GetBufferPointer());
// 		return nullptr;
// 	}
// 
// 	assert(signatureBlob);
// 	ID3D12RootSignature* signature = nullptr;
// 	DXCall(hr = m_ownerRenderer->GetDevice()->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&signature)));
// 
// 	if (!SUCCEEDED(hr))
// 	{
// 		DX_SAFE_RELEASE(signature);
// 	}
// 
// 	return signature;
// }

ID3D12PipelineState* D12Helpers::CreatePipelineState(void* stream, u32 streamSize)
{
	assert(stream && streamSize);
	D3D12_PIPELINE_STATE_STREAM_DESC desc{};
	desc.pPipelineStateSubobjectStream = stream;
	desc.SizeInBytes = streamSize;

	return CreatePipelineState(desc);
}

ID3D12PipelineState* D12Helpers::CreatePipelineState(D3D12_PIPELINE_STATE_STREAM_DESC desc)
{
	assert(desc.pPipelineStateSubobjectStream && desc.SizeInBytes);
	assert(m_ownerRenderer);
	ID3D12PipelineState* pos = nullptr;
	DXCall(m_ownerRenderer->GetDevice()->CreatePipelineState(&desc, IID_PPV_ARGS(&pos)));
	assert(pos);
	return pos;
}

void D12Helpers::TransitionResource(id3d12GraphicsCommandList* cmdList, ID3D12Resource* resource, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after, D3D12_RESOURCE_BARRIER_FLAGS flags /*= D3D12_RESOURCE_BARRIER_FLAG_NONE*/, u32 subresource /*= D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES*/)
{
	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = flags;
	barrier.Transition.pResource = resource;
	barrier.Transition.StateBefore = before;
	barrier.Transition.StateAfter = after;
	barrier.Transition.Subresource = subresource;

	cmdList->ResourceBarrier(1, &barrier);
}

ID3D12Resource* D12Helpers::CreateBuffer(const void* data, u32 bufferSize, bool isCPUAccessible /*= false*/,
	D3D12_RESOURCE_STATES state /*= D3D12_RESOURCE_STATE_COMMON*/, D3D12_RESOURCE_FLAGS flags /*= D3D12_RESOURCE_FLAG_NONE*/, ID3D12Heap* heap /*= nullptr*/, u64 heapOffset /*= 0*/)
{
	assert(bufferSize);

	D3D12_RESOURCE_DESC desc{};
	desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	desc.Alignment = 0;
	desc.Width = bufferSize;
	desc.Height = 1;
	desc.DepthOrArraySize = 1;
	desc.MipLevels = 1;
	desc.Format = DXGI_FORMAT_UNKNOWN;
	desc.SampleDesc = { 1,0 };
	desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	desc.Flags = isCPUAccessible ? D3D12_RESOURCE_FLAG_NONE : flags;

	// The buffer will be only used for upload or as constant buffer/UAV.
	assert(desc.Flags == D3D12_RESOURCE_FLAG_NONE ||
		desc.Flags == D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

	ID3D12Resource* resource = nullptr;
	const D3D12_RESOURCE_STATES resource_state = 
		isCPUAccessible ? D3D12_RESOURCE_STATE_GENERIC_READ : state;

	if (heap)
	{
		DXCall(m_ownerRenderer->GetDevice()->CreatePlacedResource(
			heap, heapOffset, &desc, resource_state,
			nullptr, IID_PPV_ARGS(&resource)));
	}
	else
	{
		DXCall(m_ownerRenderer->GetDevice()->CreateCommittedResource(
			isCPUAccessible ? &HeapProperties.uploadHeap : &HeapProperties.defaultHeap,
			D3D12_HEAP_FLAG_NONE, &desc, resource_state,
			nullptr, IID_PPV_ARGS(&resource)));
	}

	if (data) 
	{
		// If we have initial data which we'd like to be able to change later, we set is_cpu_accessible
		// to true. If we only want to upload some data once to be used by the GPU, then is_cpu_accessible
		// should be set to false.

		if (isCPUAccessible)
		{
			// NOTE: range's Begin and End fields are set to 0, to indicate that
			//       the CPU is not reading any data (i.e. write-only)
			const D3D12_RANGE range{};
			void* cpuAddress{ nullptr };
			DXCall(resource->Map(0, &range, reinterpret_cast<void**>(&cpuAddress)));
			assert(cpuAddress);
			memcpy(cpuAddress, data, bufferSize);
			resource->Unmap(0, nullptr);
		}
		else
		{
			//D12UploadContext context{ m_ownerRenderer, bufferSize };
			D12UploadContext context = D12UploadContext(m_ownerRenderer, bufferSize);
			memcpy(context.GetCPUAddress(), data, bufferSize);
			context.GetCommandList()->CopyResource(resource, context.GetUploadBuffer());
			context.EndUpload();
		}
	}
	assert(resource);
	return resource;
}

u64 D12Helpers::AlignSizeForConstantBuffer(u64 size)
{
	u64 alignment = D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT;
	u64 mask = alignment - 1;
	u64 result = ((size + mask) & ~mask);
	return result;
}

u64 D12Helpers::AlignSizeForTextureBuffer(u64 size)
{
	u64 alignment = D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT;
	u64 mask = alignment - 1;
	u64 result = ((size + mask) & ~mask);
	return result;
}

#endif // ENABLE_D3D12