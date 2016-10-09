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
	};

	struct PostProcessData
	{
		XMFLOAT2 screenWH;
		float expa;
	};

public:
	CTextureRenderShader();
	CTextureRenderShader(const CTextureRenderShader&);
	~CTextureRenderShader();

	float Exposure = 0.0f;

	bool Initialize(ID3D11Device*, HWND);
	void Shutdown();
	void SetSpecularHighLights(ID3D11DeviceContext* devcon, ID3D11ShaderResourceView* tex);
	bool Render(ID3D11DeviceContext*, int, XMMATRIX&, XMMATRIX&, XMMATRIX&, ID3D11ShaderResourceView*, XMFLOAT2 swh);
	void RenderShader(ID3D11DeviceContext*, int);
private:
	bool InitializeShader(ID3D11Device*, HWND, WCHAR*, WCHAR*);
	void ShutdownShader();
	void OutputShaderErrorMessage(ID3D10Blob*, HWND, WCHAR*);

	bool SetShaderParameters(ID3D11DeviceContext*, XMMATRIX&, XMMATRIX&, XMMATRIX&, ID3D11ShaderResourceView*, XMFLOAT2 swh);


private:
	ID3D11VertexShader* m_vertexShader;
	ID3D11PixelShader* m_pixelShader;
	ID3D11InputLayout* m_layout;
	ID3D11Buffer* m_matrixBuffer;
	ID3D11Buffer* m_PostProcess;
	ID3D11SamplerState* m_sampleState;
};

#endif