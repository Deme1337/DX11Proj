#pragma once


#ifndef UI_H
#define UI_H

#include "DeviceClass.h"
#include <AntTweakBar.h>
#include <vector>
#include "SceneClass.h"
#include "Lights.h"


class CAntUI
{
public:
	CAntUI();
	~CAntUI();
	TwBar* testBar;
	TwBar* loaderBar;
	TwBar* lightBar;

	SceneClass* MainScene;
	HWND mainWindow;
	int SelectedIndex = 0;

	bool InitializeTW(CDeviceClass *devclass, int width, int height, SceneClass *scene);

	
	void AddVariableString(const char* name, std::string &var);
	void AddVariableXMfloat(const char* name, XMFLOAT4 &var);
	void AddVariableXMfloat(const char* name, XMFLOAT3 &var);

	void AddVariableFloat(const char* name, float &var);
	void AddVariableBoolean(const char* name, bool &var);

	void AddDirectionalLight(DirectionalLight& dlight);
	void AddPointLights(std::vector<PointLight>& plight);
	
	void ShutDownPointLights(const CAntUI *clientData);
	void LoadModel(const CAntUI *clientData);
	void DeleteModel(const CAntUI* clientData);

	void SaveScene(const CAntUI* clientData);
	void LoadScene(const CAntUI* clientData);

	void DrawTW();

	void Release();

	CDeviceClass* m_D3DClass;

private:
	
	int _width, _height;
	std::ofstream SaveFile;
	std::string ModelPathLoad;


};



#endif
