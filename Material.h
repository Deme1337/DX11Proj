#pragma once

#ifndef MATERIAL_H
#define MATERIAL_H

#include "DeviceClass.h"
#include "TextureTA.h"


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


	void ReleaseMaterial();

};

#endif 