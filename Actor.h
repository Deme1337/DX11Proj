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
#include "Material.h"

class Actor
{
private:
	struct ActorMatrix
	{
		XMFLOAT4 position = XMFLOAT4(0.0, 0.0, 0.0, 1.0);
		XMFLOAT4 rotation = XMFLOAT4(0.0, 0.0, 0.0, 1.0);
		XMFLOAT4 size	  = XMFLOAT4(1.0, 1.0, 1.0, 1.0);
		XMFLOAT4 objColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		float texOffsetx = 1.0f;
		float texOffsety = 1.0f;
		float roughness = 0;
		float metallic = 0.003f;
	};



	unsigned int SpriteAnimationCounter = 0;
	unsigned int SpriteAnimationIntervalCounter = 0;


	CDeviceClass* devclass;

	//obsolete
	CTextureTA* diffuse;
	CTextureTA* specular;
	CTextureTA* bump;
	//obsolete

	

public:
	std::vector<Material*> spriteSheet;
	unsigned int SpriteAnimationInterval = 5;
	bool UseAnimatedSpriteSheet = false;
	
	Material *objectMaterial;

	bool useMaterial = false;

	bool UseTextures = true;
	bool HasAlpha = false;
	bool HasShadow = true;

	std::string ObjectTransmissionString();
	std::string ActorPath;
	std::string actorFile;
	//Holder for meshes
	Model *actorMeshes;

	Actor(const char* modelpath,CDeviceClass *devclass);
	Actor(const Actor &m, CDeviceClass* devclass);
	~Actor();

	void SetMeshUseCustomMaterial();
	void UnSetMeshUseCustomMaterial();

	inline void UpdateMatrix();

	void Animate(DeferredShader* defshader);

	void SetModelPosition(XMVECTOR s);
	void SetModelRotation(XMVECTOR r);
	void SetModelSize(XMVECTOR r);

	void AppendSpriteSheet(std::string path);
	void SetMaterial(Material* m);
	void UnsetMaterial();


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