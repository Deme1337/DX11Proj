#include "stdafx.h"
#include "AntUI.h"
#include "Actor.h"



CAntUI::CAntUI()
{
}


CAntUI::~CAntUI()
{
}



void CAntUI::LoadModel(CAntUI *clientData)
{
	
	std::string Path = "";
	OpenFileDialog* ofd = new OpenFileDialog();

	if (ofd->ShowDialog() && ofd->FileName != nullptr)
	{
		std::wstring p(ofd->FileName);
		Path = ws2s(p);

		Actor* a = new Actor(Path.c_str(), clientData->m_D3DClass);
		MainScene->AddSceneActor(a,clientData->m_D3DClass);
		clientData->m_D3DClass->SetFullScreen();
	}

	else
	{
		clientData->m_D3DClass->SetFullScreen();
		return;
	}
}

void CAntUI::DeleteModel(CAntUI * clientData)
{
	clientData->MainScene->m_Actors[clientData->SelectedIndex]->Release();
	clientData->MainScene->m_Actors.erase(clientData->MainScene->m_Actors.begin() + clientData->SelectedIndex);
}

void CAntUI::SaveScene(CAntUI * clientData)
{
	std::string SceneSavePath;

	SceneSavePath = "Scenes\\" + clientData->MainScene->SceneName + ".scenefile";

	clientData->MainScene->SaveScene(SceneSavePath);

}

void TW_CALL SaveSceneS(void *clientData)
{
	CAntUI *ca = (CAntUI*)clientData;
	ca->SaveScene(ca);
}

void TW_CALL DeleteActor(void *clientData)
{
	CAntUI *ca = (CAntUI*)clientData;
	ca->DeleteModel(ca);
}

void TW_CALL LoadModelS(void *clientData)
{
	CAntUI *ca = (CAntUI*)clientData;
	ca->LoadModel(ca);
}

bool CAntUI::InitializeTW(CDeviceClass *devclass, int width, int height,SceneClass *scene)
{
	TwInit(TW_DIRECT3D11, devclass->GetDevice());

	this->_height = height;
	this->_width = width;
	this->MainScene = scene;
	this->m_D3DClass = devclass;

	TwWindowSize(width, height);

	testBar = TwNewBar("Main");
	


	TwDefine(" Main label = 'Scene Parameters' position = '1600 10'");


	loaderBar = TwNewBar("Loader");


	TwDefine(" Loader label = 'Object load' position = '1600 500'");


	TwAddVarRW(loaderBar, "Load: ", TW_TYPE_STDSTRING, &ModelPathLoad, "");

	TwAddButton(loaderBar, "Load Model", LoadModelS, this, "");
	TwAddButton(loaderBar, "Delete Model", DeleteActor, this, "");
	TwAddButton(loaderBar, "Save Scene",SaveSceneS , this, "");
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

void CAntUI::AddVariableBoolean(const char * name, bool & var)
{
	TwAddVarRW(testBar, name, TW_TYPE_BOOL32, &var, "");
}

void CAntUI::DrawTW()
{
	TwDraw();
}

void CAntUI::Release()
{
	TwTerminate();
}


