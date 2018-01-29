#pragma once
#ifndef SKYDOMESHADER_H
#define SKYDOMESHADER_H

#include "DeviceClass.h"
#include <fstream>
#include "Lights.h"
#include "FreeCamera.h"




using namespace DirectX;


#include "Ext\HosekSky\Include\ArHosekSkyModel.h"
class CSkyDomeShader
{
public:
	CSkyDomeShader();
	~CSkyDomeShader();

private:
	struct SkyModelData
	{
		ArHosekSkyModelState* StateR = nullptr;
		ArHosekSkyModelState* StateG = nullptr;
		ArHosekSkyModelState* StateB = nullptr;

		XMFLOAT3 SunDirection;
		float Turbidity = 0.0f;
		XMFLOAT3 Albedo;
		float Elevation = 0.0f;
		ID3D11ShaderResourceView* CubeMap;
		void Init(XMFLOAT3 sunDirection, XMFLOAT3 groundAlbedo, float turbidity);
	};

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
		float radiance;
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




public:
	SkyModelData skyModel;

    XMFLOAT3 SampleSky(const SkyModelData& data, XMVECTOR sampleDir);

private:

	ID3D11VertexShader* m_vertexShader;
	ID3D11PixelShader* m_pixelShader;
	ID3D11InputLayout* m_layout;
	ID3D11Buffer* m_matrixBuffer;
	ID3D11Buffer* m_colorBuffer;
};

static float clip(float n, float lower, float upper);


static XMFLOAT3 Saturate(XMFLOAT3 val);
static float Saturate(float val);

#endif