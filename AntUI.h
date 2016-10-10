#pragma once


#ifndef UI_H
#define UI_H

#include "DeviceClass.h"
#include <AntTweakBar.h>
#include <vector>


class CAntUI
{
public:
	CAntUI();
	~CAntUI();

	bool InitializeTW(CDeviceClass *devclass, int width, int height);

	
	void AddVariableString(const char* name, std::string &var);
	void AddVariableXMfloat(const char* name, XMFLOAT4 &var);
	void AddVariableFloat(const char* name, float &var);



	void DrawTW();

	void Release();

private:
	CDeviceClass* m_D3DClass;
	int _width, _height;

	TwBar* testBar;

	std::vector<TwBar*> twBars;

};

#endif
