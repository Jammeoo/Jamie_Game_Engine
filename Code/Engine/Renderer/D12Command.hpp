#pragma once
#include "Engine/Renderer/D12CommonHeader.hpp"
#include "Engine/Renderer/RendererD12.hpp"

#ifdef ENABLE_D3D12

class D12Surface;
struct CommandFrame
{
	ID3D12CommandAllocator* m_cmdAllocator = nullptr;
	unsigned int m_fenceValue = 0;
	void Release()
	{
		if ((m_cmdAllocator) != nullptr)
		{
			m_cmdAllocator->Release();
			m_fenceValue = 0;
			m_cmdAllocator = nullptr;
		}
	}
	void Wait(HANDLE fenceEvent,ID3D12Fence1* fence)
	{
		assert(fence && fenceEvent);
		// If the current fence value is still less than "fence_value"
		// then we know the GPU has not finished executing the command lists
		// since it has not reached the "_cmd_queue->Signal()" command

		if (fence->GetCompletedValue() < m_fenceValue) 
		{
			//This event is singled one the fence's current value equals "fence value"
			DXCall(fence->SetEventOnCompletion(m_fenceValue, fenceEvent));
			
			//Wait until the fence has triggered the event that its current value has reached "fence value"
			//Indicating that command queue has finished executing
			WaitForSingleObject(fenceEvent, INFINITE);
		}

	}
};

class D12Command
{
public:
	D12Command() = default;
	DISABLE_COPY_AND_MOVE(D12Command);
	explicit D12Command(id3d12Device* const device, D3D12_COMMAND_LIST_TYPE type, ID3D12PipelineState* optionalState = nullptr);
	//explicit D12Command(id3d12Device* const device, D3D12_COMMAND_LIST_TYPE typem);
	~D12Command();
	void BeginFrame(ID3D12PipelineState* pipelineState = nullptr);
	void EndFrame(D12Surface* surface);
	void ExecuteCmdLists(ID3D12PipelineState* optionalState = nullptr);
	void Release();
	void Flush();
public:
	ID3D12CommandQueue* const GetCommandQueue()const { return m_cmdQueue; }
	id3d12GraphicsCommandList* const GetCommandList()const { return m_cmdList; }
	unsigned int const GetFrameIndex() const { return m_frameIndex; }

private:

	ID3D12CommandQueue* m_cmdQueue = nullptr;
	id3d12GraphicsCommandList* m_cmdList = nullptr;
	CommandFrame m_cmdFrames[FRAME_BUFFER_COUNT] = { 0 };
	unsigned int m_frameIndex = 0;

	ID3D12Fence1* m_fence = nullptr;
	unsigned int m_fenceValue = 0;

	HANDLE m_fenceEvent = nullptr;
};

#endif // ENABLE_D3D12