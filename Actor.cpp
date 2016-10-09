#include "stdafx.h"
#include "Actor.h"


//construct a string to transfer to gui
std::string Actor::ObjectTransmissionString()
{
	std::string transmission;

	XMFLOAT4 LocationData; XMStoreFloat4(&LocationData, this->actorMatrix.position);
	XMFLOAT4 RotationData; XMStoreFloat4(&RotationData, this->actorMatrix.rotation);
	XMFLOAT4 SizeData;     XMStoreFloat4(&SizeData, this->actorMatrix.size);

	transmission += this->ActorPath;

	std::string locS = "+" + std::to_string(LocationData.x) + "+" + std::to_string(LocationData.y) + "+" + std::to_string(LocationData.z) + "+" + std::to_string(LocationData.w) + "+";
	std::string rotS = std::to_string(RotationData.x) + "+" + std::to_string(RotationData.y) + "+" + std::to_string(RotationData.z) + "+" + std::to_string(RotationData.w) + "+";
	std::string sizS = std::to_string(SizeData.x) + "+" + std::to_string(SizeData.y) + "+" + std::to_string(SizeData.z) + "+" + std::to_string(SizeData.w);

	transmission += locS + rotS + sizS;

	return transmission;
}

Actor::Actor(const char * modelpath, CDeviceClass *devclass)
{
	this->devclass = devclass;
	actorMeshes = new Model(modelpath,devclass);

	this->ActorPath = std::string(modelpath);


}

Actor::~Actor()
{
}

inline void Actor::UpdateMatrix()
{
	modelMatrix = XMMatrixIdentity();
	XMMATRIX ScaleMatrix = XMMatrixIdentity();
	XMMATRIX TranslationMatrix = XMMatrixIdentity();
	XMMATRIX RotationMatrix = XMMatrixIdentity();

	TranslationMatrix = XMMatrixTranslationFromVector(actorMatrix.position);

	RotationMatrix = XMMatrixRotationRollPitchYawFromVector(actorMatrix.rotation);
	ScaleMatrix = XMMatrixScalingFromVector(actorMatrix.size);


	modelMatrix = RotationMatrix  * ScaleMatrix *TranslationMatrix;
}

void Actor::SetModelSize(XMVECTOR r)
{
	this->actorMatrix.size = r;
	UpdateMatrix();
}

void Actor::SetDiffuseTexture(char * path)
{
	diffuse = new CTextureTA();
	diffuse->LoadFreeImage(devclass->GetDevice(),devclass->GetDevCon(), path);

	for (size_t i = 0; i < actorMeshes->meshes.size(); i++)
	{
		actorMeshes->meshes[i].UseMeshMaterials = false;
	}
}

void Actor::SetSpecularTexture(char * path)
{
	specular = new CTextureTA();
	specular->LoadFreeImage(devclass->GetDevice(), devclass->GetDevCon(), path);
}

void Actor::SetBumpTexture(char * path)
{
	bump = new CTextureTA();
	bump->LoadFreeImage(devclass->GetDevice(), devclass->GetDevCon(), path);
}

void Actor::RenderShadowMap(CDeviceClass * devclass, CDepthShader *shader)
{
	modelMatrix = XMMatrixIdentity();
	XMMATRIX ScaleMatrix = XMMatrixIdentity();
	XMMATRIX TranslationMatrix = XMMatrixIdentity();
	XMMATRIX RotationMatrix = XMMatrixIdentity();

	TranslationMatrix = XMMatrixTranslationFromVector(actorMatrix.position);

	RotationMatrix = XMMatrixRotationRollPitchYawFromVector(actorMatrix.rotation);
	ScaleMatrix = XMMatrixScalingFromVector(actorMatrix.size);


	modelMatrix = RotationMatrix  * ScaleMatrix *TranslationMatrix;


	for (size_t i = 0; i < this->actorMeshes->meshes.size(); i++)
	{
		actorMeshes->meshes[i].DrawShadow(shader);
	}
}

void Actor::RenderModel(CDeviceClass * devclass, DeferredShader* defshader)
{

	modelMatrix = XMMatrixIdentity();
	XMMATRIX ScaleMatrix = XMMatrixIdentity();
	XMMATRIX TranslationMatrix = XMMatrixIdentity();
	XMMATRIX RotationMatrix = XMMatrixIdentity();

	TranslationMatrix = XMMatrixTranslationFromVector(actorMatrix.position);

	RotationMatrix = XMMatrixRotationRollPitchYawFromVector(actorMatrix.rotation);
	ScaleMatrix = XMMatrixScalingFromVector(actorMatrix.size);


	modelMatrix = RotationMatrix  * ScaleMatrix *TranslationMatrix;

	if (diffuse != nullptr || specular != nullptr || bump != nullptr)
	{
		defshader->UpdateTexture(devclass, diffuse->GetTexture());
		defshader->UpdateTextureSpecular(devclass, specular->GetTexture());
		defshader->UpdateTextureBump(devclass, bump->GetTexture());
	}

	

	for (size_t i = 0; i < actorMeshes->meshes.size(); ++i)
	{

		actorMeshes->meshes[i].DrawMeshGeometry(defshader);
		
	}
}

void Actor::Release()
{
	actorMeshes->Release();
}

void Actor::SetModelPosition(XMVECTOR s)
{
	this->actorMatrix.position = s;
	UpdateMatrix();
}

void Actor::SetModelRotation(XMVECTOR r)
{
	this->actorMatrix.rotation = r;
	UpdateMatrix();
}
