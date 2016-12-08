#pragma once

//cubemap code https://dirkiek.wordpress.com/category/graphics-programming/


#ifndef CUBEMAP_H
#define CUBEMAP_H

#include "DeviceClass.h"
#include "CubeMapFace.h"

class CubeMap
{
private:
	ID3D11ShaderResourceView* shaderResourceView;
	ID3D11Texture2D *texture;

public:
	CubeMap(CDeviceClass *devclass, int mips, int size);
	~CubeMap();

	ID3D11ShaderResourceView* GetShaderResourceViewAddress()
	{
		return this->shaderResourceView;
	}

	CubeMapFace* surfaces[6];

	void RenderCubeMap(CDeviceClass* devclass);

};

#endif