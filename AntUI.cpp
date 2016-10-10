#include "stdafx.h"
#include "AntUI.h"


CAntUI::CAntUI()
{
}


CAntUI::~CAntUI()
{
}

bool CAntUI::InitializeTW(CDeviceClass *devclass, int width, int height)
{
	TwInit(TW_DIRECT3D11, devclass->GetDevice());

	this->_height = height;
	this->_width = width;

	TwWindowSize(width, height);

	testBar = TwNewBar("Main");
	

	TwDefine(" Main label = 'Scene Parameters' position = '1600 10'");

	return true;
}

void CAntUI::AddVariableString(const char * name, std::string &var)
{
	TwAddVarRW(testBar, name, TW_TYPE_STDSTRING, &var, "");
}

void CAntUI::AddVariableXMfloat(const char * name, XMFLOAT4 &var)
{
	TwAddVarRW(testBar, name, TW_TYPE_DIR3F, &var, "");
}

void CAntUI::AddVariableFloat(const char * name, float & var)
{
	TwAddVarRW(testBar, name, TW_TYPE_FLOAT, &var, "");
}

void CAntUI::DrawTW()
{
	TwDraw();
}

void CAntUI::Release()
{
	TwTerminate();
}
