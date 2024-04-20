#include "Engine/Renderer/D12UploadContext.hpp"
#ifdef ENABLE_D3D12
#include "Engine/Renderer/RendererD12.hpp"
#include "Engine/Renderer/D12Helpers.hpp"


D12UploadContext::D12UploadContext(RendererD12* renderer, u32 alignedSize):m_ownerRenderer(renderer)
{
	// We don't want to lock this function for longer than necessary. So, we scope this lock.
	{
		std::lock_guard lock{ m_frameMutex };
		m_frameIndex = GetAvailableUploadFrame();

		assert(m_frameIndex != U32_INVALID_ID);
		// Before unlocking, we prevent other threads from picking
		// this frame by making IsReady() return false.
		m_uploadFrames[m_frameIndex].m_uploadBuffer = (ID3D12Resource*)1;
	}

	D12Helpers* helper = m_ownerRenderer->GetHelper();

	UploadFrame& frame = m_uploadFrames[m_frameIndex];
	if (m_ownerRenderer->GetUploadContext())
	{
		frame = m_ownerRenderer->GetUploadContext()->m_uploadFrames[m_frameIndex];
	}
	frame.m_uploadBuffer = helper->CreateBuffer(nullptr, alignedSize, true);
	//frame.m_uploadBuffer->SetName(L"Upload Buffer - size " + alignedSize);
	NAME_D3D12_OBJECT_INDEXED(frame.m_uploadBuffer, (u64)alignedSize, L"Upload Buffer");
	
	const D3D12_RANGE range{};
	DXCall(frame.m_uploadBuffer->Map(0, &range, reinterpret_cast<void**>(&frame.m_cpuAddress)));
	assert(frame.m_cpuAddress);

	m_cmdList = frame.m_cmdList;
	m_uploadBuffer = frame.m_uploadBuffer;
	m_cpuAddress = frame.m_cpuAddress;

	assert(m_cmdList && m_uploadBuffer && m_cpuAddress);

	DXCall(frame.m_cmdAllocator->Reset());
	DXCall(frame.m_cmdList->Reset(frame.m_cmdAllocator, nullptr));

}

D12UploadContext::D12UploadContext(RendererD12* renderer) :m_ownerRenderer(renderer)
{

}

D12UploadContext::~D12UploadContext()
{
	//assert(m_frameIndex == U32_INVALID_ID);
	assert(m_ownerRenderer->GetUploadContext()->m_frameIndex != U32_INVALID_ID);
}

bool D12UploadContext::Initialize()
{
	id3d12Device* const device = m_ownerRenderer->GetDevice();
	assert(device && !m_uploadCmdQueue);
	HRESULT hr = S_OK;

	for (u32 i = 0; i < UPLOAD_FRAME_COUNT; ++i) 
	{
		UploadFrame& frame = m_uploadFrames[i];
		frame.SetUploadContext(this);
		DXCall(hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COPY, IID_PPV_ARGS(&frame.m_cmdAllocator)));
		if (!SUCCEEDED(hr)) return InitFailed();
		DXCall(hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_COPY, frame.m_cmdAllocator, nullptr, IID_PPV_ARGS(&frame.m_cmdList)));
		if (!SUCCEEDED(hr)) return InitFailed();
		DXCall(frame.m_cmdList->Close());

		frame.m_cmdAllocator->SetName(L"Upload Command Allocator" + i);
		frame.m_cmdList->SetName(L"Upload Command List" + i);

	}
	D3D12_COMMAND_QUEUE_DESC desc{};
	desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	desc.NodeMask = 0;
	desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	desc.Type = D3D12_COMMAND_LIST_TYPE_COPY;

	DXCall(hr = device->CreateCommandQueue(&desc, IID_PPV_ARGS(&m_uploadCmdQueue)));
	if (!SUCCEEDED(hr)) return InitFailed();
	m_uploadCmdQueue->SetName(L"Upload Copy Queue");

	DXCall(hr = device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_uploadFence)));
	if (!SUCCEEDED(hr)) return InitFailed();
	m_uploadFence->SetName(L"Upload Fence");

	m_fenceEvent = CreateEventEx(nullptr, nullptr, 0, EVENT_ALL_ACCESS);
	assert(m_fenceEvent);
	if (!m_fenceEvent) return InitFailed();

	return true;
}

void D12UploadContext::ShutDown()
{
	for (u32 i = 0; i < UPLOAD_FRAME_COUNT; ++i) 
	{
		m_uploadFrames[i].Release();
	}
	if (m_fenceEvent) 
	{
		CloseHandle(m_fenceEvent);
		m_fenceEvent = nullptr;
	}

	DX_SAFE_RELEASE(m_uploadCmdQueue);
	DX_SAFE_RELEASE(m_uploadFence);
	m_uploadFenceValue = 0;
}

void D12UploadContext::EndUpload()
{
	assert(m_frameIndex != U32_INVALID_ID);
	UploadFrame& frame = m_uploadFrames[m_frameIndex];
	if (m_ownerRenderer->GetUploadContext())
	{
		frame = m_ownerRenderer->GetUploadContext()->m_uploadFrames[m_frameIndex];
	}
	id3d12GraphicsCommandList* const cmdList = frame.m_cmdList;
	DXCall(cmdList->Close());

	std::lock_guard lock{ m_ownerRenderer->GetUploadContext()->m_queueMutex };
	//std::lock_guard lock{ m_queueMutex };
	ID3D12CommandList* const cmdLists[]{ cmdList };
	//ID3D12CommandQueue* const cmdQueue{ m_uploadCmdQueue };
	ID3D12CommandQueue* cmdQueue{ m_uploadCmdQueue };
	if (!cmdQueue) 
	{
		cmdQueue = m_ownerRenderer->GetUploadContext()->m_uploadCmdQueue;
	}
	cmdQueue->ExecuteCommandLists(_countof(cmdLists), cmdLists);
	//++m_uploadFenceValue;
	++m_ownerRenderer->GetUploadContext()->m_uploadFenceValue;
	m_uploadFenceValue = m_ownerRenderer->GetUploadContext()->m_uploadFenceValue;
	frame.m_fenceValue = m_uploadFenceValue;
	if (!m_uploadFence) 
	{
		m_uploadFence = m_ownerRenderer->GetUploadContext()->m_uploadFence;
	}
	DXCall(cmdQueue->Signal(m_uploadFence, frame.m_fenceValue));

	// Wait for copy queue to finish. Then release the upload buffer.
	frame.WaitAndReset();
	// This instance of upload context is now expired. Make sure we don't use it again.
	new (this) D12UploadContext{ m_ownerRenderer };

}

bool D12UploadContext::InitFailed()
{
	ShutDown();
	return false;
}

// NOTE: frames should be locked before this function is called.
u32 D12UploadContext::GetAvailableUploadFrame()
{
	u32 index = U32_INVALID_ID;

	for (u32 i=0; i < UPLOAD_FRAME_COUNT; ++i)
	{
		if (m_uploadFrames[i].IsReady())
		{
			index = i;
			break;
		}
	}

	// None of the frames were done uploading. We're the only thread here, so
	// we can iterate through frames until we find one that is ready.
	if (index == U32_INVALID_ID)
	{
		index = 0;
		while (!m_uploadFrames[index].IsReady())
		{
			index = (index + 1) % UPLOAD_FRAME_COUNT;
			std::this_thread::yield();
		}
	}

	return index;
}

#endif // ENABLE_D3D12