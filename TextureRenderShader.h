#pragma once


//Used for post processing so no other rendering
#ifndef TEXTURESHADERRENDER_H
#define TEXTURESHADERRENDER_H

#include "DeviceClass.h"
#include <fstream>

class CTextureRenderShader
{
private:
	struct MatrixBufferType
	{
		XMMATRIX world;
		XMMATRIX view;
		XMMATRIX projection;
		XMMATRIX projV;
	};

	struct PostProcessData
	{
		XMFLOAT2 screenWH;
		float expa;
		XMVECTOR ssaoSampl[64];
	};

public:
	CTextureRenderShader();
	CTextureRenderShader(const CTextureRenderShader&);
	~CTextureRenderShader();

	float Exposure = 0.0f;

	bool Initialize(CDeviceClass *devclass, HWND);
	void Shutdown();
	void UpdateTextureIndex(ID3D11DeviceContext* devcon, ID3D11ShaderResourceView* tex, int index);
	void SetSpecularHighLights(ID3D11DeviceContext* devcon, ID3D11ShaderResourceView* tex);
	bool Render(ID3D11DeviceContext*, int, XMMATRIX&, XMMATRIX&, XMMATRIX&, XMFLOAT2 swh);

	bool SetShaderParameters(ID3D11DeviceContext*, XMMATRIX&, XMMATRIX&, XMMATRIX&, XMFLOAT2 swh, std::vector<XMVECTOR> ssaoSampl);

	void RenderShader(ID3D11DeviceContext*, int);


	void RenderWithShaders(ID3D11DeviceContext * deviceContext, int indexCount, ID3D11VertexShader* vs, ID3D11PixelShader* ps);

	void RenderShaderBlurV(ID3D11DeviceContext*, int);
	void RenderShaderBlurH(ID3D11DeviceContext*, int);
	void RenderShaderBloom(ID3D11DeviceContext*, int);
	void RenderShaderCombine(ID3D11DeviceContext*, int);
	void RenderShaderToneMap(ID3D11DeviceContext*, int);
	void RenderShaderColor(ID3D11DeviceContext*, int);

	void PPSSAO(ID3D11DeviceContext* deviceContext, int indexCount);

private:
	bool InitializeShader(CDeviceClass *devclass, HWND, WCHAR*, WCHAR*);
	void ShutdownShader();
	void OutputShaderErrorMessage(ID3D10Blob*, HWND, WCHAR*);

	
public:
	ID3D11VertexShader* m_vertexShader;
	ID3D11PixelShader* m_pixelShader;

	ID3D11PixelShader* m_pixelShaderSSAO;

	ID3D11PixelShader* m_pixelShaderColor;
	ID3D11PixelShader* m_pixelShaderBloom;
	ID3D11PixelShader* m_pixelShaderBlurV;
	ID3D11PixelShader* m_pixelShaderBlurH;
	ID3D11PixelShader* m_pixelShaderToneMap;

	ID3D11VertexShader* m_SMAALumaEdgeVS;
	ID3D11VertexShader* m_SmaaBlendingWeightVS;
	ID3D11VertexShader* m_SMAANeighborhoodBlendVS;

	ID3D11PixelShader* m_SMAALumaEdgePS;
	ID3D11PixelShader* m_SMAABlendingWeightPS;
	ID3D11PixelShader* m_SMAANeighborhoodBlendPS;

private:


	

	CDeviceClass* devclass;
	ID3D11InputLayout* m_layout;
	ID3D11InputLayout* m_layoutSMAA;
	ID3D11Buffer* m_matrixBuffer;
	ID3D11Buffer* m_PostProcess;
	ID3D11SamplerState* m_sampleState;
};

#endif