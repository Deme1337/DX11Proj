#include "stdafx.h"
#include "AntUI.h"
#include "Actor.h"
#include <sstream>
#include <string>


CAntUI::CAntUI()
{
}


CAntUI::~CAntUI()
{
}



void CAntUI::ShutDownPointLights(CAntUI * clientData)
{
	for (size_t i = 0; i < clientData->MainScene->pointLights.size(); i++)
	{
		clientData->MainScene->pointLights[i].lightProperties.Color = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	}
}

void CAntUI::LoadModel(CAntUI *clientData)
{
	
	std::string Path = "";
	OpenFileDialog* ofd = new OpenFileDialog();
	ofd->Owner = clientData->mainWindow;
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
	
	ofd = nullptr;
	delete ofd;
}

void CAntUI::DeleteModel(CAntUI * clientData)
{
	clientData->MainScene->m_Actors[clientData->SelectedIndex]->Release();
	clientData->MainScene->m_Actors.erase(clientData->MainScene->m_Actors.begin() + clientData->SelectedIndex);
}


//TODO: Add "save file dialog"
void CAntUI::SaveScene(CAntUI * clientData)
{
	std::string SceneSavePath;

	SceneSavePath = "D:\\Graphics-programming\\D3D_Template\\D3D_Template\\Scenes\\MainScene1.scenefile";

	

	SaveFile.open(SceneSavePath);

	if (SaveFile.is_open())
	{

		for (size_t i = 0; i < clientData->MainScene->m_Actors.size(); i++)
		{
			std::string aData = clientData->MainScene->m_Actors[i]->ObjectTransmissionString() + "\n";
			SaveFile << aData;
		}

		SaveFile.close();
	}
	else
	{
	
		MessageBox(NULL, L"ERROR CANNOT OPEN FILE", L"ERROR", MB_OK);
	
		SaveFile.close();
	}

	SceneSavePath = "";
}

void CAntUI::LoadScene(CAntUI * clientData)
{
	std::vector<std::string> lines;

	std::string Path = "";
	OpenFileDialog* ofd = new OpenFileDialog();
	ofd->Owner = clientData->mainWindow;
	if (ofd->ShowDialog() && ofd->FileName != nullptr)
	{
		std::wstring p(ofd->FileName);
		Path = ws2s(p);

		std::ifstream inFile(Path);
		std::string line;
		while (std::getline(inFile, line))
		{
			lines.push_back(line);
		}

		clientData->MainScene->m_Actors.clear();

		for (size_t i = 0; i < lines.size(); i++)
		{
			std::vector<std::string> astring = split(lines[i], '+');
			std::string::size_type sz;
			Actor *a = new Actor(astring[0].c_str(), clientData->m_D3DClass);
			a->actorMatrix.position.x = std::stof(astring[1], &sz);
			a->actorMatrix.position.y = std::stof(astring[2], &sz);
			a->actorMatrix.position.z = std::stof(astring[3], &sz);

			a->actorMatrix.rotation.x = std::stof(astring[5], &sz);
			a->actorMatrix.rotation.y = std::stof(astring[6], &sz);
			a->actorMatrix.rotation.z = std::stof(astring[7], &sz);

			a->actorMatrix.size.x = std::stof(astring[9], &sz);
			a->actorMatrix.size.y = std::stof(astring[10], &sz);
			a->actorMatrix.size.z = std::stof(astring[11], &sz);

			clientData->MainScene->AddSceneActor(a, m_D3DClass);
		}



		clientData->m_D3DClass->SetFullScreen();
	}

	else
	{
		clientData->m_D3DClass->SetFullScreen();
		return;
	}

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

void TW_CALL LoadSceneS(void *clientData)
{
	CAntUI *ca = (CAntUI*)clientData;
	ca->LoadScene(ca);
}


void TW_CALL PointLightOff(void *clientData)
{
	CAntUI *ca = (CAntUI*)clientData;
	ca->ShutDownPointLights(ca);
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
	


	TwDefine(" Main label = 'Scene Parameters' position = '1200 10' size='600 300'");


	loaderBar = TwNewBar("Loader");

	lightBar = TwNewBar("Lights");

	TwDefine(" Loader label = 'Object load' position = '1600 500'");

	TwDefine(" Lights label = 'Light properties' position = '500 800' size='500 400' ");

	TwAddVarRW(loaderBar, "Load: ", TW_TYPE_STDSTRING, &ModelPathLoad, "");

	TwAddButton(loaderBar, "Load Model", LoadModelS, this, "");
	TwAddButton(loaderBar, "Delete Model", DeleteActor, this, "");
	TwAddButton(loaderBar, "Save Scene",SaveSceneS , this, "");
	TwAddButton(loaderBar, "Load Scene", LoadSceneS, this, "");
	TwAddButton(loaderBar, "Point light off", PointLightOff, this, "");
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

void CAntUI::AddDirectionalLight(DirectionalLight& dlight)
{
	TwAddVarRW(lightBar, "Light position: ", TW_TYPE_DIR3F, &dlight.lightProperties.Position, "");
	TwAddVarRW(lightBar, "Light color: ", TW_TYPE_DIR3F, &dlight.lightProperties.Color, "");
	TwAddVarRW(lightBar, "Light projection matrix: ", TW_TYPE_DIR3F, &dlight.lightProjectionF, "");
}

void CAntUI::AddPointLights(std::vector<PointLight> &plight)
{
	for (size_t i = 0; i < plight.size(); i++)
	{
		std::string name1 = "Plight position: " + std::to_string(i) + ": ";
		std::string name2 = "Plight Color: " + std::to_string(i) + ": ";
		TwAddVarRW(lightBar, name1.c_str(), TW_TYPE_DIR3F, &plight[i].lightProperties.Position, "");
		TwAddVarRW(lightBar, name2.c_str(), TW_TYPE_DIR3F, &plight[i].lightProperties.Color, "");
	}
}

void CAntUI::DrawTW()
{
	TwDraw();
}

void CAntUI::Release()
{
	TwTerminate();
}


