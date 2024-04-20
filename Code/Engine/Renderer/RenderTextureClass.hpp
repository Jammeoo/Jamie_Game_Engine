#pragma once
#include <d3d11.h>
#include "Engine/Core/Rgba8.hpp"
class RenderTextureClass
{
public:
	RenderTextureClass();
	~RenderTextureClass();

	bool Initialize(ID3D11Device* device, int textureWidth, int textureHeight);
	void Shutdown();

	void SetRenderTarget(ID3D11DeviceContext* deviceContext, ID3D11DepthStencilView* depthStencilView);
	void ClearRenderTarget(ID3D11DeviceContext* deviceContext, ID3D11DepthStencilView* depthStencilView, Rgba8 const& color);
	ID3D11ShaderResourceView* GetShaderResourceView();

public:
	ID3D11Texture2D* m_renderTargetTexture = nullptr;
	ID3D11RenderTargetView* m_renderTargetView = nullptr;
	ID3D11ShaderResourceView* m_shaderResourceView = nullptr;
};