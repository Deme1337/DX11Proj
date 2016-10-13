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
		XMFLOAT4 Position = XMFLOAT4(1.0, 1.0, 1.0, 1.0); 
		XMFLOAT4 Color = XMFLOAT4(1.0, 1.0, 1.0, 1.0); 
	};


	XMMATRIX lightViewMat;
	XMMATRIX lightProjectionMat;

public:

	XMFLOAT4 lightProjectionF = XMFLOAT4(5000.0f, 1800, 1800, 1.0f);

	DLight lightProperties;

	void CalcLightViewMatrix();
	void CalcProjectionMatrix();
	XMMATRIX GetLightViewMatrix() { return this->lightViewMat; }
	XMMATRIX GetLightProjectionMatrix() { return this->lightProjectionMat; }



};


class PointLight
{
private:
	struct PLight
	{
		XMFLOAT4 Position = XMFLOAT4(1.0, 1.0, 1.0, 1.0); 
		XMFLOAT4 Color = XMFLOAT4(1.0, 1.0, 1.0, 1.0); 
	};

public:

	PLight lightProperties;

};


inline void DirectionalLight::CalcLightViewMatrix()
{
	XMVECTOR pos = XMLoadFloat4(&lightProperties.Position);

	XMVECTOR lightLookAt = -XMVector3Normalize(pos);
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f);

	XMMATRIX viewMatrix = XMMatrixLookAtLH(pos, pos + lightLookAt, up);

	this->lightViewMat = viewMatrix;
}

inline void DirectionalLight::CalcProjectionMatrix()
{
	XMMATRIX projMat = XMMatrixOrthographicLH(lightProjectionF.y, lightProjectionF.z, 0.1, lightProjectionF.x);

	this->lightProjectionMat = projMat;
}


#endif