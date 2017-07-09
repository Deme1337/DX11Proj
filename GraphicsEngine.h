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
	bool _vSyncEnabled = false;

private:

	std::vector<Material*> materiallist;

	void SetImgui();


	SIZE_T GetTotalMemory();

	void LoadMaterialTexture(const char* type);

	void SaveScene();

	void PrepareScene();
	void PrepareTW();
	void GetHwndSize(HWND hWnd, int &width, int &height);

	CDeviceClass *m_D3DDevice;

	SceneClass* m_Scene;
	CAntUI* m_GUI;
	HWND mainWindow;

	const char** actorListNames;
	std::vector<std::string> materialNames;

	
	std::string GPUinfo;

	TinyTextContext_c textContext;
	int _sampleCount = 1;
	bool FullScreen = false;
	unsigned int ObjectSelectedIndex = 0;
	int materialIndex = 0;
	bool matListUpdated = false;

	std::string SceneInitTime;
	Actor* a[11];
};

#endif