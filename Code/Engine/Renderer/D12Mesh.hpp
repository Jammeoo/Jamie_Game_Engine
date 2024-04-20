#pragma once
#include "Engine/Core/EngineCommon.hpp"
#ifdef ENABLE_D3D12
#include "Engine/Renderer/D12CommonHeader.hpp"
#include "Engine/Renderer/D12Helpers.hpp"
//#include "Engine/Core/Vertex_PNCU.hpp"
#include "Engine/Core/Vertex_PCUTBN.hpp"
#include "Engine/Core/OBJLoader.hpp"
#include <vector>
class RendererD12;

struct TransformFromXML
{
	Vec3 x = Vec3(0.f, 0.f, 0.f);
	Vec3 y = Vec3(0.f, 0.f, 0.f);
	Vec3 z = Vec3(0.f, 0.f, 0.f);
	Vec3 t = Vec3(0.f, 0.f, 0.f);
	float scale = 1.f;
};

struct MeshView
{
	D3D12_VERTEX_BUFFER_VIEW    m_vertexBufferView{};
	D3D12_INDEX_BUFFER_VIEW     m_indexBufferView{};
	D3D_PRIMITIVE_TOPOLOGY      m_primitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
};

class D12GPUMesh
{
public:
	D12GPUMesh(std::string const& name);
	D12GPUMesh(std::vector<Vertex_PCUTBN>const& vertexBuffer);
	D12GPUMesh(std::vector<Vertex_PCUTBN>const& vertexBuffer, std::vector<unsigned int>const& indexBuffer);
	~D12GPUMesh();
	//void Render()const;
	bool LoadXMLFile(char const* xmlFilePath);//Get from XML file
	void CreateMeshAndIndexView();
protected:
public:
	std::string m_name = "";
	std::string m_filePath = "";
	Mat44 m_fixedMatrix;
	TransformFromXML m_transform;
	u32 m_shaderIndex = U32_INVALID_ID;

	std::vector<Vertex_PCUTBN> m_vertexes;
	std::vector<unsigned int> m_indexes;

	MeshView* m_meshView = nullptr;
	ID3D12Resource* m_vertexMeshBuffer = nullptr;
	ID3D12Resource* m_indexMeshBuffer = nullptr;
	UINT m_indexCount = 0;

};


#endif // ENABLE_D3D12