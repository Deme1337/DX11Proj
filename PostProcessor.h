#pragma once

#ifndef POSTPROCESSOR_H
#define POSTPROCESSOR_H

#include "DeviceClass.h"
#include "OrthoWindow.h"
#include "RenderTarget.h"
#include "TextureRenderShader.h"

class PostProcessor
{
public:
	PostProcessor();
	~PostProcessor();

	bool InitializePostProcessor(CDeviceClass* devclass, int windowwidth, int windowheight);
	void ClearRenderTarget();

	void SetPostProcessInputs(ID3D11ShaderResourceView* color, ID3D11ShaderResourceView* light, COrthoWindow* window, float exposure);

	void PostProcess(COrthoWindow* window);

	void Release();

private:
	CDeviceClass* devclass;
	int _width, _height;

	RenderTarget* bloom;
	RenderTarget* blurV;
	RenderTarget* blurH;
	RenderTarget* combine;

	CTextureRenderShader* rtShader;

	XMMATRIX worldMatrix;
	XMMATRIX orthoMatrix;
	XMMATRIX baseViewMatrix;

	ID3D11ShaderResourceView* color;

	int viewPortOffset = 0;

	void UpdatePostProcessorMatrices();
};

#endif