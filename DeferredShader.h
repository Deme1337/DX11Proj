#pragma once
#ifndef DEFERRED_H
#define DEFERRED_H

#include <fstream>

#include "DeviceClass.h"



class DeferredShader
{

private:

	struct MatrixBufferType
	{
		XMMATRIX world;
		XMMATRIX view;
		XMMATRIX projection;
		int HasAlpha;
	};

	

public:
	DeferredShader();
	~DeferredShader();

	bool Initialize(CDeviceClass* devclass, WCHAR* vsFilename, WCHAR* psFilename);
	void UpdateTexture(CDeviceClass * devclass, ID3D11ShaderResourceView* texture);
	void UpdateTextureBump(CDeviceClass * devclass, ID3D11ShaderResourceView* texture);
	void UpdateTextureSpecular(CDeviceClass * devclass, ID3D11ShaderResourceView* texture);
	void UpdateTextureRough(CDeviceClass * devclass, ID3D11ShaderResourceView* texture);

	void UpdateShader(CDeviceClass* devclass, XMMATRIX& world, XMMATRIX& view, XMMATRIX& projection, bool HasAlpha);

	void RenderShader(CDeviceClass* devclass, int indexCount);
	
	void Release();
private:

	void OutputShaderErrorMessage(ID3D10Blob*, HWND, WCHAR*);

	ID3D11VertexShader* m_vertexShader;
	ID3D11PixelShader* m_pixelShader;
	ID3D11InputLayout* m_layout;
	ID3D11SamplerState* m_sampleStateWrap;
	ID3D11Buffer* m_matrixBuffer;
};

#endif