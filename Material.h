#pragma once

#ifndef MATERIAL_H
#define MATERIAL_H

#include "DeviceClass.h"
#include "TextureTA.h"

static int materialCounter = 0;

class Material
{
private:
	CTextureTA* AlbedoTexture;
	CTextureTA* SpecularTexture;
	CTextureTA* NormalTexture;
	CTextureTA* RoughnessTexture;


public:
	Material();
	~Material();


	//Types albedo, specular, roughness, normal
	void LoadTexture(CDeviceClass* devclass, const char* path, const char* type);

	//Types albedo, specular, roughness, normal
	CTextureTA* GetTexture(const char* type);


	std::string matname = "";

	void ReleaseMaterial();

};

#endif 