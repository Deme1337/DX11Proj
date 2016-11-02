#pragma once

#ifndef CUBEMAPFACE_H
#define CUBEMAPFACE_H

#include "DeviceClass.h"


class CubeMapFace
{
public:

	CubeMapFace(ID3D11Device* device, ID3D11Texture2D* texture, DXGI_FORMAT format, int size, int arrayIndex, int mipCount);

	/// <summary>
	/// Finalizes an instance of the <see cref="IBLCubemapFace"/> class.
	/// </summary>
	~CubeMapFace();

	ID3D11RenderTargetView*	renderTargets[7];
};

#endif