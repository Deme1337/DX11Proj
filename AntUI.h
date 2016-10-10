#pragma once


#ifndef UI_H
#define UI_H

#include "DeviceClass.h"
#include <AntTweakBar.h>
#include <vector>
#include "SceneClass.h"


class CAntUI
{
public:
	CAntUI();
	~CAntUI();
	TwBar* testBar;
	TwBar* loaderBar;
	SceneClass* MainScene;
	HWND mainWindow;

	bool InitializeTW(CDeviceClass *devclass, int width, int height, SceneClass *scene);

	
	void AddVariableString(const char* name, std::string &var);
	void AddVariableXMfloat(const char* name, XMFLOAT4 &var);
	void AddVariableFloat(const char* name, float &var);


	void LoadModel(CAntUI *clientData);

	void DrawTW();

	void Release();

private:
	CDeviceClass* m_D3DClass;
	int _width, _height;

	std::string ModelPathLoad;


};



#endif
