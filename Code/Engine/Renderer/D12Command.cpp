#include "Engine/Renderer/D12Command.hpp"

#ifdef ENABLE_D3D12
#include "Engine/Renderer/D12Surface.hpp"
#define  DX_SAFE_RELEASE(dxObject)				\
{												\
	if ((dxObject)!=nullptr)					\
	{											\
		(dxObject)->Release();					\
		(dxObject) = nullptr;					\
	}											\
}												\

D12Command::D12Command(id3d12Device* const device, D3D12_COMMAND_LIST_TYPE type, ID3D12PipelineState* optionalState /*= nullptr*/)
{
	HRESULT hr;
	D3D12_COMMAND_QUEUE_DESC desc = {};
	desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	desc.NodeMask = 0;
	desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	desc.Type = type;

	hr = device->CreateCommandQueue(&desc, IID_PPV_ARGS(&m_cmdQueue));

	if (!SUCCEEDED(hr))
	{
		goto _error;
	}
	if (type == D3D12_COMMAND_LIST_TYPE_DIRECT)
	{
		m_cmdQueue->SetName(L"GFX Command Queue");
	}
	else if (type == D3D12_COMMAND_LIST_TYPE_COMPUTE)
	{
		m_cmdQueue->SetName(L"Compute Command Queue");
	}
	else
	{
		m_cmdQueue->SetName(L"Command Queue");
	}

	for (int i = 0; i < FRAME_BUFFER_COUNT; ++i)
	{
		CommandFrame& frame = m_cmdFrames[i];
		hr = device->CreateCommandAllocator(type, IID_PPV_ARGS(&frame.m_cmdAllocator));

		wchar_t fullName[128];
		if (type == D3D12_COMMAND_LIST_TYPE_DIRECT)
		{
			const wchar_t* name = L"GFX Command Allocator";
			if (swprintf_s(fullName, L"%s[%u]", name, i) > 0)
			{
				frame.m_cmdAllocator->SetName(fullName);
			}

		}
		else if (type == D3D12_COMMAND_LIST_TYPE_COMPUTE)
		{
			const wchar_t* name = L"Compute Command Allocator";
			if (swprintf_s(fullName, L"%s[%u]", name, i) > 0)
			{
				frame.m_cmdAllocator->SetName(fullName);
			}
		}
		else
		{
			const wchar_t* name = L"Command Allocator";
			if (swprintf_s(fullName, L"%s[%u]", name, i) > 0)
			{
				frame.m_cmdAllocator->SetName(fullName);
			}
		}

		if (!SUCCEEDED(hr))
		{
			goto _error;
		}
	}


	hr = device->CreateCommandList(0, type, m_cmdFrames[0].m_cmdAllocator, optionalState, IID_PPV_ARGS(&m_cmdList));

	if (!SUCCEEDED(hr))
	{
		goto _error;
	}
	//hr = m_cmdList->Close();
	if (!SUCCEEDED(hr))
	{
		goto _error;
	}

	if (type == D3D12_COMMAND_LIST_TYPE_DIRECT)
	{
		m_cmdList->SetName(L"GFX Command List");
		DebuggerPrintf("D12 Object Created: GFX Command List\n");
	}
	else if (type == D3D12_COMMAND_LIST_TYPE_COMPUTE)
	{
		m_cmdList->SetName(L"Compute Command List");
		DebuggerPrintf("D12 Object Created: Compute Command List\n");
	}
	else
	{
		m_cmdList->SetName(L"Command List");
		DebuggerPrintf("D12 Object Created: Command List\n");
	}

	//Create Fence and Fence Event
	DXCall(hr = device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
	if (!SUCCEEDED(hr)) goto _error;
	DebuggerPrintf("D12 Object Created: Fence\n");
	m_fenceEvent = CreateEventEx(nullptr, nullptr, 0, EVENT_ALL_ACCESS);
	assert(m_fenceEvent);
	return;

_error:
	Release();
	ERROR_AND_DIE("ERROR occurs when creating Commands.");

}

D12Command::~D12Command()
{
	assert(!m_cmdQueue && !m_cmdList && !m_fence);

}

//Wait for the current frame to be signaled and reset the command list/allocator

void D12Command::BeginFrame(ID3D12PipelineState* pipelineState /*= nullptr*/)
{
	CommandFrame& frame = m_cmdFrames[m_frameIndex];
	frame.Wait(m_fenceEvent, m_fence);
	HRESULT hr = frame.m_cmdAllocator->Reset();
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("ERROR occurs when reset Command Allocators.");
	}

	hr = m_cmdList->Reset(frame.m_cmdAllocator, pipelineState);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("ERROR occurs when reset Command list.");
	}
}

//Signal the fence with the new fence value.
void D12Command::EndFrame(D12Surface* surface)
{
	DXCall(m_cmdList->Close());
	ID3D12CommandList* const cmdLists[]{ m_cmdList };
	m_cmdQueue->ExecuteCommandLists(_countof(cmdLists), &cmdLists[0]);

	if (surface)
	{
		surface->Present();
	}


	m_fenceValue++;
	CommandFrame& frame = m_cmdFrames[m_frameIndex];
	frame.m_fenceValue = m_fenceValue;
	m_cmdQueue->Signal(m_fence, m_fenceValue);

	m_frameIndex = (m_frameIndex + 1) % FRAME_BUFFER_COUNT;
}



// void D12Command::ExecuteCmdLists()
// {
// 	DXCall(m_cmdList->Close());
// 	ID3D12CommandList* const cmdLists[]{ m_cmdList };
// 	m_cmdQueue->ExecuteCommandLists(_countof(cmdLists), &cmdLists[0]);
// 
// 	m_fenceValue++;
// 	CommandFrame& frame = m_cmdFrames[m_frameIndex];
// 	frame.m_fenceValue = m_fenceValue;
// 	m_cmdQueue->Signal(m_fence, m_fenceValue);
// 
// 	m_frameIndex = (m_frameIndex + 1) % FRAME_BUFFER_COUNT;
// 
// 	frame.Wait(m_fenceEvent, m_fence);
// 	HRESULT hr = frame.m_cmdAllocator->Reset();
// 	if (!SUCCEEDED(hr))
// 	{
// 		ERROR_AND_DIE("ERROR occurs when reset Command Allocators.");
// 	}
// 
// 	hr = m_cmdList->Reset(frame.m_cmdAllocator, nullptr);
// 	if (!SUCCEEDED(hr))
// 	{
// 		ERROR_AND_DIE("ERROR occurs when reset Command list.");
// 	}
// }

void D12Command::ExecuteCmdLists(ID3D12PipelineState* optionalState /*= nullptr*/)
{
	DXCall(m_cmdList->Close());
	ID3D12CommandList* const cmdLists[]{ m_cmdList };
	m_cmdQueue->ExecuteCommandLists(_countof(cmdLists), &cmdLists[0]);

	m_fenceValue++;
	CommandFrame& frame = m_cmdFrames[m_frameIndex];
	frame.m_fenceValue = m_fenceValue;
	m_cmdQueue->Signal(m_fence, m_fenceValue);

	m_frameIndex = (m_frameIndex + 1) % FRAME_BUFFER_COUNT;

	frame.Wait(m_fenceEvent, m_fence);
	HRESULT hr = frame.m_cmdAllocator->Reset();
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("ERROR occurs when reset Command Allocators.");
	}

	hr = m_cmdList->Reset(frame.m_cmdAllocator, optionalState);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("ERROR occurs when reset Command list.");
	}
}

void D12Command::Release()
{
	Flush();
	DX_SAFE_RELEASE(m_fence);
	m_fenceValue = 0;

	CloseHandle(m_fenceEvent);
	m_fenceEvent = nullptr;

	DX_SAFE_RELEASE(m_cmdQueue);
	DX_SAFE_RELEASE(m_cmdList);
	
	for (int i = 0; i < FRAME_BUFFER_COUNT; ++i)
	{
		m_cmdFrames[i].Release();

	}
}

void D12Command::Flush()
{
	for (int i = 0; i < FRAME_BUFFER_COUNT; ++i) 
	{
		m_cmdFrames[i].Wait(m_fenceEvent, m_fence);

	}
	m_frameIndex = 0;
}


#endif // ENABLE_D3D12