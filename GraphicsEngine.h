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

public: //Functions
	GraphicsEngine();
	~GraphicsEngine();

	void InitializeEngine(HWND hWnd, HINSTANCE hInst);
	void UpdateEngine(int fps, double frameTime);
	void Release();

	void UpdateWindow(int x, int y);

public: //Objects

public: //Variables

	bool _vSyncEnabled = false;
	bool gUseTerrain = false;

private: //Functions

	void SetImgui();
	void GenerateFoliage();

	SIZE_T GetTotalMemory();

	void CustomizeActorSpriteSheet();
	void AddSpriteSheetToObject();

	void LoadMaterialTexture(const char* type);

	void SaveScene();
	bool LoadSceneFromFile();
	void PrepareScene();
	void PrepareTW();
	void GetHwndSize(HWND hWnd, int &width, int &height);

private: //Objects

	TcpClass GuiMessager;
	CDeviceClass *m_D3DDevice;
	SceneClass* m_Scene;
	CAntUI* m_GUI;
	HWND mainWindow;
	Actor* a[11];

	std::vector<Material*> materiallist;
	std::vector<Actor*> LoadedActorList;
	std::ofstream SaveFile;

	TinyTextContext_c textContext;

private: //Variables

	std::vector<std::string> materialNames;
	std::vector<std::string> ActorNames;

	std::string SceneInitTime;
	std::string GPUinfo;
	std::string gSceneName = " ";

	char* sceneNameBuffer = " ";
	const char** actorListNames;

	int SpriteSheetIndex = 0;
	int materialIndex = 0;
	int _sampleCount = 1;

	//Index of the selected object
	int ObjectSelectedIndex = 0;


	bool matListUpdated = false;
	bool FullScreen = false;

	float ssaoBias = 3.0f;
	float ssaoRadius = 32.5f;



};

#endif