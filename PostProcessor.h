#pragma once

#ifndef POSTPROCESSOR_H
#define POSTPROCESSOR_H

#include "DeviceClass.h"
#include "OrthoWindow.h"
#include "RenderTarget.h"
#include "DeferredBufferClass.h"
#include "TextureRenderShader.h"
#include "TextureTA.h"
#include "FreeCamera.h"

class PostProcessor
{
public:
	PostProcessor();
	~PostProcessor();

	bool InitializePostProcessor(CDeviceClass* devclass, int windowwidth, int windowheight);
	void ClearRenderTarget();

	void SetPostProcessInputs(ID3D11ShaderResourceView* color, ID3D11ShaderResourceView* light, COrthoWindow* window, float exposure);
	ID3D11ShaderResourceView* SmaaProcess(COrthoWindow * window, ID3D11ShaderResourceView* input, ID3D11ShaderResourceView* areaTex, ID3D11ShaderResourceView* searchTex); //Returns SMAA texture that will be rendered on screen 
	void PostProcess(COrthoWindow* window, ID3D11ShaderResourceView* color);

	ID3D11ShaderResourceView* prepareSmaa(COrthoWindow * window, ID3D11ShaderResourceView* input, ID3D11ShaderResourceView * areaTex, ID3D11ShaderResourceView * searchTex);

	ID3D11ShaderResourceView* CreateSSAO(CDeviceClass* devclass, COrthoWindow* window, ID3D11ShaderResourceView* pos, ID3D11ShaderResourceView* normal, 
										 ID3D11ShaderResourceView* ssaoNoise, float expss, ID3D11ShaderResourceView* tangentTexture, ID3D11ShaderResourceView* bitangentTexture,
											XMMATRIX& lightView, XMMATRIX& lightProjection, FreeCamera *cam);

	ID3D11ShaderResourceView* BlurShadows(CDeviceClass* devclass, COrthoWindow* window, ID3D11ShaderResourceView* pos);

	void DebugGBufferTextures(DeferredBuffersClass* buf, CDeviceClass *devclass, COrthoWindow* window);

	void Release();



public: //Variables

	bool useSmaa = false;
	XMFLOAT2 ssaoBiasAndRadius;

public: //Objects
	RenderTarget* smaaFinalizeTex;
	RenderTarget* bloom;
	RenderTarget* blurV;
	RenderTarget* blurH;
	RenderTarget* combine;
	RenderTarget* toneMap;
	RenderTarget* SSAO;

	//SMAA
	RenderTarget* blendTex;
	RenderTarget* edgesTex;
	RenderTarget* smaaResultTex;

private: // functions

	void GenerateSSAOSamples();
	void UpdatePostProcessorMatrices();

private: //Variables

	int _width, _height;
	int viewPortOffset = 0;
	XMMATRIX worldMatrix;
	XMMATRIX orthoMatrix;
	XMMATRIX baseViewMatrix;

private: //Objects

	CDeviceClass* devclass;
	CTextureRenderShader* rtShader;
	ID3D11ShaderResourceView* color;

	CTextureTA *ssaoRand;
	std::vector<XMFLOAT4> ssaoKernel;

};

#endif