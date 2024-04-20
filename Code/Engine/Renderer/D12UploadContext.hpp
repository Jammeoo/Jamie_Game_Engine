#pragma once
#include "Engine/Renderer/D12CommonHeader.hpp"

#ifdef ENABLE_D3D12
class RendererD12;
#define  DX_SAFE_RELEASE(dxObject)				\
{												\
	if ((dxObject)!=nullptr)					\
	{											\
		(dxObject)->Release();					\
		(dxObject) = nullptr;					\
	}											\
}												\


constexpr u32 UPLOAD_FRAME_COUNT = 4;


// class D12Upload
// {
// public:
// 
// public:
// 	//D12Upload(D12UploadContext* uploadContext) :m_ownerUploadContext(uploadContext) 
// 	//{
// 	//
// 	//}
// 	//~D12Upload() = default;
// public:
// 	//ID3D12Fence1* GetUploadFence() { return m_uploadFence; }
// 	//HANDLE GetFenceEvent() { return m_fenceEvent; }
// private:
// 	//D12UploadContext* m_ownerUploadContext = nullptr;
// 	//UploadFrame m_uploadFrames[UPLOADFRAMECOUNT];
// 	//ID3D12CommandQueue* m_uploadCmdQueue = nullptr;
// 	//ID3D12Fence1* m_uploadFence = nullptr;
// 	//u64 m_uploadFenceValue = 0;
// 	//HANDLE m_fenceEvent{};
// };

class D12UploadContext
{
public:
	struct UploadFrame
	{
	public:
		UploadFrame() = default;
		//UploadFrame(D12UploadContext* uploadContext) :m_ownerUploadContext(m_ownerUploadContext) 
		//{
		//
		//}
		void WaitAndReset()
		{
			assert(m_ownerUploadContext);
			ID3D12Fence1* uploadFence = m_ownerUploadContext->GetUploadFence();
			HANDLE fenceEvent = m_ownerUploadContext->GetFenceEvent();
			assert(uploadFence && fenceEvent);
			if (uploadFence->GetCompletedValue() < m_fenceValue)
			{
				DXCall(uploadFence->SetEventOnCompletion(m_fenceValue, fenceEvent));
				WaitForSingleObject(fenceEvent, INFINITE);
			}
			DX_SAFE_RELEASE(m_uploadBuffer);
			m_cpuAddress = nullptr;
		}
		void Release()
		{
			WaitAndReset();
			DX_SAFE_RELEASE(m_cmdAllocator);
			DX_SAFE_RELEASE(m_cmdList);
		}
		bool IsReady()const { return m_uploadBuffer == nullptr; }
		void SetUploadContext(D12UploadContext* uploadContext) 
		{
			m_ownerUploadContext = uploadContext;
		}
		public:
		D12UploadContext* m_ownerUploadContext = nullptr;
		ID3D12CommandAllocator* m_cmdAllocator = nullptr;
		id3d12GraphicsCommandList* m_cmdList = nullptr;
		ID3D12Resource* m_uploadBuffer = nullptr;
		void* m_cpuAddress = nullptr;
		u64 m_fenceValue = 0;
	};
public:
	//D12UploadContext() = default;
	D12UploadContext(RendererD12* renderer);
	D12UploadContext(RendererD12* renderer, u32 alignedSize);
	DISABLE_COPY_AND_MOVE(D12UploadContext);
	~D12UploadContext();

	bool Initialize();
	void ShutDown();
	void EndUpload();

	bool InitFailed();

	u32 GetAvailableUploadFrame();
public:
	constexpr id3d12GraphicsCommandList* const GetCommandList()const { return m_cmdList; }
	constexpr ID3D12Resource* const GetUploadBuffer()const { return m_uploadBuffer; }
	constexpr void* const GetCPUAddress()const { return m_cpuAddress; }
	constexpr ID3D12Fence1* GetUploadFence() { return m_uploadFence; }
	constexpr HANDLE GetFenceEvent() { return m_fenceEvent; }
private:
	RendererD12* m_ownerRenderer = nullptr;
	id3d12GraphicsCommandList* m_cmdList = nullptr;
	ID3D12Resource* m_uploadBuffer = nullptr;
	void* m_cpuAddress = nullptr;
	u32 m_frameIndex = 0;

	//D12Upload* m_upload = nullptr;

	//for Upload
	UploadFrame m_uploadFrames[UPLOAD_FRAME_COUNT];
	ID3D12CommandQueue* m_uploadCmdQueue = nullptr;
	ID3D12Fence1* m_uploadFence = nullptr;
	u64 m_uploadFenceValue = U32_INVALID_ID;
	HANDLE m_fenceEvent{};
	std::mutex m_frameMutex;
	std::mutex m_queueMutex;

};

#endif // ENABLE_D3D12