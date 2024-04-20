#include "Engine/Renderer/D12Mesh.hpp"
#ifdef ENABLE_D3D12
#include "Engine/Renderer/D12Helpers.hpp"
#include "Engine/Renderer/RendererD12.hpp"
extern RendererD12* g_theRendererD12;

#define  DX_SAFE_RELEASE(dxObject)				\
{												\
	if ((dxObject)!=nullptr)					\
	{											\
		(dxObject)->Release();					\
		(dxObject) = nullptr;					\
	}											\
}												\

D12GPUMesh::D12GPUMesh(std::string const& name)
{
	LoadXMLFile(name.c_str());

	//OBJLoader::LoadOBJFilePNCU(m_filePath, m_fixedMatrix, m_vertexes, m_indexes);
	OBJLoader::LoadOBJFile(m_filePath, m_fixedMatrix, m_vertexes, m_indexes);
	assert(g_theRendererD12);
	D12Helpers* helper = new D12Helpers(g_theRendererD12);
	assert(helper);

	if (name == "Data/Models/Cube_v.xml")
	{
		m_vertexes[0].m_color = Rgba8::PINK;
		m_vertexes[3].m_color = Rgba8::PINK;
		m_vertexes[6].m_color = Rgba8::PINK;
		m_vertexes[9].m_color = Rgba8::PINK;
		m_vertexes[12].m_color = Rgba8::PINK;
		m_vertexes[15].m_color = Rgba8::PINK;
		m_vertexes[18].m_color = Rgba8::PINK;
		m_vertexes[21].m_color = Rgba8::PINK;
		m_vertexes[24].m_color = Rgba8::PINK;
		m_vertexes[27].m_color = Rgba8::PINK;
		m_vertexes[30].m_color = Rgba8::PINK;
		m_vertexes[33].m_color = Rgba8::PINK;
	}

	//Create Vertex buffer
	UINT indexUintSize = (UINT)sizeof(int);
	UINT indexBufferSize = indexUintSize * (UINT)m_indexes.size();
	m_indexMeshBuffer = helper->CreateBuffer(m_indexes.data(), (u32)indexBufferSize);

	//m_indexMeshBuffer->SetName(L"Index Buffer in size " + indexBufferSize);

	UINT vertexUintSize = (UINT)sizeof(Vertex_PCUTBN);
	UINT vertexBufferSize = vertexUintSize * (UINT)m_vertexes.size();
	m_vertexMeshBuffer = helper->CreateBuffer(m_vertexes.data(), (u32)vertexBufferSize/*,false,D3D12_RESOURCE_STATE_COMMON,D3D12_RESOURCE_FLAG_NONE,*/);

	//m_vertexMeshBuffer->SetName(L"Vertex Buffer in size " + vertexBufferSize);

}

D12GPUMesh::D12GPUMesh(std::vector<Vertex_PCUTBN>const& vertexBuffer)
{
	D12Helpers* helper = new D12Helpers(g_theRendererD12);
	assert(helper);

	for (int i = 0; i < (int)vertexBuffer.size(); i++) 
	{
		m_indexes.push_back(i);
		m_vertexes.push_back(vertexBuffer[i]);
	}


	//Create Vertex buffer
	UINT indexUintSize = (UINT)sizeof(int);
	UINT indexBufferSize = indexUintSize * (UINT)m_indexes.size();
	m_indexMeshBuffer = helper->CreateBuffer(m_indexes.data(), (u32)indexBufferSize);

	//m_indexMeshBuffer->SetName(L"Index Buffer in size " + indexBufferSize);

	UINT vertexUintSize = (UINT)sizeof(Vertex_PCUTBN);
	UINT vertexBufferSize = vertexUintSize * (UINT)m_vertexes.size();
	m_vertexMeshBuffer = helper->CreateBuffer(m_vertexes.data(), (u32)vertexBufferSize/*,false,D3D12_RESOURCE_STATE_COMMON,D3D12_RESOURCE_FLAG_NONE,*/);

}

D12GPUMesh::D12GPUMesh(std::vector<Vertex_PCUTBN>const& vertexBuffer, std::vector<unsigned int>const& indexBuffer)
{
	D12Helpers* helper = new D12Helpers(g_theRendererD12);
	assert(helper);

	for (int i = 0; i < (int)vertexBuffer.size(); i++)
	{
		m_vertexes.push_back(vertexBuffer[i]);
	}

	for (int i = 0; i < (int)indexBuffer.size(); i++)
	{
		m_indexes.push_back(i);
	}
	//Create Vertex buffer
	UINT indexUintSize = (UINT)sizeof(unsigned int);
	UINT indexBufferSize = indexUintSize * (UINT)m_indexes.size();
	m_indexMeshBuffer = helper->CreateBuffer(m_indexes.data(), (u32)indexBufferSize);

	//m_indexMeshBuffer->SetName(L"Index Buffer in size " + indexBufferSize);

	UINT vertexUintSize = (UINT)sizeof(Vertex_PCUTBN);
	UINT vertexBufferSize = vertexUintSize * (UINT)m_vertexes.size();
	m_vertexMeshBuffer = helper->CreateBuffer(m_vertexes.data(), (u32)vertexBufferSize/*,false,D3D12_RESOURCE_STATE_COMMON,D3D12_RESOURCE_FLAG_NONE,*/);

}

D12GPUMesh::~D12GPUMesh()
{
	g_theRendererD12->DeferredRelease(m_vertexMeshBuffer);
	g_theRendererD12->DeferredRelease(m_indexMeshBuffer);

	m_indexes.clear();
	m_vertexes.clear();
}

bool D12GPUMesh::LoadXMLFile(char const* xmlFilePath)
{
	XmlDocument modelXml;
	XmlResult result = modelXml.LoadFile(xmlFilePath);
	if (result != tinyxml2::XML_SUCCESS)
	{
		return false;
	}
	XmlElement* rootElement = modelXml.RootElement();

	m_name = ParseXmlAttribute(*rootElement, "name", m_name);
	m_filePath = ParseXmlAttribute(*rootElement, "path", m_filePath);
	std::string shaderPath = ParseXmlAttribute(*rootElement, "shader", shaderPath);
	m_shaderIndex = 0;//#ToDo: 
	//GUARANTEE_OR_DIE(m_shader, "failed to create a correct shader");

	XmlElement* transElement = rootElement->FirstChildElement();

	m_transform.x = ParseXmlAttribute(*transElement, "x", m_transform.x);
	m_transform.y = ParseXmlAttribute(*transElement, "y", m_transform.y);
	m_transform.z = ParseXmlAttribute(*transElement, "z", m_transform.z);
	m_transform.t = ParseXmlAttribute(*transElement, "t", m_transform.t);
	m_transform.scale = ParseXmlAttribute(*transElement, "scale", m_transform.scale);

	Mat44 fixMat44;
	fixMat44.SetIJKT3D(m_transform.x, m_transform.y, m_transform.z, m_transform.t);
	fixMat44.AppendScaleUniform3D(m_transform.scale);
	m_fixedMatrix = fixMat44;
	return true;
}
void D12GPUMesh::CreateMeshAndIndexView()
{

	UINT vertexUintSize = (UINT)sizeof(Vertex_PCUTBN);
	UINT vertexBufferSize = vertexUintSize * (UINT)m_vertexes.size();
	m_meshView = new MeshView();
	m_meshView->m_vertexBufferView.BufferLocation = m_vertexMeshBuffer->GetGPUVirtualAddress();
	m_meshView->m_vertexBufferView.SizeInBytes = vertexBufferSize;
	m_meshView->m_vertexBufferView.StrideInBytes = vertexUintSize;


	UINT indexUintSize = (UINT)sizeof(int);
	UINT indexBufferSize = indexUintSize * (UINT)m_indexes.size();
	m_meshView->m_indexBufferView.BufferLocation = m_indexMeshBuffer->GetGPUVirtualAddress();
	m_meshView->m_indexBufferView.SizeInBytes = indexBufferSize;
	m_meshView->m_indexBufferView.Format = DXGI_FORMAT_R32_UINT;
	m_indexCount = (UINT)m_indexes.size();
}




















#endif // ENABLE_D3D12