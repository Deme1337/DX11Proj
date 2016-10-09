#pragma once
#ifndef FREECAM_H
#define FREECAM_H
#include <directxmath.h>
#include "Timer.h"

using namespace DirectX;

#define MAX_SPEED 500.0f
#define MIN_SPEED 0.5f

class FreeCamera
{
public:

	//constructor sets main window and initial values for camera
	FreeCamera(HWND hWnd);

	void UpdateCamera();
	//sets camera position
	void SetCameraPosition(XMVECTOR pos);
	~FreeCamera();
	XMVECTOR GetCameraPosition(){ return position; }
	//returns camera view matrix
	XMMATRIX GetCameraView();
	XMVECTOR lookAt;
	XMMATRIX GetBaseViewMatrix();

private:
	void Speedup();
	void Slow();
	HWND _viewport;
	float horizontalangle = 0.0f;
	float verticalangle = 0.0f;
	float mousespeed = 0.004f;
	float speed = 100.0f;
	int width = 0;
	int height = 0;

	POINT pLastPos, pInitialPos;

	
	XMVECTOR position;
	XMVECTOR up;
	XMVECTOR right;


public:
	float GetCameraSpeed(){ return speed; };
	XMMATRIX camView;
};

#endif