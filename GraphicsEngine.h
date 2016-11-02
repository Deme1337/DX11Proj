#pragma once

#ifndef GRAPHICS_ENGINE_H
#define GRAPHICS_ENGINE_H

#include "DeviceClass.h"
#include "TinyText.h"
#include "SceneClass.h"
#include "TcpClass.h"
#include "AntUI.h"

class GraphicsEngine
{
private:
	TcpClass GuiMessager;
public:
	GraphicsEngine();
	~GraphicsEngine();

	void InitializeEngine(HWND hWnd, HINSTANCE hInst);
	void UpdateEngine(int fps, double frameTime);
	void Release();

	void UpdateWindow(int x, int y);
	bool _vSyncEnabled = true;
private:

	void SaveScene();

	void PrepareScene();
	void PrepareTW();
	void GetHwndSize(HWND hWnd, int &width, int &height);

	CDeviceClass *m_D3DDevice;

	SceneClass* m_Scene;
	CAntUI* m_GUI;

	std::string GPUinfo;

	TinyTextContext_c textContext;
	int _sampleCount = 1;
	bool FullScreen = false;
	int ObjectSelectedIndex = 0;
	

	std::string SceneInitTime;
	Actor* a[11];
};

#endif