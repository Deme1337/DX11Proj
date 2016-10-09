#pragma once

#ifndef LIGHTS_H
#define LIGHTS_H

#include <d3d11_2.h>
#include <d3dcompiler.h>
#include <directxmath.h>
using namespace DirectX;

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")


class DirectionalLight
{

private:

	struct DLight
	{
		XMVECTOR Position = XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);
		XMVECTOR Color = XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);
	};


	XMMATRIX lightViewMat;
	XMMATRIX lightProjectionMat;

public:


	DLight lightProperties;

	void CalcLightViewMatrix();
	void CalcProjectionMatrix(float nearplane, float farplane, int iwidth, int iheight);
	XMMATRIX GetLightViewMatrix() { return this->lightViewMat; }
	XMMATRIX GetLightProjectionMatrix() { return this->lightProjectionMat; }



};



inline void DirectionalLight::CalcLightViewMatrix()
{
	XMVECTOR lightLookAt = -XMVector3Normalize(lightProperties.Position);
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f);

	XMMATRIX viewMatrix = XMMatrixLookAtLH(lightProperties.Position, lightProperties.Position + lightLookAt, up);

	this->lightViewMat = viewMatrix;
}

inline void DirectionalLight::CalcProjectionMatrix(float nearplane, float farplane, int iwidth, int iheight)
{
	XMMATRIX projMat = XMMatrixOrthographicLH(iwidth, iheight, nearplane, farplane);

	this->lightProjectionMat = projMat;
}


#endif