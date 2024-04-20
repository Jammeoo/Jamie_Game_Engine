#pragma once
#include "Engine/Renderer/D12CommonHeader.hpp"
#ifdef ENABLE_D3D12

class RendererD12;
class D12PostProcess
{
public:
	D12PostProcess(RendererD12* renderer):m_ownerRenderer(renderer)
	{

	}
	
	
	bool Initialize();
	void Shutdown();

	bool CreateFXPosAndRootSignature();
	void PostProcess(id3d12GraphicsCommandList* cmdList, D3D12_CPU_DESCRIPTOR_HANDLE targetRTV);
private:
	RendererD12* m_ownerRenderer = nullptr;
	ID3D12RootSignature* m_fxRootSig = nullptr;
	ID3D12PipelineState* m_fxPSO = nullptr;
};
#endif // ENABLE_D3D12