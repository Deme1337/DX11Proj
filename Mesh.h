#pragma once
#ifndef MESH_H
#define MESH_H


#include "Ext\Assimp\include\assimp\Importer.hpp"
#include "Ext\Assimp\include\assimp\scene.h"
#include "Ext\Assimp\include\assimp\postprocess.h"

#include "DeferredShader.h"
#include "DepthShader.h"
#include "TextureTA.h"
#include "DeviceClass.h"
#include "Timer.h"
#include <vector>


struct Vertex
{
	XMFLOAT3 Position;
	XMFLOAT2 TexCoords;
	XMFLOAT3 Normal;
	XMFLOAT3 Tangent;
	XMFLOAT3 BiTangent;
};

struct Texture
{
	CTextureTA *tex;
	std::string type;
	aiString path;
};



class Mesh
{
public:
	Mesh(std::vector<Vertex> vertices, std::vector<uint32> indices, std::vector<Texture> textures, CDeviceClass* devc);
	~Mesh();
	Mesh();
	std::vector<Vertex> vertices;
	std::vector<uint32> indices;
	std::vector<Texture> textures;

	uint32 GetIndices() { return this->indices.size(); }

	void DrawMeshGeometry(DeferredShader* defshader);

	/*
	void DrawIndicesToShader(CTextureRenderShader *shader);
	void Draw(TextureShaderClass* shader);
	*/

	void DrawShadow(CDepthShader *shader);
	void Release();
	bool UseMeshMaterials = true;
private:
	ID3D11Buffer *m_vertexBuffer, *m_indexBuffer;
	CDeviceClass* DevClass;
	void SetupMesh();
	
};

#endif