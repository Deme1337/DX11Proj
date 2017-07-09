#pragma once

#ifndef POSTPROCESSOR_H
#define POSTPROCESSOR_H

#include "DeviceClass.h"
#include "OrthoWindow.h"
#include "RenderTarget.h"
#include "DeferredBufferClass.h"
#include "TextureRenderShader.h"
#include "TextureTA.h"

class PostProcessor
{
public:
	PostProcessor();
	~PostProcessor();

	bool useSmaa = false;

	bool InitializePostProcessor(CDeviceClass* devclass, int windowwidth, int windowheight);
	void ClearRenderTarget();

	void SetPostProcessInputs(ID3D11ShaderResourceView* color, ID3D11ShaderResourceView* light, COrthoWindow* window, float exposure);
	ID3D11ShaderResourceView* SmaaProcess(ID3D11ShaderResourceView* input, COrthoWindow* window, ID3D11ShaderResourceView* areaTex, ID3D11ShaderResourceView* searchTex, RenderTarget* blurred); //Returns SMAA texture that will be rendered on screen 
	void PostProcess(COrthoWindow* window, ID3D11ShaderResourceView* color);

	ID3D11ShaderResourceView* prepareSmaa(COrthoWindow* window, ID3D11ShaderResourceView* color, ID3D11ShaderResourceView* light, ID3D11ShaderResourceView* areaTex, ID3D11ShaderResourceView* searchTex);

	ID3D11ShaderResourceView* CreateSSAO(CDeviceClass* devclass, COrthoWindow* window, ID3D11ShaderResourceView* pos, ID3D11ShaderResourceView* normal);

	void DebugGBufferTextures(DeferredBuffersClass* buf, CDeviceClass *devclass, COrthoWindow* window);

	void Release();

private:
	CDeviceClass* devclass;
	int _width, _height;

	RenderTarget* bloom;
	RenderTarget* blurV;
	RenderTarget* blurH;
	RenderTarget* combine;
	RenderTarget* SSAO;

	//SMAA
	RenderTarget* blendTex;
	RenderTarget* edgesTex;
	RenderTarget* smaaResultTex;

	void GenerateSSAOSamples();

	std::vector<XMVECTOR> ssaoKernel;
	CTextureRenderShader* rtShader;

	XMMATRIX worldMatrix;
	XMMATRIX orthoMatrix;
	XMMATRIX baseViewMatrix;

	ID3D11ShaderResourceView* color;

	CTextureTA *ssaoRand;

	int viewPortOffset = 0;

	void UpdatePostProcessorMatrices();
};

#endif