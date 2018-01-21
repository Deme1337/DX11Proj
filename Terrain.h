#pragma once
#ifndef TERRAIN_H
#define TERRAIN_H

#include "DeviceClass.h"
#include <fstream>
#include "TextureTA.h"
#include <vector>

class CTerrain
{
private:
	struct VertexType
	{
		XMFLOAT3 position;
		XMFLOAT2 texture;
		XMFLOAT3 normal;
		XMFLOAT3 tangent;
		XMFLOAT3 binormal;
		XMFLOAT3 color;
	};

	struct HeightMapType
	{
		float x, y, z;
		float nx, ny, nz;
		float r, g, b;
	};

	struct ModelType
	{
		float x, y, z;
		float tu, tv;
		float nx, ny, nz;
		float tx, ty, tz;
		float bx, by, bz;
		float r, g, b;
	};

	struct VectorType
	{
		float x, y, z;
	};

	struct TempVertexType
	{
		float x, y, z;
		float tu, tv;
		float nx, ny, nz;
	};

	struct TerrainMatrix
	{
		XMFLOAT3 terrainPosition = XMFLOAT3(0.0f, 0.0f, 0.0f);
		XMFLOAT3 terrainScale = XMFLOAT3(1.0f, 1.0f, 1.0f);
	};


public:
	CTerrain();
	CTerrain(const CTerrain&);
	~CTerrain();
	bool SetTerrainTextures(CDeviceClass* devclass, char* path1, char* path2);

	bool Initialize(ID3D11Device*, char*);
	void Shutdown();
	bool Render(ID3D11DeviceContext*);

	int GetIndexCount();
	CTextureTA *m_TerrainTexture[2];

	ModelType* m_terrainModel;

	std::vector<XMVECTOR> terrainmodels;
	TerrainMatrix terrainMatrix;

private:
	bool LoadSetupFile(char*);
	bool LoadBitmapHeightMap();
	bool LoadRawHeightMap();
	void ShutdownHeightMap();
	void SetTerrainCoordinates();
	bool CalculateNormals();
	bool LoadColorMap();
	bool BuildTerrainModel();
	void ShutdownTerrainModel();
	void CalculateTerrainVectors();
	void CalculateTangentBinormal(TempVertexType, TempVertexType, TempVertexType, VectorType&, VectorType&);

	bool InitializeBuffers(ID3D11Device*);
	void ShutdownBuffers();
	void RenderBuffers(ID3D11DeviceContext*);

private:
	
	ID3D11Buffer *m_vertexBuffer, *m_indexBuffer;
	int m_vertexCount, m_indexCount;
	int m_terrainHeight, m_terrainWidth;
	float m_heightScale;
	char *m_terrainFilename, *m_colorMapFilename;
	HeightMapType* m_heightMap;
	
};

#endif