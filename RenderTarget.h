#pragma once

#ifndef RENDERTARGET_H
#define RENDERTARGET_H

#include "DeviceClass.h"



class RenderTarget
{
public:
	RenderTarget();
	RenderTarget(const RenderTarget&);
	~RenderTarget();

	bool Initialize(CDeviceClass*, int, int, float, float, int samplecount, DXGI_FORMAT format);
	void Shutdown();

	void SetRenderTargets(ID3D11DeviceContext*);
	void SetRenderTarget(ID3D11DeviceContext*);
	void ClearRenderTarget(ID3D11DeviceContext*, float, float, float, float);


	ID3D11ShaderResourceView* GetShaderResourceView(int index);
	//return null
	ID3D11Texture2D* GetSubResource();
	//sets nothing
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
