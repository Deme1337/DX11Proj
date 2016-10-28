#include "stdafx.h"
#include "FreeCamera.h"
#include "Input.h"


FreeCamera::FreeCamera(HWND hWnd)
{
	this->_viewport = hWnd;
	lookAt = XMVectorSet(0.0f, 0.0f, 1.0f, 1.0f);
	position = XMVectorSet(0.0f, 20.0f, -50.0f, 1.0f);
	up = XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f);

	camView = XMMatrixLookAtLH(position, position + lookAt, up);
}

XMMATRIX FreeCamera::GetBaseViewMatrix()
{
	XMMATRIX baseViewMatrix = XMMatrixIdentity();
	XMFLOAT3 upb = XMFLOAT3(0.0f, 1.0f, 0.0f);
	XMVECTOR positionb = position;
	XMFLOAT3 lookatB; XMFLOAT3 positionFloat; 

	XMStoreFloat3(&positionFloat, position);
	XMStoreFloat3(&lookatB, lookAt);

	lookatB.y = positionFloat.y;
	XMVECTOR lookatBV = XMLoadFloat3(&lookatB);

	baseViewMatrix = XMMatrixLookAtLH(positionb, lookatBV, up);

	return baseViewMatrix;
}

void FreeCamera::SetCameraPosition(XMVECTOR pos)
{
	this->position = pos;
}


XMMATRIX FreeCamera::GetCameraView()
{
	return this->camView;
}


// to give the mouse some area to move
static POINT GetScreenCenter()
{
	POINT ret;

	ret.x = GetSystemMetrics(SM_CXSCREEN) / 2;
	ret.y = GetSystemMetrics(SM_CYSCREEN) / 2;

	return ret;
}


void FreeCamera::UpdateCamera()
{
	if (Keys::key(VKEY_PLUS))
	{
		Speedup();
	}
	if (Keys::key(VKEY_MINUS))
	{
		Slow();
	}

	//Capture mouse position each frame even if mouse right btn is not pressed
	POINT cPos;
	GetCursorPos(&cPos);
	ScreenToClient(_viewport, &cPos);

	XMStoreFloat4(&CameraPositionXF, position);

	if (Keys::key(VKEY_RIGHT_BUTTON))
	{
		
		RECT rRect; GetClientRect(_viewport, &rRect);
		width = rRect.right;
		height = rRect.bottom;


		int centx = width / 2;
		int centy = height / 2;
		
		
		//Rotate camera only if mouse is moved outside "deadzone"
		if (centx - cPos.x > 5 || centx - cPos.x < -5 || centy - cPos.y > 5 || centy - cPos.y < -5)
		{
			horizontalangle -= (mousespeed * float(pLastPos.x - cPos.x)) * 60.0f * Timer::GetDeltaTime();
			verticalangle   += (mousespeed * float(pLastPos.y - cPos.y)) * 60.0f * Timer::GetDeltaTime();
			
		}
	
		
	
		float xCos = std::cos(verticalangle) * std::sin(horizontalangle);
		float ySin = std::sin(verticalangle);
		float zCos = std::cos(verticalangle) * std::cos(horizontalangle);
		
		XMFLOAT3 dir = XMFLOAT3(xCos, ySin, zCos);
		
		lookAt = XMLoadFloat3(&dir);
		
		right = XMVectorSet(
			std::sin(horizontalangle - XM_PI / 2.0f),
			0.0f, 
			std::cos(horizontalangle - XM_PI / 2.0f), 1.0f);

		up = XMVector3Cross(right, lookAt);

		if (Keys::key(VKEY_W)) position += lookAt  * speed * Timer::GetDeltaTime();
		if (Keys::key(VKEY_S)) position -= lookAt  * speed * Timer::GetDeltaTime();
		if (Keys::key(VKEY_A)) position += right   * speed * Timer::GetDeltaTime();
		if (Keys::key(VKEY_D)) position -= right   * speed * Timer::GetDeltaTime();

		camView = XMMatrixLookAtLH(position, position + lookAt, up);
	}

	//set cursor last position 
	pLastPos = cPos;
	
}


void FreeCamera::Speedup()
{
	if (speed < MAX_SPEED)
	{
		speed += 0.05f;
	}
	
}

void FreeCamera::Slow()
{
	if (speed > MIN_SPEED)
	{
		speed -= 0.05f;
	}
	
}

FreeCamera::~FreeCamera()
{
}
