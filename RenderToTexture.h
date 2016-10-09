#pragma once

#ifndef TRENDERER_H
#define TRENDERER_H

#include "DeviceClass.h"
#define BUFFER_COUNT_PP 2


class CRenderToTexture
{
public:
	CRenderToTexture();
	CRenderToTexture(const CRenderToTexture&);
	~CRenderToTexture();

	bool Initialize(CDeviceClass*, int, int, float, float, int samplecount);
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
	ID3D11Texture2D* m_renderTargetTexture[BUFFER_COUNT_PP];
	ID3D11RenderTargetView* m_renderTargetView[BUFFER_COUNT_PP];
	ID3D11ShaderResourceView* m_shaderResourceView[BUFFER_COUNT_PP];
	ID3D11Texture2D* m_depthStencilBuffer;
	ID3D11DepthStencilView* m_depthStencilView;
	D3D11_VIEWPORT m_viewport;
	XMMATRIX m_projectionMatrix;
	XMMATRIX m_orthoMatrix;
};

#endif