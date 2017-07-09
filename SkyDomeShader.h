#pragma once
#ifndef SKYDOMESHADER_H
#define SKYDOMESHADER_H

#include "DeviceClass.h"
#include <fstream>
#include "Lights.h"
#include "FreeCamera.h"



using namespace DirectX;



class CSkyDomeShader
{
public:
	CSkyDomeShader();
	~CSkyDomeShader();

private:
	struct MatrixBufferType
	{
		XMMATRIX world;
		XMMATRIX view;
		XMMATRIX projection;
		XMVECTOR sunPosition;
	};

	struct ColorBufferType
	{
		XMFLOAT4 apexColor;
		XMFLOAT4 centerColor;
		XMVECTOR sunPosition;
		XMVECTOR cameraPosition;
		float sunSize;
		float sunPower;
		float scale1;
		float scale2;
	};

public:

	CSkyDomeShader(const CSkyDomeShader&);


	bool Initialize(ID3D11Device* device, HWND hwnd);
	void Shutdown();
	bool Update(ID3D11DeviceContext* deviceContext, int indexCount, XMMATRIX &worldMatrix, XMMATRIX &viewMatrix,
		XMMATRIX &projectionMatrix, XMFLOAT4 &apexColor, XMFLOAT4 &centerColor, DirectionalLight &dlight, FreeCamera* cam);
	void SetSkyDomeTexture(ID3D11DeviceContext* devcon, ID3D11ShaderResourceView *tex, int index);
private:
	bool InitializeShader(ID3D11Device* device, HWND hwnd, WCHAR* vsFilename, WCHAR* psFilename);
	void ShutdownShader();
	void OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, WCHAR* shaderFilename);

	bool SetShaderParameters(ID3D11DeviceContext* deviceContext, XMMATRIX &worldMatrix, XMMATRIX &viewMatrix,
		XMMATRIX &projectionMatrix, XMFLOAT4 &apexColor, XMFLOAT4 &centerColor, DirectionalLight &dlight, FreeCamera* cam);
	void RenderShader(ID3D11DeviceContext* deviceContext, int indexCount);

private:

	

	ID3D11VertexShader* m_vertexShader;
	ID3D11PixelShader* m_pixelShader;
	ID3D11InputLayout* m_layout;
	ID3D11Buffer* m_matrixBuffer;
	ID3D11Buffer* m_colorBuffer;
};

#endif