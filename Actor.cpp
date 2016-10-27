#include "stdafx.h"
#include "Actor.h"


//construct a string to transfer to gui
std::string Actor::ObjectTransmissionString()
{
	std::string transmission;

	XMFLOAT4 LocationData = actorMatrix.position; //XMStoreFloat4(&LocationData, this->actorMatrix.position);
	XMFLOAT4 RotationData = actorMatrix.rotation; //XMStoreFloat4(&RotationData, this->actorMatrix.rotation);
	XMFLOAT4 SizeData	  = actorMatrix.size;     //XMStoreFloat4(&SizeData,    this->actorMatrix.size);
	  
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
	XMVECTOR pos, rot, siz;

	pos = XMLoadFloat4(&actorMatrix.position);
	rot = XMLoadFloat4(&actorMatrix.rotation);
	siz = XMLoadFloat4(&actorMatrix.size);

	TranslationMatrix = XMMatrixTranslationFromVector(pos);

	RotationMatrix = XMMatrixRotationRollPitchYawFromVector(rot);
	ScaleMatrix = XMMatrixScalingFromVector(siz);


	modelMatrix = RotationMatrix  * ScaleMatrix *TranslationMatrix;
}

void Actor::SetModelSize(XMVECTOR r)
{
	XMStoreFloat4(&actorMatrix.size, r);
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
	UpdateMatrix();

	for (size_t i = 0; i < this->actorMeshes->meshes.size(); i++)
	{
		actorMeshes->meshes[i].DrawShadow(shader);
	}
}

void Actor::RenderModel(CDeviceClass * devclass, DeferredShader* defshader)
{

	UpdateMatrix();


	if (diffuse != nullptr || specular != nullptr || bump != nullptr)
	{
		defshader->UpdateTexture(devclass, diffuse->GetTexture());
		defshader->UpdateTextureSpecular(devclass, specular->GetTexture());
		defshader->UpdateTextureBump(devclass, bump->GetTexture());
	}

	
	defshader->SetObjectData(devclass, UseTextures);
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
	XMStoreFloat4(&actorMatrix.position, s);
	UpdateMatrix();
}

void Actor::SetModelRotation(XMVECTOR r)
{
	XMStoreFloat4(&actorMatrix.rotation, r);
	UpdateMatrix();
}
