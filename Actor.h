#pragma once

#ifndef ACTOR_H
#define ACTOR_H
#include "DeviceClass.h"
#include "Mesh.h"
#include "Model.h"
#include "TextureTA.h"
#include "DeferredShader.h"
#include "LightShader.h"
#include "DeferredBufferClass.h"
#include "DepthShader.h"

class Actor
{
private:
	struct ActorMatrix
	{
		XMFLOAT4 position = XMFLOAT4(0.0, 0.0, 0.0, 1.0);
		XMFLOAT4 rotation = XMFLOAT4(0.0, 0.0, 0.0, 1.0);
		XMFLOAT4 size	  = XMFLOAT4(1.0, 1.0, 1.0, 1.0);
	};



	


	CDeviceClass* devclass;

	CTextureTA* diffuse;
	CTextureTA* specular;
	CTextureTA* bump;

public:
	bool UseTextures = true;
	bool HasAlpha = false;
	std::string ObjectTransmissionString();
	std::string ActorPath;

	//Holder for meshes
	Model *actorMeshes;

	Actor(const char* modelpath,CDeviceClass *devclass);
	~Actor();

	inline void UpdateMatrix();

	void SetModelPosition(XMVECTOR s);
	void SetModelRotation(XMVECTOR r);
	void SetModelSize(XMVECTOR r);

	void SetDiffuseTexture(char *path);
	void SetSpecularTexture(char *path);
	void SetBumpTexture(char *path);

	void RenderShadowMap(CDeviceClass* devclass, CDepthShader *shader);
	void RenderModel(CDeviceClass* devclass, DeferredShader* defshader);
	XMMATRIX modelMatrix;
	ActorMatrix actorMatrix;
	void Release();

};

#endif