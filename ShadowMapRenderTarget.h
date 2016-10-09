#pragma once

#ifndef SHADOWMAP_RT
#define SHADOWMAP_RT

#include "DeviceClass.h"


class ShadowMapRenderTarget
{
public:
	ShadowMapRenderTarget();
	ShadowMapRenderTarget(const ShadowMapRenderTarget&);
	~ShadowMapRenderTarget();

	bool Initialize(CDeviceClass*, int, int, float, float);
	void Shutdown();

	void SetRenderTarget(ID3D11DeviceContext*);
	void ClearRenderTarget(ID3D11DeviceContext*, float, float, float, float);
	ID3D11ShaderResourceView* GetShaderResourceView();
	ID3D11Texture2D* GetSubResource();

	void SetTexture(ID3D11Texture2D* texture);
	void GetProjectionMatrix(XMMATRIX&);
	void GetOrthoMatrix(XMMATRIX&);

private:
	ID3D11Texture2D* m_renderTargetTexture;
	ID3D11RenderTargetView* m_renderTargetView;
	ID3D11ShaderResourceView* m_shaderResourceView;
	ID3D11Texture2D* m_depthStencilBuffer;
	ID3D11DepthStencilView* m_depthStencilView;
	D3D11_VIEWPORT m_viewport;
	XMMATRIX m_projectionMatrix;
	XMMATRIX m_orthoMatrix;
};

#endif