#pragma once
#define ENGINE_DEBUG_RENDER
#define WIN32_LEAN_AND_MEAN							// Always #define this before #including <windows.h>
#pragma comment (lib,"d3d11.lib")
#pragma comment (lib,"dxgi.lib")
#pragma comment (lib,"d3dcompiler.lib")

#include "Engine/Renderer/Window.hpp"
#include <d3d11.h>
#include <dxgi.h>
#include <d3dcompiler.h>

#if defined(OPAQUE)
#undef OPAQUE
#endif

#if defined(ENGINE_DEBUG_RENDER)
#include <dxgidebug.h>
#pragma comment (lib,"dxguid.lib")
#endif
#include "Game/EngineBuildPreferences.hpp"
#include"Engine/Core/Rgba8.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/Window.hpp"
#include"Engine/Renderer/Camera.hpp"
#include"Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/Vertex_PNCU.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Core/Vertex_PCUTBN.hpp"
#include <vector>

class Shader;
class VertexBuffer;
class IndexBuffer;
class ConstantBuffer;
class Image;
class RenderTextureClass;
struct Vec4;
enum class BlendMode
{
	ALPHA,
	ADDITIVE,
	OPAQUE,
	COUNT
};

enum class SamplerMode 
{
	POINT_CLAMP,
	BILINEAR_WRAP,
	COUNT
};

enum class RasterizerMode 
{
	SOLID_CULL_NONE,
	SOLID_CULL_BACK,
	SOLID_CULL_FRONT,
	WIREFRAME_CULL_NONE,
	WIREFRAME_CULL_BACK,
	COUNT
};

enum class DepthMode
{
	DISABLED,
	ENABLED,
	COUNT
}; 

struct RendererConfig
{
	Window* m_window = nullptr;
};

// struct DepthTexture :public Texture
// {
// 
// 	ID3D11DepthStencilView* m_depthStencilView = nullptr;
// 	ID3D11Texture2D* m_baseTexture = nullptr;
// 	ID3D11ShaderResourceView* m_shaderRecourceView = nullptr;
// 	//ID3D11Texture2D* m_texture = nullptr;
// };

enum class VertexType 
{
	Vertex_PCU,
	//Vertex_PNCU,
	Vertex_PCUTBN
};

//--------------------------------------------------------------------------------------------------
enum class PrimitiveTopology
{
	D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED = 0,
	D3D11_PRIMITIVE_TOPOLOGY_POINTLIST = 1,
	D3D11_PRIMITIVE_TOPOLOGY_LINELIST = 2,
	D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP = 3,
	D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST = 4,
	D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP = 5,
	D3D11_PRIMITIVE_TOPOLOGY_LINELIST_ADJ = 10,
	D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ = 11,
	D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ = 12,
	D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ = 13,
};


class Renderer {
public:
	Renderer(RendererConfig const& config);
	~Renderer();
	
	void Startup();
	void BeginFrame();
	void EndFrame();
	void Shutdown();

	void ClearScreen(const Rgba8& clearColor);
	void BeginCamera(const Camera& camera);
	void EndCamera(const Camera& camera);
	void DrawVertexArray(int numVertexes, const Vertex_PCU* vertexes, PrimitiveTopology type = PrimitiveTopology::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	void DrawVertexPNCUArray(int numVertexes, const Vertex_PCUTBN* vertexes);
	//void DrawIndexArray(int numVertexes, int numIndexes, const int* indexes, const Vertex_PCU* vertexes);

	//void SetBlendMode(BlendMode blendMode);

	void CreateD3DStateObjects();
	//To Implement Texture
	void BindTexture(const Texture* texture);
	void BindTextures(const Texture* texture1, const Texture* texture2, const Texture* texture3);
	void BindFourTextures(const Texture* texture0, const Texture* texture1, const Texture* texture2, const Texture* texture3);
	void BindTexturesList(unsigned int Num, std::vector<Texture*>const textures);

	Texture* CreateOrGetTextureFromFile(char const* imageFilePath);

	void BindShader(Shader* shader);
	Shader* CreateShader(char const* shaderName, char const* shaderSource, VertexType type = VertexType::Vertex_PCU);
	Shader* CreateShader(char const* shaderName, VertexType type = VertexType::Vertex_PCU);
	bool CompileShaderToByteCode(std::vector<unsigned char>& outByteCode,
		char const* name, char const* source, char const* entryPoint, char const* target);
	void SetModelConstants(const Mat44& modelMatrix = Mat44(), const Rgba8& modelColor = Rgba8::WHITE);
	void SetLightConstants(Vec3 const& sunDirection, float sunIntensity, float ambientIntensity, Vec3 const& worldEyePosi = Vec3::WORLD_ORIGIN, int normalMode = 0, int specularMode = 0, float specularIntensity = 0.f, float specularPower = 0.f);
	void SetGamePlayConstants(Vec3 const& cameraWorldPosition, Vec4 const& skyColor);
	void SetScreenSizeBuffer(float screenWidth, float screenHeight);

	BitmapFont* CreateOrGetBitmapFont(char const* imageFilePath);

	void SetStateIfChanged();
	void SetBlendMode(BlendMode blendMode);
	void SetSamplerMode(SamplerMode samplerMode);
	void SetRasterizerMode(RasterizerMode rasterizerMode);
	void SetDepthMode(DepthMode depthMode);

	Shader* GetShaderByName(std::string const& name);
	
protected:
	Texture* CreateTextureFromFile(char const* imageFilePath);
	Texture* CreateTextureFromImage(const Image& image);
	Texture* GetTextureForFileName(char const* imageFilePath);
public:
	VertexBuffer* CreateVertexBuffer(const size_t size);
	void CopyCPUToGPU(const void* data, size_t size, VertexBuffer*& vbo);
	void BindVertexBuffer(VertexBuffer* vbo, PrimitiveTopology type = PrimitiveTopology::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	VertexBuffer* CreateVertexBuffer(const size_t size, unsigned int stride);
	void CopyCPUToGPU(const void* data, size_t size, VertexBuffer*& vbo, unsigned int stride);
	void BindPNCUVertexBuffer(VertexBuffer* vbo, PrimitiveTopology type = PrimitiveTopology::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	IndexBuffer* CreateIndexBuffer(const size_t size);
	void CopyCPUToGPU(const void* data, size_t size, IndexBuffer*& ibo);
	void BindIndexBuffer(IndexBuffer* ibo);

	ConstantBuffer* CreateConstantBuffer(const size_t size);
	void CopyCPUToGPU(const void* data, size_t size, ConstantBuffer*& cbo);
	void BindConstantBuffer(int slot, ConstantBuffer* vbo);

	void DrawVertexBuffer(VertexBuffer* vbo, int vertexCount, int vertexOffset = 0, PrimitiveTopology type = PrimitiveTopology::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	void DrawVertexPNCUBuffer(VertexBuffer* vbo, int vertexCount, int vertexOffset = 0, PrimitiveTopology type = PrimitiveTopology::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	void DrawIndexBuffer(IndexBuffer* ibo, int indexCount, int vertexOffset = 0);
	void DrawVertexAndIndexBuffer(VertexBuffer* vbo, IndexBuffer* ibo, int indexCount, int indexOffset = 0, int vertexOffset = 0, PrimitiveTopology type = PrimitiveTopology::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	Texture* CreateTextureFromNothing(std::string const& name, IntVec2 const& dimensions, ID3D11Texture2D* texture, ID3D11ShaderResourceView* shaderRecourceView);
	void SetBackBufferRenderTarget()const;
	void UnbindRenderTarget()const;
	ID3D11Device* GetDevice()const { return m_device; }
	ID3D11DeviceContext* GetDeviceContext()const { return m_deviceContext; }
	ID3D11DepthStencilView* GetDepthStencilView()const { return m_depthStencilView; }

	//Create depth
	Texture* CreateDepthTexture();
	void ClearDepthTexture(Texture* depthTexture, float clearColor = 1.f);
	void BindDepthTextureWhileSetRenderTarget(Texture* depthTex, ID3D11RenderTargetView* renderTargetView = nullptr);
	//
	bool InitializeRnederTarget(RenderTextureClass* renderTargetToIntialize, int textureWidth, int textureHeight);

protected:
	BitmapFont* CreateBitmapFont(char const* fontFilePathNameNoExtension);
	BitmapFont* GetBitmapFontForFileName(char const* imageFilePath);

	

	RendererConfig const& GetConfig()const 	{		return m_config;	}

	RendererConfig		m_config;
private:
	std::vector<Texture*> m_loadedTextures;
	std::vector<BitmapFont*> m_loadedBitMapFond;
	//for internal member functions
	void* m_dxgiDebugModule = nullptr;
	void* m_dxgiDebug = nullptr;
protected:
	ID3D11Device*			m_device = nullptr;
	ID3D11DeviceContext*	m_deviceContext = nullptr;
	IDXGISwapChain*			m_swapChain = nullptr;
	ID3D11RenderTargetView* m_renderTargetView = nullptr;
	ID3D11RasterizerState*	m_rasterizerState = nullptr;

	std::vector<Shader*> m_loadedShaders;
	Shader const* m_currentShader = nullptr;
	Shader* m_defaultShader = nullptr;
	VertexBuffer* m_immediateVBO = nullptr;
	VertexBuffer* m_immediateVBOPNCU = nullptr;
	ConstantBuffer* m_cameraCBO = nullptr;
	ConstantBuffer* m_modelCBO = nullptr;
	ConstantBuffer* m_lightCBO = nullptr;
	
	//For simple miner
	ConstantBuffer* m_gameCBO = nullptr;

	ConstantBuffer* m_screenSizeCBP = nullptr;

	ID3D11BlendState* m_blendState = nullptr;

	const Texture* m_defaultTexture = nullptr;

	ID3D11SamplerState* m_samplerState = nullptr;

	ID3D11DepthStencilState* m_depthStencilState = nullptr;
	ID3D11DepthStencilView* m_depthStencilView = nullptr;
	ID3D11Texture2D* m_depthStencilTexture = nullptr;

	BlendMode					m_desiredBlendMode = BlendMode::ALPHA;
	SamplerMode					m_desiredSamplerMode = SamplerMode::POINT_CLAMP;
	RasterizerMode				m_desiredRasterizerMode = RasterizerMode::SOLID_CULL_BACK;
	DepthMode					m_desiredDepthmode = DepthMode::ENABLED;

	ID3D11BlendState* m_blendStates[(int)(BlendMode::COUNT)] = { };
	ID3D11SamplerState* m_samplerStates[(int)(SamplerMode::COUNT)] = { };
	ID3D11RasterizerState* m_rasterizerStates[(int)(RasterizerMode::COUNT)] = { };
	ID3D11DepthStencilState* m_depthStencilStates[(int)(DepthMode::COUNT)] = { };


	//RenderScreen to Texture
	//RenderTextureClass* m_renderScreenTexture = nullptr;//asdf

};