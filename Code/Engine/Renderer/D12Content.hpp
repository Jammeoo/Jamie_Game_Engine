#pragma once
#include "Engine/Core/EngineCommon.hpp"
#ifdef ENABLE_D3D12
#include "Engine/Renderer/D12CommonHeader.hpp"
#include "Engine/Renderer/D12Mesh.hpp"
#include "Engine/Renderer/D12Shader.hpp"
#include "Engine/Renderer/D12Resources.hpp"

struct OpaqueRootParameter {
	enum parameter : u32 {
		//global_shader_data,
		position_buffer,
		//srv_indices,
		//directional_lights,
		//cullable_lights,
		count
	};
};

enum PSOROOTSIG_ID : u32 {
	model_mesh = 0,

	count
};

struct ShaderFlags {
	enum flags : u32 {
		none = 0x0,
		vertex = 0x01,
		hull = 0x02,
		domain = 0x04,
		geometry = 0x08,
		pixel = 0x10,
		compute = 0x20,
		amplification = 0x40,
		mesh = 0x80,
	};
};

struct ModelList {
	enum id :u32
	{
		CUBE_V = 0,
		CUBE_VF = 1,
		CUBE_VFN = 2,
		CUBE_VFNT = 3,
		//TEDDY = 0,
		//TANK1 = 4,
		//COW = 4,
		COUNT
	};
};

class D12Content
{
public:
	D12Content() = default;
	//D12Content();
	~D12Content();
	bool InitializeMeshResource();
	void AddTriangleResource();
	void AddSphereModelResource();
	void Shutdown();
	void Add(std::string const& name);
	void Remove(u32 Index);
	void Render(D3D12_CPU_DESCRIPTOR_HANDLE targetRTV);
	void SetRootSigAnsPSO();

	bool CreateRootSignatureAndPSOForMesh();

	constexpr D3D12_ROOT_SIGNATURE_FLAGS GetRootSignatureFlags(ShaderFlags::flags flags);
	[[nodiscard]] ID3D12Resource* const GetDeepthStencilResource()const { return m_contentDepthBuffer->GetResource(); }
protected:
private:

	std::vector<MeshView*>					m_meshViews;
	std::vector<ID3D12Resource*>			m_meshVertexBuffers;
	std::vector<ID3D12Resource*>			m_meshIndexBuffers;
	std::mutex								m_meshMutex{};
	std::vector<ID3D12RootSignature*>		m_rootSignatures;
	std::vector<ID3D12PipelineState*>       m_pipelineStates;
	std::vector<u32> m_shaderIndexes;
public:
	std::vector<D12GPUMesh*>				m_gpuMeshList;
	D12DepthBuffer* m_contentDepthBuffer = nullptr;
	const std::string m_modelXMLPathPrefix = "Data/Models/";//"Engine/Renderer/Shaders/";
	std::string m_modelXMLNameList[ModelList::COUNT]{
		"Teddy.xml" ,
		//"Cow.xml",
		"Cube_v.xml",
		//"Cube_vf.xml",
		"Cube_vfn.xml",
		"Cube_vfnt.xml"
		//"Teddy.xml" 
		//"Cow.xml", 
	};

	

};



#endif // ENABLE_D3D12