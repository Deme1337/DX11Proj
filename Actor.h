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
		XMVECTOR position = XMVectorSet(0.0, 0.0, 0.0, 1.0);
		XMVECTOR rotation = XMVectorSet(0.0, 0.0, 0.0, 1.0);
		XMVECTOR size	  = XMVectorSet(1.0, 1.0, 1.0, 1.0);
	};


	//Holder for meshes
	Model *actorMeshes;

	std::string ActorPath;

	CDeviceClass* devclass;

	CTextureTA* diffuse;
	CTextureTA* specular;
	CTextureTA* bump;

public:

	std::string ObjectTransmissionString();

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