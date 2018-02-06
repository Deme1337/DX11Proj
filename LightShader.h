#pragma once


#ifndef LIGHTSHADER_H
#define LIGHTSHADER_H

#include <fstream>

#include "DeviceClass.h"
#include "Lights.h"
#include "DeferredBufferClass.h"

class LightShader
{
private:
#define POINT_LIGHTS 40

	struct MatrixBuffer
	{
		XMMATRIX world;
		XMMATRIX view;
		XMMATRIX projection;
	};

	struct LightBuffer
	{
		XMVECTOR LightPosition;
		XMVECTOR LightColor;
		XMMATRIX lightViewMatrix;
		XMMATRIX lightProjectionMatrix;
		XMMATRIX viewMatrix;
		XMMATRIX projectionMatrix;
		float GlobalAmbient;
		float shadowBias;
		float attenuation;
	};

	struct DisneyParam
	{
		XMFLOAT4 subspectintani;
		XMFLOAT4 sheentintcleargloss;
	};

public:
	struct PointLightBuffer
	{
		XMVECTOR PointLightPosition[POINT_LIGHTS];
		XMVECTOR PointLightColor[POINT_LIGHTS];
	};

private:
	struct CameraBuffer
	{
		XMVECTOR CameraPosition;
	};

public:
	LightShader();
	~LightShader();

	XMMATRIX tempViewMatrix;

	bool Initialize(CDeviceClass *devclass, WCHAR* vsFilename, WCHAR* psFilename);

	void UpdateDisneyBuffer(CDeviceClass* devclass, XMFLOAT4 f1, XMFLOAT4 f2);
	void UpdateCameraPosition(CDeviceClass * devclass, XMVECTOR cp);
	void UpdateTextureByIndex(CDeviceClass* devclass, ID3D11ShaderResourceView* tex, int index);
	void UpdateShadowMap(CDeviceClass* devclass, ID3D11ShaderResourceView* shadowmap);
	void UpdateShaderParameters(CDeviceClass *devclass, XMMATRIX& worldMatrix, XMMATRIX& viewMatrix, XMMATRIX& projectionMatrix,
		ID3D11ShaderResourceView* colorTexture, ID3D11ShaderResourceView* normalTexture, ID3D11ShaderResourceView* specularTexture, 
		ID3D11ShaderResourceView* positionTexture, ID3D11ShaderResourceView* roughnessTexture, ID3D11ShaderResourceView* tangentTexture, ID3D11ShaderResourceView* binormalTexture,
		DirectionalLight dlight, std::vector<PointLight> plights);

	void UpdateShaderParameters(CDeviceClass *devclass, XMMATRIX& worldMatrix, XMMATRIX& viewMatrix, XMMATRIX& projectionMatrix,
		DeferredBuffersClass* defBuf, DirectionalLight dlight, std::vector<PointLight> plights);
	void Release();

	void Update(CDeviceClass *devclass, int indexCount);
private:

	void OutputErrorMessage(ID3D10Blob * errorMessage, HWND hWnd, WCHAR * shaderFilename);

	ID3D11VertexShader* m_vertexShader;
	ID3D11PixelShader* m_pixelShader;
	ID3D11InputLayout* m_layout;
	ID3D11SamplerState* m_sampleState;
	ID3D11SamplerState* m_sampleClamp;
	ID3D11Buffer* m_matrixBuffer;
	ID3D11Buffer* m_lightBuffer;
	ID3D11Buffer* m_PointLightBuffer;
	ID3D11Buffer* m_cameraBuffer;
	ID3D11Buffer* m_disneyBuffer;
};

#endif