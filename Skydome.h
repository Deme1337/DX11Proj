#pragma once

#ifndef SKYDOME_H
#define SKYDOME_H

#include "DeviceClass.h"
#include "TextureTA.h"
using namespace std;

class CSkydome
{
private:
	struct ModelType
	{
		float x, y, z;
		float tu, tv;
		float nx, ny, nz;
	};

	struct VertexType
	{
		XMFLOAT3 position;
		XMFLOAT2 uv;
		XMFLOAT3 normal;
	};

public:
	CSkydome();
	CSkydome(const CSkydome&);
	~CSkydome();

	bool Initialize(ID3D11Device*);
	void LoadTexture(CDeviceClass *devclass, char* path);
	void Shutdown();
	void Render(ID3D11DeviceContext*);
	CTextureTA* textureSD;
	CTextureTA* textureSDcube;
	int GetIndexCount();
	XMFLOAT4 GetApexColor();
	XMFLOAT4 GetCenterColor();

	//To work with ui
	XMFLOAT4 m_apexColor, m_centerColor;

private:
	bool LoadSkyDomeModel(char*);
	void ReleaseSkyDomeModel();

	bool InitializeBuffers(ID3D11Device*);
	void ReleaseBuffers();
	void RenderBuffers(ID3D11DeviceContext*);

private:
	ModelType* m_model;
	int m_vertexCount, m_indexCount;
	ID3D11Buffer *m_vertexBuffer, *m_indexBuffer;

	
};

#endif