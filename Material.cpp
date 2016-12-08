#include "stdafx.h"
#include "Material.h"


Material::Material()
{
}


Material::~Material()
{
}

void Material::LoadTexture(CDeviceClass * devclass, const char * path, const char * type)
{
	if (type == "albedo")
	{
		AlbedoTexture = new CTextureTA();
		AlbedoTexture->LoadFreeImage(devclass->GetDevice(), devclass->GetDevCon(), path);
	}
	if (type == "specular")
	{
		SpecularTexture = new CTextureTA();
		SpecularTexture->LoadFreeImage(devclass->GetDevice(), devclass->GetDevCon(), path);
	}
	if (type == "roughness")
	{
		RoughnessTexture = new CTextureTA();
		RoughnessTexture->LoadFreeImage(devclass->GetDevice(), devclass->GetDevCon(), path);
	}
	if (type == "normal")
	{
		NormalTexture = new CTextureTA();
		NormalTexture->LoadFreeImage(devclass->GetDevice(), devclass->GetDevCon(), path);
	}
}

CTextureTA* Material::GetTexture(const char * type)
{
	if (type == "albedo")
	{
		return AlbedoTexture;
	}
	if (type == "specular")
	{
		return SpecularTexture;
	}
	if (type == "roughness")
	{
		return RoughnessTexture;
	}
	if (type == "normal")
	{
		return NormalTexture;
	}
	else
	{
		MessageBox(NULL, L"Incorrect type return", L"ERROR", MB_OK);
	}
}

void Material::ReleaseMaterial()
{
	if (AlbedoTexture != nullptr)
	{
		AlbedoTexture->Shutdown();
	}
	if (SpecularTexture != nullptr)
	{
		SpecularTexture->Shutdown();
	}
	if (RoughnessTexture != nullptr)
	{
		RoughnessTexture->Shutdown();
	}
	if (NormalTexture != nullptr)
	{
		NormalTexture->Shutdown();
	}
	
}
