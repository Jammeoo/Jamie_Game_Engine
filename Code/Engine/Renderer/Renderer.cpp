#pragma once
#define D3D_COMPILE_STANDARD_FILE_INCLUDE ((ID3DInclude*)(UINT_PTR)1)				

#define  DX_SAFE_RELEASE(dxObject)				\
{												\
	if ((dxObject)!=nullptr)					\
	{											\
		(dxObject)->Release();					\
		(dxObject) = nullptr;					\
	}											\
}												\

#include<windows.h>
#include "Engine/Renderer/VertexBuffer.hpp"
#include "Engine/Renderer/IndexBuffer.hpp"
#include "Engine/Renderer/DefaultShader.hpp"
#include "Engine/Renderer/SpriteLit.hpp"
#include "Engine/Renderer/ConstantBuffer.hpp"
#include "Engine/Renderer/Shader.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/Image.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Core/FileUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Renderer/RenderTextureClass.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Math/Vec4.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "ThirdParty/stb/stb_image.h"

extern BitmapFont* g_testFont;
extern Window* g_theWindow;

//CAMERA CONSTANTS

struct CameraConstants 
{
	Mat44 ProjectionMatrix;
	Mat44 ViewMatrix;
};

struct ModelConstants 
{
	Mat44 ModelMatrix;
	float ModeColor[4];

};
struct LightConstants 
{
	Vec3 SunDirection = Vec3(2.f, 1.f, -1.f);
	float SunIntensity = 0.85f;
	float AmbientIntensity = 0.35f;
	Vec3 WorldEyePosition = Vec3(0.f, 0.f, 0.f);
	int NormalMode = 0;
	int SpecularMode;
	float SpecularIntensity = 0.f;
	float SpecularPower = 0.f;
};

//struct LightConstants
//{
//	Vec3 SunDirection = Vec3(2.f, 1.f, -1.f);
//	float SunIntensity = 0.85f;
//	float AmbientIntensity = 0.35f;
//	Vec3 SunPadding = Vec3(0.f, 0.f, 0.f);
//};

struct GameConstants
{
	float		b_camWorldPos[4];										// Used for fog thickness calculations, specular lighting, etc.
	float		b_skyColor[4] = { 1.f, 1.f, 0.f,1.f };				// Also used as fog color
	float		b_outdoorLightColor[4] = { 1.f, 1.f, 1.f,1.f };			// Used for outdoor lighting exposure
	float		b_indoorLightColor[4] = { 1.f, 0.8f, 0.6f,1.f };		// Used for outdoor lighting exposure
	float		b_fogStartDist = 125.f;									// Fog has zero opacity at or before this distance
	float		b_fogEndDist = 300.f;									// Fog has maximum opacity at or beyond this distance
	float		b_fogMaxAlpha = 0.8f;									// At and beyond fogEndDist, fog gets this much opacity
	float		b_time = 0.f;											// CBO structs and members must be 16B-aligned and 16B-sized!!
	// REMEMBER: CBO structs must have size multiple of 16B, and <16B members cannot straddle 16B boundaries!!
};

struct ScreenSizeConstants 
{
	float screenWidth = 0.1f;
	float screenHeight = 0.1f;
	Vec2 screenPading = Vec2(0.f, 0.f);
};

static const int k_lightConstantsSlot = 1;
static const int k_cameraConstantsSlot = 2;
static const int k_modelConstantsSlot = 3;
static const int k_gameConstantsSlot = 8;
static const int k_screenSizeConstantsSlot = 4;
//static const int k_pointLightConstantsSlot = 5;
Renderer::Renderer(RendererConfig const& config)
	: m_config(config)
{
	
}

Renderer::~Renderer() {

}
void Renderer::Startup() 
{

#if defined(ENGINE_DEBUG_RENDER)
	m_dxgiDebugModule = (void*)::LoadLibraryA("dxgidebug.dll");
	if (m_dxgiDebugModule == nullptr)
	{
		ERROR_AND_DIE("Could not load dxgidebug.dll");
	}


	typedef HRESULT(WINAPI* GetDebugModuleCB)(REFIID, void**);
	((GetDebugModuleCB)::GetProcAddress((HMODULE)m_dxgiDebugModule, "DXGIGetDebugInterface"))(__uuidof(IDXGIDebug), &m_dxgiDebug);
	if (m_dxgiDebug == nullptr)
	{
		ERROR_AND_DIE("Could not load debug module");
	}
#endif

	DXGI_SWAP_CHAIN_DESC m_swapDevice = { 0 };
	m_swapDevice.BufferDesc.Width = m_config.m_window->GetClientDimensions().x;
	m_swapDevice.BufferDesc.Height = m_config.m_window->GetClientDimensions().y;
	m_swapDevice.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	m_swapDevice.SampleDesc.Count = 1;
	m_swapDevice.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	m_swapDevice.BufferCount = 2;
	m_swapDevice.OutputWindow = (HWND)(m_config.m_window->GetHwnd());
	m_swapDevice.Windowed = true;
	m_swapDevice.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

	HRESULT hr;
	unsigned int deviceFlags = 0;
#if defined(ENGINE_DEBUG_RENDER)
	deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
	hr = D3D11CreateDeviceAndSwapChain(NULL,
		D3D_DRIVER_TYPE_HARDWARE, NULL,
		deviceFlags,
		NULL, NULL, D3D11_SDK_VERSION,
		&m_swapDevice,
		&m_swapChain,
		&m_device,
		NULL,
		&m_deviceContext);

	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("Could not create D3D 11 device and swap chain.");
	}

	//Get back buffer texture
	ID3D11Texture2D* backBuffer;
	hr = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("Could not get swap chain buffer.");
	}
	hr = m_device->CreateRenderTargetView(backBuffer, NULL, &m_renderTargetView);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("Could not create render target view for swap chain buffer.");
	}

	backBuffer->Release();

	m_defaultShader = CreateShader("Data/Shaders/Default");

	//CreateShader("Data/Shaders/World");
	//Shader* litShader = CreateShader("Data/Shaders/SpriteLit");
	//Shader* spriteShader = CreateShader("Data/Shaders/Sprite");
	CreateShader("Data/Shaders/Sprite");

	BindShader(m_defaultShader);
	

	//VertexBuffer vertexBuffer = VertexBuffer(sizeof(Vertex_PNCU), sizeof(Vertex_PNCU));
	m_immediateVBO = CreateVertexBuffer(sizeof(Vertex_PCU));
	m_immediateVBOPNCU = CreateVertexBuffer(sizeof(Vertex_PNCU), sizeof(Vertex_PNCU));
	ConstantBuffer constantBuffer = ConstantBuffer(sizeof(Vertex_PCU));
/*	size_t sizeOfPCU16 = sizeof(Vertex_PCU) + 8;*/

	m_cameraCBO = CreateConstantBuffer(sizeof(CameraConstants));
	m_modelCBO = CreateConstantBuffer(sizeof(ModelConstants));
	m_lightCBO = CreateConstantBuffer(sizeof(LightConstants));
	m_gameCBO = CreateConstantBuffer(sizeof(GameConstants));
	m_screenSizeCBP = CreateConstantBuffer(sizeof(ScreenSizeConstants));

	SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
	SetBlendMode(BlendMode::ALPHA);
	
	Image* imageForDefaultTexture = new Image(IntVec2(1, 1), Rgba8::WHITE);
	m_defaultTexture = CreateTextureFromImage(*imageForDefaultTexture);
	BindTexture(m_defaultTexture);

	SetSamplerMode(SamplerMode::POINT_CLAMP);
	//Create and set a depth stencil view and state
	D3D11_TEXTURE2D_DESC texture2DDesc = { 0 };
	texture2DDesc.Width = m_config.m_window->GetClientDimensions().x;
	texture2DDesc.Height = m_config.m_window->GetClientDimensions().y;
	texture2DDesc.MipLevels = 1;
	texture2DDesc.ArraySize = 1;
	texture2DDesc.Usage = D3D11_USAGE_DEFAULT;
	texture2DDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	texture2DDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	texture2DDesc.SampleDesc.Count = 1;

	m_device->CreateTexture2D(&texture2DDesc, nullptr, &m_depthStencilTexture);
	m_device->CreateDepthStencilView(m_depthStencilTexture, nullptr, &m_depthStencilView);
	//Create and set a depth stencil state
// 	D3D11_DEPTH_STENCIL_DESC depthStencilDesc = { 0 };
// 	depthStencilDesc.DepthEnable = true;
// 	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
// 	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
// 
// 	m_device->CreateDepthStencilState(&depthStencilDesc, &m_depthStencilState);
// 
// 	m_deviceContext->OMSetDepthStencilState(m_depthStencilState, 0);

	CreateD3DStateObjects();
}
void Renderer::BeginFrame() 
{
	//Put it to beginFrame
	m_deviceContext->OMSetRenderTargets(1, &m_renderTargetView, m_depthStencilView);		//beginFrame
}
void Renderer::EndFrame() 
{
	//Presenting Put it to End Frame
	HRESULT hr;
	hr = m_swapChain->Present(0, 0);
	if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
	{
		ERROR_AND_DIE("Device has been lost, application will be terminated.");
	}
}
void Renderer::Shutdown() {


	for (int blendIndex = 0; blendIndex < (int)BlendMode::COUNT; blendIndex++) 
	{
		DX_SAFE_RELEASE(m_blendStates[blendIndex]);
	}
	for (int samplerIndex = 0; samplerIndex < (int)SamplerMode::COUNT; samplerIndex++)
	{
		DX_SAFE_RELEASE(m_samplerStates[samplerIndex]);
	}
	for (int rasterizerIndex = 0; rasterizerIndex < (int)RasterizerMode::COUNT; rasterizerIndex++)
	{
		DX_SAFE_RELEASE(m_rasterizerStates[rasterizerIndex]);
	}



	m_blendState = nullptr;
	m_samplerState = nullptr;
	m_rasterizerState = nullptr;
	m_depthStencilState = nullptr;


	DX_SAFE_RELEASE(m_renderTargetView);
	DX_SAFE_RELEASE(m_swapChain);
	DX_SAFE_RELEASE(m_deviceContext);
	DX_SAFE_RELEASE(m_device);

	DX_SAFE_RELEASE(m_depthStencilState);
	for (int depthIndex = 0; depthIndex < (int)DepthMode::COUNT; depthIndex++)
	{
		DX_SAFE_RELEASE(m_depthStencilStates[depthIndex]);
	}

	DX_SAFE_RELEASE(m_depthStencilView);
	DX_SAFE_RELEASE(m_depthStencilTexture);
	for (int shaderIndex = 0; shaderIndex < m_loadedShaders.size(); shaderIndex++) 
	{
		delete m_loadedShaders[shaderIndex];
		m_loadedShaders[shaderIndex] = nullptr;
	}
	for (int fontIndex = 0; fontIndex < m_loadedBitMapFond.size(); fontIndex++)
	{
		delete m_loadedBitMapFond[fontIndex];
		m_loadedBitMapFond[fontIndex] = nullptr;
	}
	
	m_currentShader = nullptr;

	delete m_defaultTexture;
	m_defaultTexture = nullptr;

	delete m_immediateVBO;
	m_immediateVBO = nullptr;

	delete m_immediateVBOPNCU;
	m_immediateVBOPNCU = nullptr;

	delete m_cameraCBO;
	m_cameraCBO = nullptr;

	delete m_modelCBO;
	m_modelCBO = nullptr;

	delete m_lightCBO;
	m_lightCBO = nullptr;

	delete m_gameCBO;
	m_gameCBO = nullptr;

	delete m_screenSizeCBP;
	m_screenSizeCBP = nullptr;

	for (int textureIndex = 0; textureIndex < m_loadedTextures.size(); textureIndex++)
	{
		delete m_loadedTextures[textureIndex];
		m_loadedTextures[textureIndex] = nullptr;
	}

	delete m_cameraCBO;
	m_cameraCBO = nullptr;
// 	if (m_renderScreenTexture) 
// 	{
// 		m_renderScreenTexture->Shutdown();
// 		delete m_renderScreenTexture;
// 		m_renderScreenTexture = nullptr;
// 	}

#if defined(ENGINE_DEBUG_RENDER)
	((IDXGIDebug*)m_dxgiDebug)->ReportLiveObjects(DXGI_DEBUG_ALL, (DXGI_DEBUG_RLO_FLAGS)(DXGI_DEBUG_RLO_DETAIL | DXGI_DEBUG_RLO_IGNORE_INTERNAL));
	((IDXGIDebug*)m_dxgiDebug)->Release();
	m_dxgiDebug = nullptr;

	::FreeLibrary((HMODULE)m_dxgiDebugModule);
	m_dxgiDebugModule = nullptr;
#endif
}
void Renderer::ClearScreen(const Rgba8& clearColor) {
	float array[4] = { 0.f };
	clearColor.GetAsFloats(array);
	m_deviceContext->ClearRenderTargetView(m_renderTargetView, array);
	m_deviceContext->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1, 0);
}

void Renderer::BeginCamera(const Camera& camera)
{
	//Set the viewPort
	D3D11_VIEWPORT viewport = { 0 };
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = (float)g_theWindow->GetClientDimensions().x;
	viewport.Height = (float)g_theWindow->GetClientDimensions().y;
	viewport.MinDepth = 0;
	viewport.MaxDepth = 1;
	
	m_deviceContext->RSSetViewports(1, &viewport);

	CameraConstants localCameraConstants = {};
	localCameraConstants.ProjectionMatrix = camera.GetProjectionMatrix();
	localCameraConstants.ViewMatrix = camera.GetViewMatrix();
	size_t size = (size_t)sizeof(CameraConstants);
	CopyCPUToGPU(&localCameraConstants, size, m_cameraCBO);
	BindConstantBuffer(k_cameraConstantsSlot, m_cameraCBO);
}

void Renderer::EndCamera(const Camera& camera)
{
	UNUSED(camera);

}


void Renderer::DrawVertexArray(int numVertexes, const Vertex_PCU* vertexes, PrimitiveTopology type /*= PrimitiveTopology::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST*/)
{
	size_t size = numVertexes * sizeof(Vertex_PCU);
	CopyCPUToGPU(vertexes, size, m_immediateVBO);
	DrawVertexBuffer(m_immediateVBO, numVertexes, 0, type);
}

void Renderer::DrawVertexPNCUArray(int numVertexes, const Vertex_PCUTBN* vertexes)
{
	size_t size = numVertexes * sizeof(Vertex_PCUTBN);
	CopyCPUToGPU(vertexes, size, m_immediateVBOPNCU, sizeof(Vertex_PCUTBN));
	DrawVertexPNCUBuffer(m_immediateVBOPNCU, numVertexes, 0);
}


void Renderer::CreateD3DStateObjects()
{
	D3D11_BLEND_DESC blendModeDesc = { 0 };
	blendModeDesc.RenderTarget[0].BlendEnable = true;
	blendModeDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendModeDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendModeDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
	blendModeDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendModeDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;


	blendModeDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	blendModeDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;
	HRESULT hr = m_device->CreateBlendState(&blendModeDesc, &m_blendStates[(int)BlendMode::OPAQUE]);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("Could not create blend mode OPAQUE.");
	}


	blendModeDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendModeDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	hr = m_device->CreateBlendState(&blendModeDesc, &m_blendStates[(int)BlendMode::ALPHA]);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("Could not create blend mode ALPHA.");
	}

	blendModeDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	blendModeDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	hr = m_device->CreateBlendState(&blendModeDesc, &m_blendStates[(int)BlendMode::ADDITIVE]);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("Could not create blend mode ADDITIVE.");
	}
	
	//////////////////////////////////////////////////////////////////////////

	D3D11_SAMPLER_DESC samplerDesc = { };
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;


	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;

	hr = m_device->CreateSamplerState(&samplerDesc, &m_samplerStates[(int)SamplerMode::POINT_CLAMP]);

	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("Could not create sampler mode POINT_CLAMP.");
	}

	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	hr = m_device->CreateSamplerState(&samplerDesc, &m_samplerStates[(int)SamplerMode::BILINEAR_WRAP]);

	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("Could not create sampler mode BILINEAR_WRAP.");
	}

	D3D11_RASTERIZER_DESC rasterizerDesc = {};
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.CullMode = D3D11_CULL_BACK;
	rasterizerDesc.FrontCounterClockwise = true;
	rasterizerDesc.DepthClipEnable = true;
	rasterizerDesc.AntialiasedLineEnable = true;


	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.CullMode = D3D11_CULL_NONE;
	
	hr = m_device->CreateRasterizerState(&rasterizerDesc, &m_rasterizerStates[(int)RasterizerMode::SOLID_CULL_NONE]);

	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("Could not Create Rasterizer State SOLID_CULL_NONE.");
	}

	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.CullMode = D3D11_CULL_BACK;

	hr = m_device->CreateRasterizerState(&rasterizerDesc, &m_rasterizerStates[(int)RasterizerMode::SOLID_CULL_BACK]);

	rasterizerDesc.CullMode = D3D11_CULL_FRONT;

	hr = m_device->CreateRasterizerState(&rasterizerDesc, &m_rasterizerStates[(int)RasterizerMode::SOLID_CULL_FRONT]);

	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("Could not Create Rasterizer State SOLID_CULL_BACK.");
	}

	rasterizerDesc.FillMode = D3D11_FILL_WIREFRAME;
	rasterizerDesc.CullMode = D3D11_CULL_NONE;

	hr = m_device->CreateRasterizerState(&rasterizerDesc, &m_rasterizerStates[(int)RasterizerMode::WIREFRAME_CULL_NONE]);

	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("Could not Create Rasterizer State WIREFRAME_CULL_NONE.");
	}

	rasterizerDesc.FillMode = D3D11_FILL_WIREFRAME;
	rasterizerDesc.CullMode = D3D11_CULL_BACK;

	hr = m_device->CreateRasterizerState(&rasterizerDesc, &m_rasterizerStates[(int)RasterizerMode::WIREFRAME_CULL_BACK]);

	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("Could not Create Rasterizer State WIREFRAME_CULL_BACK.");
	}

	//Create depth stencil state
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc = { 0 };
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_ALWAYS;

	hr = m_device->CreateDepthStencilState(&depthStencilDesc, &m_depthStencilStates[(int)(DepthMode::DISABLED)]);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("Could not Create depth stencil state DISABLED.");
	}

	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;

	hr = m_device->CreateDepthStencilState(&depthStencilDesc, &m_depthStencilStates[(int)(DepthMode::ENABLED)]);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("Could not Create depth stencil state DISABLED.");
	}
}


Texture* Renderer::CreateOrGetTextureFromFile(char const* imageFilePath)
{
	// See if we already have this texture previously loaded
	Texture* existingTexture = GetTextureForFileName(imageFilePath);
	if (existingTexture)
	{
		return existingTexture;
	}
	// Never seen this texture before!  Let's load it.
	Texture* newTexture = CreateTextureFromFile(imageFilePath);
	m_loadedTextures.push_back(newTexture);
	return newTexture;
}

void Renderer::BindShader(Shader* shader)
{
	if (shader == nullptr) 
	{
		shader = m_defaultShader;
	}

	m_deviceContext->IASetInputLayout(shader->m_inputLayout);
	m_deviceContext->VSSetShader(shader->m_vertexShader, 0, 0);
	m_deviceContext->PSSetShader(shader->m_pixelShader, 0, 0);
	m_currentShader = shader;
}

Shader* Renderer::CreateShader(char const* shaderName, VertexType type /*= VertexType::Vertex_PCU*/)
{
	std::string shaderFullName, shaderInString;
	shaderFullName = shaderName;
	shaderFullName.append(".hlsl");
	FileReadToString(shaderInString, shaderFullName);//add address of that file like Data/test.bin
	Shader* shader = CreateShader(shaderFullName.c_str(), shaderInString.c_str(), type);
	return shader;
}

Shader* Renderer::CreateShader(char const* shaderName, char const* shaderSource, VertexType type /*= VertexType::Vertex_PCU*/)
{
	HRESULT hr;
	ShaderConfig shaderConfig;
	shaderConfig.m_name = shaderName;
	Shader* shader = new Shader(shaderConfig);
	std::vector <unsigned char> vertexByteCode;
	bool isVertexShaderCompiled = CompileShaderToByteCode(vertexByteCode, shaderName, shaderSource, shaderConfig.m_vertexEntryPoint.c_str(), "vs_5_0");
	if (isVertexShaderCompiled == false) {
		ERROR_AND_DIE("Could not compile, Vertex shader");
	}
	hr = m_device->CreateVertexShader(vertexByteCode.data(), vertexByteCode.size(), NULL, &(shader->m_vertexShader));
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("Could not Create Vertex Shader.");
	}
	std::vector <unsigned char> pixelByteCode;
	bool isPixelShaderCompiled = CompileShaderToByteCode(pixelByteCode, shaderName, shaderSource, shaderConfig.m_pixelEntryPoint.c_str(), "ps_5_0");
	if (isPixelShaderCompiled == false) {
		ERROR_AND_DIE("Could not compile, Pixel shader");
	}

	hr = m_device->CreatePixelShader(pixelByteCode.data(), pixelByteCode.size(), NULL, &(shader->m_pixelShader));
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("Could not Create Pixel Shader.");
	}
	D3D11_INPUT_ELEMENT_DESC inputElementDesc[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};
	D3D11_INPUT_ELEMENT_DESC inputElementDescPNCU[] =
	{
		{"POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT,		0,		0,								D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR",		0, DXGI_FORMAT_R8G8B8A8_UNORM,		0,		D3D11_APPEND_ALIGNED_ELEMENT,	D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD",	0, DXGI_FORMAT_R32G32_FLOAT,		0,		D3D11_APPEND_ALIGNED_ELEMENT,	D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL",		0, DXGI_FORMAT_R32G32B32_FLOAT,		0,		D3D11_APPEND_ALIGNED_ELEMENT,	D3D11_INPUT_PER_VERTEX_DATA, 0}
	};
	D3D11_INPUT_ELEMENT_DESC inputElementDescPCUTBN[] =
	{
		{"POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT,		0,		0,								D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR",		0, DXGI_FORMAT_R8G8B8A8_UNORM,		0,		D3D11_APPEND_ALIGNED_ELEMENT,	D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD",	0, DXGI_FORMAT_R32G32_FLOAT,		0,		D3D11_APPEND_ALIGNED_ELEMENT,	D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TANGENT",		0, DXGI_FORMAT_R32G32B32_FLOAT,		0,		D3D11_APPEND_ALIGNED_ELEMENT,	D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"BINORMAL",	0, DXGI_FORMAT_R32G32B32_FLOAT,		0,		D3D11_APPEND_ALIGNED_ELEMENT,	D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL",		0, DXGI_FORMAT_R32G32B32_FLOAT,		0,		D3D11_APPEND_ALIGNED_ELEMENT,	D3D11_INPUT_PER_VERTEX_DATA, 0}
	};
	std::string name = shaderConfig.m_name;
	if (type == VertexType::Vertex_PCUTBN)
	{
		hr = m_device->CreateInputLayout(inputElementDescPCUTBN, 6, vertexByteCode.data(), vertexByteCode.size(), &(shader->m_inputLayout));
	}
	else if (type == VertexType::Vertex_PCU) 
	{
		//PNCU
		if (shaderName[13] == 'S' || shaderName[13] == 'P' || shaderName[13] == 'U' || name.find("Lit") || name.find("Normal") != name.npos || name.find("Lit") != name.npos)//SpriteLit  PointLit   Unlit
		{
			hr = m_device->CreateInputLayout(inputElementDescPNCU, 4, vertexByteCode.data(), vertexByteCode.size(), &(shader->m_inputLayout));
		}
		else  //PCU Default
		{
			hr = m_device->CreateInputLayout(inputElementDesc, 3, vertexByteCode.data(), vertexByteCode.size(), &(shader->m_inputLayout));
		}
	}
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("Could not Create Input layout .");
	}

	m_loadedShaders.push_back(shader);
	return shader;
}

bool Renderer::CompileShaderToByteCode(std::vector<unsigned char>& outByteCode, char const* name, char const* source, char const* entryPoint, char const* target)
{
	DWORD flags = D3DCOMPILE_OPTIMIZATION_LEVEL3;
#if defined(ENGINE_DEBUG_RENDER)
	flags = D3DCOMPILE_DEBUG;
	flags |= D3DCOMPILE_SKIP_OPTIMIZATION;
	flags |= D3DCOMPILE_WARNINGS_ARE_ERRORS;
#endif
	ID3DBlob* shaderBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;
	HRESULT hr1;
	hr1 = D3DCompile(source, strlen(source), name, NULL, NULL, entryPoint, target, flags, 0, &shaderBlob, &errorBlob);
	if (SUCCEEDED(hr1)) 
	{
		outByteCode.resize(shaderBlob->GetBufferSize());
		memcpy(outByteCode.data(), shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize());
		return true;
	}
	else 
	{
		DebuggerPrintf((const char*)errorBlob->GetBufferPointer());
		return false;
	}
}

Texture* Renderer::CreateTextureFromFile(char const* imageFilePath)
{
	IntVec2 dimensions = IntVec2(0, 0);// IntVec2::ZERO;		// This will be filled in for us to indicate image width & height
	int bytesPerTexel = 0; // This will be filled in for us to indicate how many color components the image had (e.g. 3=RGB=24bit, 4=RGBA=32bit)
	int numComponentsRequested = 0; // don't care; we support 3 (24-bit RGB) or 4 (32-bit RGBA)

	// Load (and decompress) the image RGB(A) bytes from a file on disk into a memory buffer (array of bytes)
	stbi_set_flip_vertically_on_load(1); // We prefer uvTexCoords has origin (0,0) at BOTTOM LEFT
	unsigned char* texelData = stbi_load(imageFilePath, &dimensions.x, &dimensions.y, &bytesPerTexel, numComponentsRequested);
	// Check if the load was successful
	GUARANTEE_OR_DIE(texelData, Stringf("Failed to load image \"%s\"", imageFilePath));
	Image image = Image(imageFilePath);

	Texture* newTexture = CreateTextureFromImage(image);

	// Free the raw image textural data now that we've sent a copy of it down to the GPU to be stored in video memory
	stbi_image_free(texelData);

	return newTexture;
}

Texture* Renderer::CreateTextureFromImage(const Image& image)
{
	//Create a new texture
	Texture* newTexture = new Texture();
	newTexture->m_dimensions = image.GetDimensions();
	newTexture->m_name = image.GetImageFilePath();

	//Create a texture 2D Description
	D3D11_TEXTURE2D_DESC texture2DDesc = { 0 };
	texture2DDesc.Width = newTexture->GetDimensions().x;
	texture2DDesc.Height = newTexture->GetDimensions().y;
	texture2DDesc.MipLevels = 1;
	texture2DDesc.ArraySize = 1;
	texture2DDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texture2DDesc.Usage = D3D11_USAGE_IMMUTABLE;
	texture2DDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texture2DDesc.SampleDesc.Count = 1;

	//Create a sub-resource date
	D3D11_SUBRESOURCE_DATA subresourceData = { 0 };
	subresourceData.pSysMem = image.GetRawData();
	subresourceData.SysMemPitch = sizeof(int) * (newTexture->GetDimensions().x);

	//Create Texture2D and Create Shader Resource View
	m_device->CreateTexture2D(&texture2DDesc, &subresourceData, &newTexture->m_texture);
	m_device->CreateShaderResourceView(newTexture->m_texture, NULL, &newTexture->m_shaderRecourceView);
	return newTexture;
}

Texture* Renderer::GetTextureForFileName(char const* imageFilePath)
{

	for (int i=0;i<m_loadedTextures.size();i++)
	{
		if (m_loadedTextures[i]->m_name == imageFilePath) {
			return m_loadedTextures[i];
		}
	}
	return nullptr;
}





VertexBuffer* Renderer::CreateVertexBuffer(const size_t size)
{
	D3D11_BUFFER_DESC bufferDevice = { 0 };
	bufferDevice.Usage = D3D11_USAGE_DYNAMIC;
	bufferDevice.ByteWidth = (UINT)size;
	bufferDevice.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDevice.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	VertexBuffer* localVertexBuffer = new VertexBuffer(size);
	HRESULT hr;
	hr = m_device->CreateBuffer(&bufferDevice, NULL, &localVertexBuffer->m_buffer);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("Could not create vertex buffer.");
	}
	return localVertexBuffer;
}

VertexBuffer* Renderer::CreateVertexBuffer(const size_t size, unsigned int stride)
{
	D3D11_BUFFER_DESC bufferDevice = { 0 };
	bufferDevice.Usage = D3D11_USAGE_DYNAMIC;
	bufferDevice.ByteWidth = (UINT)size;
	bufferDevice.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDevice.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	VertexBuffer* localVertexBuffer = new VertexBuffer(size, stride);
	HRESULT hr;
	hr = m_device->CreateBuffer(&bufferDevice, NULL, &localVertexBuffer->m_buffer);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("Could not create vertex buffer.");
	}
	return localVertexBuffer;
}

void Renderer::CopyCPUToGPU(const void* data, size_t size, VertexBuffer*& vbo)
{
	//Check
	if (vbo->m_size < size) 
	{
		delete vbo;
		vbo = nullptr;
		vbo = CreateVertexBuffer(size);
	}

	D3D11_MAPPED_SUBRESOURCE mappedSubresource;
	HRESULT hr;
	hr = m_deviceContext->Map(vbo->m_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);				//ToDo:Why sub resource 0 and flags 0
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("Could not Map subresource.");
	}
	memcpy(mappedSubresource.pData, data, size);
	m_deviceContext->Unmap(vbo->m_buffer, 0);
}



void Renderer::CopyCPUToGPU(const void* data, size_t size, IndexBuffer*& ibo)
{
	//Check
	if (ibo->m_size < size)
	{
		delete ibo;
		ibo = nullptr;
		ibo = CreateIndexBuffer(size);
	}

	D3D11_MAPPED_SUBRESOURCE mappedSubresource;
	HRESULT hr;
	hr = m_deviceContext->Map(ibo->m_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);				//ToDo:Why sub resource 0 and flags 0
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("Could not Map subresource.");
	}
	memcpy(mappedSubresource.pData, data, size);
	m_deviceContext->Unmap(ibo->m_buffer, 0);
}

void Renderer::CopyCPUToGPU(const void* data, size_t size, VertexBuffer*& vbo, unsigned int stride)
{
	//Check
	if (vbo->m_size < size)
	{
		delete vbo;
		vbo = nullptr;
		vbo = CreateVertexBuffer(size, stride);
	}

	D3D11_MAPPED_SUBRESOURCE mappedSubresource;
	HRESULT hr;
	hr = m_deviceContext->Map(vbo->m_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);				//ToDo:Why sub resource 0 and flags 0
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("Could not Map subresource.");
	}
	memcpy(mappedSubresource.pData, data, size);
	m_deviceContext->Unmap(vbo->m_buffer, 0);
}

void Renderer::BindIndexBuffer(IndexBuffer* ibo)
{
// 	UINT offset = 0;
// 	UINT sizeOfPCU = sizeof(Vertex_PCU);
	m_deviceContext->IASetIndexBuffer(ibo->m_buffer, DXGI_FORMAT_R32_UINT, 0);
	m_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

ConstantBuffer* Renderer::CreateConstantBuffer(const size_t size)
{
	D3D11_BUFFER_DESC bufferDevice = { 0 };
	bufferDevice.Usage = D3D11_USAGE_DYNAMIC;
	bufferDevice.ByteWidth = (UINT)size;
	bufferDevice.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDevice.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	ConstantBuffer* localConstantBuffer = new ConstantBuffer(size);
	HRESULT hr;
	hr = m_device->CreateBuffer(&bufferDevice, NULL, &localConstantBuffer->m_buffer);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("Could not create Constant buffer.");
	}
	return localConstantBuffer;
}

void Renderer::CopyCPUToGPU(const void* data, size_t size, ConstantBuffer*& cbo)
{
	D3D11_MAPPED_SUBRESOURCE mappedSubresource;
	HRESULT hr;
	hr = m_deviceContext->Map(cbo->m_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);				
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("Could not Map subresource.");
	}
	memcpy(mappedSubresource.pData, data, size);
	m_deviceContext->Unmap(cbo->m_buffer, 0);
}

void Renderer::BindConstantBuffer(int slot, ConstantBuffer* cbo)
{
	m_deviceContext->VSSetConstantBuffers(slot, 1, &cbo->m_buffer);
	m_deviceContext->PSSetConstantBuffers(slot, 1, &cbo->m_buffer);
}

void Renderer::BindVertexBuffer(VertexBuffer* vbo, PrimitiveTopology type /*= PrimitiveTopology::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST*/)
{
	UINT offset = 0;
	UINT sizeOfPCU = sizeof(Vertex_PCU);
	m_deviceContext->IASetVertexBuffers(0, 1, &vbo->m_buffer, &sizeOfPCU, &offset);
	m_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY(type));
}

void Renderer::BindPNCUVertexBuffer(VertexBuffer* vbo, PrimitiveTopology type /*= PrimitiveTopology::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST*/)
{
	UINT offset = 0;
	UINT stride = (UINT)vbo->GetStride();
	m_deviceContext->IASetVertexBuffers(0, 1, &vbo->m_buffer, &stride, &offset);
	m_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY(type));
}

IndexBuffer* Renderer::CreateIndexBuffer(const size_t size)
{
	D3D11_BUFFER_DESC bufferDevice = { 0 };
	bufferDevice.Usage = D3D11_USAGE_DYNAMIC;
	bufferDevice.ByteWidth = (UINT)size;
	bufferDevice.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bufferDevice.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	IndexBuffer* localIndexBuffer = new IndexBuffer(size);
	HRESULT hr;
	hr = m_device->CreateBuffer(&bufferDevice, NULL, &localIndexBuffer->m_buffer);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("Could not create index buffer.");
	}
	return localIndexBuffer;
}


void Renderer::DrawVertexBuffer(VertexBuffer* vbo, int vertexCount, int vertexOffset /*= 0*/, PrimitiveTopology type /*= PrimitiveTopology::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST*/)
{
	SetStateIfChanged();
	BindVertexBuffer(vbo, type);
	m_deviceContext->Draw(vertexCount, vertexOffset);
}

void Renderer::DrawVertexPNCUBuffer(VertexBuffer* vbo, int vertexCount, int vertexOffset /*= 0*/, PrimitiveTopology type /*= PrimitiveTopology::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST*/)
{
	SetStateIfChanged();
	BindPNCUVertexBuffer(vbo, type);
	m_deviceContext->Draw(vertexCount, vertexOffset);
}

void Renderer::DrawIndexBuffer(IndexBuffer* ibo, int indexCount, int vertexOffset /*= 0*/)
{
	SetStateIfChanged();
	BindIndexBuffer(ibo);
	m_deviceContext->DrawIndexed(indexCount, vertexOffset, 0);

}

void Renderer::DrawVertexAndIndexBuffer(VertexBuffer* vbo, IndexBuffer* ibo, int indexCount, int indexOffset /*= 0*/, int vertexOffset /*= 0*/, PrimitiveTopology type /*= PrimitiveTopology::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST*/)
{
	SetStateIfChanged();
	BindPNCUVertexBuffer(vbo, type);
	//BindVertexBuffer(vbo);
	BindIndexBuffer(ibo);
	m_deviceContext->DrawIndexed(indexCount, indexOffset, vertexOffset);
}

Texture* Renderer::CreateTextureFromNothing(std::string const& name, IntVec2 const& dimensions, ID3D11Texture2D* texture, ID3D11ShaderResourceView* shaderRecourceView)
{
	Texture* screenTexture = new Texture();
	screenTexture->m_dimensions = dimensions;
	screenTexture->m_name = name;
	screenTexture->m_texture = texture;
	screenTexture->m_shaderRecourceView = shaderRecourceView;
	return screenTexture;
}

void Renderer::SetBackBufferRenderTarget() const
{
	// Bind the render target view and depth stencil buffer to the output render pipeline.
	m_deviceContext->OMSetRenderTargets(1, &m_renderTargetView, m_depthStencilView);
}

void Renderer::UnbindRenderTarget() const
{
	m_deviceContext->OMSetRenderTargets(1, &m_renderTargetView, NULL);
}

Texture* Renderer::CreateDepthTexture()
{
	Texture* depthtexture = new Texture();

	//Create and set a depth stencil view and state
	D3D11_TEXTURE2D_DESC texture2DDesc = { 0 };
	texture2DDesc.Width = m_config.m_window->GetClientDimensions().x;
	texture2DDesc.Height = m_config.m_window->GetClientDimensions().y;
	texture2DDesc.MipLevels = 1;
	texture2DDesc.ArraySize = 1;
	texture2DDesc.Usage = D3D11_USAGE_DEFAULT;
	texture2DDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	texture2DDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	texture2DDesc.SampleDesc.Count = 1;

	depthtexture->m_dimensions= m_config.m_window->GetClientDimensions();
	
	m_device->CreateTexture2D(&texture2DDesc, nullptr, &depthtexture->m_texture);
	GUARANTEE_OR_DIE(depthtexture->m_texture, "Texture is not created!");
	texture2DDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencil = {};
	depthStencil.Format = DXGI_FORMAT_D32_FLOAT;
	depthStencil.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;

	m_device->CreateDepthStencilView(depthtexture->m_texture, &depthStencil, &depthtexture->m_depthStencilView);
	texture2DDesc.Format = DXGI_FORMAT_R32_FLOAT;

	D3D11_SHADER_RESOURCE_VIEW_DESC shaderRV = {};
	shaderRV.Format = DXGI_FORMAT_R32_FLOAT;
	shaderRV.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderRV.Texture2D.MipLevels = 1;

	m_device->CreateShaderResourceView(depthtexture->m_texture, &shaderRV, &depthtexture->m_shaderRecourceView);
	return depthtexture;
}



void Renderer::ClearDepthTexture(Texture* depthTexture, float clearColor /*= 1.f*/)
{
	m_deviceContext->ClearDepthStencilView(depthTexture->m_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, clearColor, 0);

}

void Renderer::BindDepthTextureWhileSetRenderTarget(Texture* depthTex, ID3D11RenderTargetView* renderTargetView /*= nullptr*/)
{
	m_deviceContext->OMSetRenderTargets(1, &m_renderTargetView, depthTex->m_depthStencilView);
}

// void Renderer::BindDepthTextureWhileSetRenderTarget(Texture* depthTex, ID3D11RenderTargetView* renderTargetView = )
// {
// 	
// }

bool Renderer::InitializeRnederTarget(RenderTextureClass* renderTargetToIntialize, int textureWidth, int textureHeight)
{
	return renderTargetToIntialize->Initialize(m_device, textureWidth, textureHeight);
}

//---------------------------------------------------------------------------------------------------------------------


BitmapFont* Renderer::CreateOrGetBitmapFont(char const* imageFilePath)
{
	BitmapFont* existingFont = GetBitmapFontForFileName(imageFilePath);
	if (existingFont)
	{
		return existingFont;
	}
	BitmapFont* bitmapFont = CreateBitmapFont(imageFilePath);
	return bitmapFont;
}

void Renderer::SetStateIfChanged()
{
	if (m_blendState != m_blendStates[(int)m_desiredBlendMode])
	{
		m_blendState = m_blendStates[(int)m_desiredBlendMode];
		float blendFactorArray[4] = { 0 };
		m_deviceContext->OMSetBlendState(m_blendState, blendFactorArray, 0xffffffff);
	}

	if (m_samplerState != m_samplerStates[(int)m_desiredSamplerMode])
	{
		m_samplerState = m_samplerStates[(int)m_desiredSamplerMode];
		m_deviceContext->PSSetSamplers(0, 1, &m_samplerState);
	}

	if (m_rasterizerState != m_rasterizerStates[(int)m_desiredRasterizerMode])
	{
		m_rasterizerState = m_rasterizerStates[(int)m_desiredRasterizerMode];
		m_deviceContext->RSSetState(m_rasterizerState);
	}

	if (m_depthStencilState != m_depthStencilStates[(int)m_desiredDepthmode])
	{
		m_depthStencilState = m_depthStencilStates[(int)m_desiredDepthmode];
		m_deviceContext->OMSetDepthStencilState(m_depthStencilState, 0);
	}
}

void Renderer::SetBlendMode(BlendMode blendMode)
{
	m_desiredBlendMode = blendMode;
}

void Renderer::SetSamplerMode(SamplerMode samplerMode)
{
	m_desiredSamplerMode = samplerMode;
}

void Renderer::SetRasterizerMode(RasterizerMode rasterizerMode)
{
	m_desiredRasterizerMode = rasterizerMode;
}

void Renderer::SetDepthMode(DepthMode depthMode)
{
	m_desiredDepthmode = depthMode;
}

Shader* Renderer::GetShaderByName(std::string const& name)
{
	for (int shaderindex = 0; shaderindex < (int)m_loadedShaders.size(); shaderindex++) 
	{
		if (m_loadedShaders[shaderindex]->m_config.m_name == name) 
		{
			return m_loadedShaders[shaderindex];
		}
	}
	return nullptr;
}

// BitmapFont* Renderer::CreateBitmapFontFromFile(char const* imageFilePath)
// {
// 	IntVec2 dimensions = IntVec2(0, 0);// IntVec2::ZERO;		// This will be filled in for us to indicate image width & height
// 	int bytesPerTexel = 0; // This will be filled in for us to indicate how many color components the image had (e.g. 3=RGB=24bit, 4=RGBA=32bit)
// 	int numComponentsRequested = 0; // don't care; we support 3 (24-bit RGB) or 4 (32-bit RGBA)
// 
// 	// Load (and decompress) the image RGB(A) bytes from a file on disk into a memory buffer (array of bytes)
// 	stbi_set_flip_vertically_on_load(1); // We prefer uvTexCoords has origin (0,0) at BOTTOM LEFT
// 	std::string imageFilePathWithExtension = imageFilePath;
// 	imageFilePathWithExtension += "png";
// 	unsigned char* texelData = stbi_load(imageFilePath, &dimensions.x, &dimensions.y, &bytesPerTexel, numComponentsRequested);
// 
// 	// Check if the load was successful
// 	GUARANTEE_OR_DIE(texelData, Stringf("Failed to load image \"%s\"", imageFilePathWithExtension));
// 
// 	Texture* newFont= CreateOrGetTextureFromFile(imageFilePathWithExtension.c_str());
// 	BitmapFont* newBitmapFont = new BitmapFont(imageFilePathWithExtension.c_str(), *newFont);
// 	// Free the raw image texel data now that we've sent a copy of it down to the GPU to be stored in video memory
// 	stbi_image_free(texelData);
// 
// 	return newBitmapFont;
// }

BitmapFont* Renderer::CreateBitmapFont(char const* fontFilePathNameNoExtension)
{
	std::string fontGlyphsImageFilePath = fontFilePathNameNoExtension;
	fontGlyphsImageFilePath += ".png";
	Texture* fontGlyphsTexture = CreateOrGetTextureFromFile(fontGlyphsImageFilePath.c_str());
	if (!fontGlyphsTexture) 
	{
		return nullptr;
	}
	BitmapFont* newFont = new BitmapFont(fontFilePathNameNoExtension, *fontGlyphsTexture);
	m_loadedBitMapFond.push_back(newFont);
	return newFont;
}

BitmapFont* Renderer::GetBitmapFontForFileName(char const* imageFilePath)
{
	for (int i = 0; i < m_loadedBitMapFond.size(); i++)
	{
		if (m_loadedBitMapFond[i]->m_fontFilePathNameWithNoExtension == imageFilePath) {
			return m_loadedBitMapFond[i];
		}
	}
	return nullptr;
}

void Renderer::SetModelConstants(const Mat44& modelMatrix /*= Mat44()*/, const Rgba8& modelColor /*= Rgba8::WHITE*/)
{
	ModelConstants localModelContants = {};
	localModelContants.ModelMatrix = modelMatrix;
	modelColor.GetAsFloats(localModelContants.ModeColor);
	size_t size = (size_t)sizeof(ModelConstants);
	CopyCPUToGPU(&localModelContants, size, m_modelCBO);
	BindConstantBuffer(k_modelConstantsSlot, m_modelCBO);
}

void Renderer::SetLightConstants(Vec3 const& sunDirection, float sunIntensity, float ambientIntensity, Vec3 const& worldEyePosi /*= Vec3::WORLD_ORIGIN*/, int normalMode /*= 0*/, int specularMode /*= 0*/, float specularIntensity /*= 0.f*/, float specularPower /*= 0.f*/)
{
	LightConstants localLightConstants = {};
	localLightConstants.AmbientIntensity = ambientIntensity;
	localLightConstants.SunDirection = sunDirection;
	localLightConstants.SunIntensity = sunIntensity;
	localLightConstants.WorldEyePosition = worldEyePosi;
	localLightConstants.NormalMode = normalMode;
	localLightConstants.SpecularMode = specularMode;
	localLightConstants.SpecularIntensity = specularIntensity;
	localLightConstants.SpecularPower = specularPower;

	size_t size = (size_t)sizeof(LightConstants);
	CopyCPUToGPU(&localLightConstants, size, m_lightCBO);
	BindConstantBuffer(k_lightConstantsSlot, m_lightCBO);
}

void Renderer::SetGamePlayConstants(Vec3 const& cameraWorldPosition, Vec4 const& skyColor)
{
	GameConstants localGameConstants = {};
	localGameConstants.b_camWorldPos[0] = cameraWorldPosition.x;
	localGameConstants.b_camWorldPos[1] = cameraWorldPosition.y;
	localGameConstants.b_camWorldPos[2] = cameraWorldPosition.z;
	localGameConstants.b_camWorldPos[3] = 0.f;

	localGameConstants.b_skyColor[0] = skyColor.x;
	localGameConstants.b_skyColor[1] = skyColor.y;
	localGameConstants.b_skyColor[2] = skyColor.z;
	localGameConstants.b_skyColor[3] = skyColor.w;

	size_t size = (size_t)sizeof(GameConstants);
	CopyCPUToGPU(&localGameConstants, size, m_gameCBO);
	BindConstantBuffer(k_gameConstantsSlot, m_gameCBO);

}

void Renderer::SetScreenSizeBuffer(float screenWidth, float screenHeight)
{
	ScreenSizeConstants localScreenSizeConstants = {};
	localScreenSizeConstants.screenWidth = screenWidth;
	localScreenSizeConstants.screenHeight = screenHeight;
	localScreenSizeConstants.screenPading = Vec2(0.f, 0.f);
	size_t size = (size_t)sizeof(ScreenSizeConstants);
	CopyCPUToGPU(&localScreenSizeConstants, size, m_screenSizeCBP);
	BindConstantBuffer(k_screenSizeConstantsSlot, m_screenSizeCBP);

}

void Renderer::BindTexture(const Texture* texture)
{
	if (texture == nullptr)
	{
		texture = m_defaultTexture;
	}
	m_deviceContext->PSSetShaderResources(0, 1, &texture->m_shaderRecourceView);


}

void Renderer::BindTextures(const Texture* texture1, const Texture* texture2, const Texture* texture3)
{
	if (texture1 == nullptr) 
	{
		texture1 = m_defaultTexture;
	}
	if (texture2 == nullptr)
	{
		texture2 = m_defaultTexture;
	}
	if (texture3 == nullptr)
	{
		texture3 = m_defaultTexture;
	}

	m_deviceContext->PSSetShaderResources(0, 1, &texture1->m_shaderRecourceView);
	m_deviceContext->PSSetShaderResources(1, 1, &texture2->m_shaderRecourceView);
	m_deviceContext->PSSetShaderResources(2, 1, &texture3->m_shaderRecourceView);
}

void Renderer::BindFourTextures(const Texture* texture0, const Texture* texture1, const Texture* texture2, const Texture* texture3)
{
	if (texture0 == nullptr)
	{
		texture0 = m_defaultTexture;
	}
	if (texture1 == nullptr)
	{
		texture1 = m_defaultTexture;
	}
	if (texture2 == nullptr)
	{
		texture2 = m_defaultTexture;
	}
	if (texture3 == nullptr)
	{
		texture3 = m_defaultTexture;
	}
	m_deviceContext->PSSetShaderResources(0, 1, &texture0->m_shaderRecourceView);
	m_deviceContext->PSSetShaderResources(1, 1, &texture1->m_shaderRecourceView);
	m_deviceContext->PSSetShaderResources(2, 1, &texture2->m_shaderRecourceView);
	m_deviceContext->PSSetShaderResources(3, 1, &texture3->m_shaderRecourceView);
}


void Renderer::BindTexturesList(unsigned int Num, std::vector<Texture*>const textures)
{
	for (unsigned int textureIndex = 0; textureIndex < Num; textureIndex++)
	{
		Texture* tex = textures[textureIndex];
		m_deviceContext->PSSetShaderResources(textureIndex, 1, &tex->m_shaderRecourceView);
	}
}

