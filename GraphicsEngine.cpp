#include "stdafx.h"
#include "GraphicsEngine.h"
#include "Timer.h"
#include <string>
#include <time.h>
#include <thread>
#include <future>



GraphicsEngine::GraphicsEngine()
{
}


GraphicsEngine::~GraphicsEngine()
{
}


//Initialize engine 
//Parameters: hWnd is the handle to the main window
void GraphicsEngine::InitializeEngine(HWND hWnd, HINSTANCE hInst)
{
	m_D3DDevice = new CDeviceClass();
	m_Scene = new SceneClass();
	FullScreen = true;
	m_D3DDevice->SetAntiAliasing(_sampleCount);
	m_D3DDevice->fullscreen = FullScreen;
	
	int wwidth, wheight;

	if (!FullScreen)
	{
		this->GetHwndSize(hWnd, wwidth, wheight);
	}
	else
	{
		//static full screen resolution for now
		wwidth = 1920;
		wheight = 1080;
		//This is weird
		m_Scene->viewPortOffSet = -420;
	}


	m_D3DDevice->InitDeviceAndSwapChain(hWnd, hInst, wwidth, wheight);

	GPUinfo = m_D3DDevice->GetGPU();

	textContext.SetViewportSize(wwidth, wheight);
	textContext.Init(m_D3DDevice->GetDevice(), m_D3DDevice->GetDevCon(), 128 * 128);

	m_GUI = new CAntUI();
	m_GUI->mainWindow = hWnd;
	m_GUI->InitializeTW(m_D3DDevice, wwidth, wheight, m_Scene);
	
	m_Scene->InitializeScene(m_D3DDevice,wwidth,wheight, hWnd);

	GuiMessager = TcpClass();
	
	bool result = GuiMessager.InitializeTcpClass();

	m_Scene->SceneName = "MainScene1";

	this->PrepareScene();
	
	this->PrepareTW();
}


//Frame update, first geometry pass and then light pass
void GraphicsEngine::UpdateEngine(int fps, double frameTime)
{
	
	m_D3DDevice->Begin();
	m_D3DDevice->AlphaBlendingOff();

	TimeVar shadowTime = timeNow();
	m_Scene->ShadowPass(m_D3DDevice);
	double sTime = duration(timeNow() - shadowTime) * pow(10, -6);

	TimeVar gbTime = timeNow();
	m_Scene->GeometryPass(m_D3DDevice);
	double gTime = duration(timeNow() - gbTime) * pow(10, -6);

	TimeVar lpTime = timeNow();
	m_Scene->LightPass(m_D3DDevice);
	double lTime = duration(timeNow() - lpTime) * pow(10, -6);




	if (Keys::key(VKEY_F9))
	{
		GuiMessager.InitializeTcpClass();
	}
	if (Keys::key(VKEY_F8))
	{
		GuiMessager.UpdateActorList(m_Scene->m_Actors);
		GuiMessager.PrepareTransmission();
		GuiMessager.Send();
	}

	if (Keys::onekey(VKEY_B))
	{
		this->SaveScene();
	}

	if (Keys::onekey(VKEY_UP_ARROW) && ObjectSelectedIndex < m_Scene->m_Actors.size())
	{
		if (ObjectSelectedIndex >= m_Scene->m_Actors.size()-1)
		{
			ObjectSelectedIndex = 0;
		}
		else
		{
			ObjectSelectedIndex += 1;
		}
	
		this->PrepareTW();
	}
	if (Keys::onekey(VKEY_DOWN_ARROW) && ObjectSelectedIndex > 0)
	{
		if (ObjectSelectedIndex <= 0)
		{
			ObjectSelectedIndex = m_Scene->m_Actors.size()-1;
		}
		else
		{
			ObjectSelectedIndex -= 1;
		}
		
		this->PrepareTW();
	}


	m_GUI->DrawTW();

	
	std::string vpofs = "Frame:  " + std::to_string(frameTime) + " FPS: " + std::to_string(fps) + " Pass1: " + std::to_string(gTime) + " Pass2: " + std::to_string(lTime) + " Shadow pass: " + std::to_string(sTime);

	SceneInitTime = "Deferred rt: " + std::to_string(m_Scene->GeoBenchMarks[0] * pow(10, -6)) + " : Mesh: " + std::to_string(m_Scene->GeoBenchMarks[1] * pow(10, -6)) + " : Mesh shader: " + std::to_string(m_Scene->GeoBenchMarks[2] * pow(10, -6));

	textContext.Print(5, 15, vpofs.c_str());
	textContext.Print(5, 35, GPUinfo.c_str());
	textContext.Print(5, 55, SceneInitTime.c_str());



	textContext.Render();

	if (_vSyncEnabled)
	{
		m_D3DDevice->GetSwapChain()->Present(1, 0);
	}
	else
	{
		m_D3DDevice->GetSwapChain()->Present(0, 0);
	}
	
}

void GraphicsEngine::Release()
{
	m_GUI->Release();
	GuiMessager.Release();
	m_D3DDevice->Release();
	m_Scene->Release();
}

void GraphicsEngine::UpdateWindow(int x, int y)
{
	m_D3DDevice->UpdateViewPort(x, y);
}

void GraphicsEngine::SaveScene()
{
	std::string SceneSavePath;

	SceneSavePath = "Scenes\\" + m_Scene->SceneName + ".scenefile";

	std::ofstream SaveFile;

	SaveFile.open(SceneSavePath);

	for (size_t i = 0; i < m_Scene->m_Actors.size(); i++)
	{
		std::string aData = m_Scene->m_Actors[i]->ObjectTransmissionString() + "\n";
		SaveFile << aData;
	}

	SaveFile.close();
}

void GraphicsEngine::PrepareScene()
{

	a[0] = new Actor("Models\\Crytek\\Sponza\\sponza.obj", m_D3DDevice);
	a[0]->SetModelSize(XMVectorSet(0.1, 0.1, 0.1, 1.0));
	a[0]->SetModelPosition(XMVectorSet(1, 1, 1, 1.0f));
	a[0]->HasAlpha = true;
	a[0]->UseTextures = true;
	m_Scene->AddSceneActor(a[0],m_D3DDevice);

}

void GraphicsEngine::PrepareTW()
{
	TwRemoveAllVars(m_GUI->testBar);
	m_GUI->SelectedIndex = ObjectSelectedIndex;
	m_GUI->AddVariableString("Model: ", m_Scene->m_Actors[ObjectSelectedIndex]->ActorPath);

	m_GUI->AddVariableXMfloat("Position: ", m_Scene->m_Actors[ObjectSelectedIndex]->actorMatrix.position);
	m_GUI->AddVariableXMfloat("Rotation: ", m_Scene->m_Actors[ObjectSelectedIndex]->actorMatrix.rotation);
	m_GUI->AddVariableXMfloat("Scale: ", m_Scene->m_Actors[ObjectSelectedIndex]->actorMatrix.size);
	m_GUI->AddVariableBoolean("Alpha cull: ", m_Scene->m_Actors[ObjectSelectedIndex]->HasAlpha);
	m_GUI->AddVariableBoolean("Use textures: ", m_Scene->m_Actors[ObjectSelectedIndex]->UseTextures);
	m_GUI->AddVariableXMfloat("Skydome center color: ", m_Scene->GetSkyDome()->m_centerColor);
	m_GUI->AddVariableXMfloat("Skydome apex color: ", m_Scene->GetSkyDome()->m_apexColor);
	m_GUI->AddVariableXMfloat("Camera RO Pos: ", m_Scene->GetCamera()->CameraPositionXF);
	m_GUI->AddVariableFloat("Sun diameter: ", m_Scene->dirLight.lightProperties.size);

	m_GUI->AddVariableFloat("Blur sigma: ", m_Scene->BlurSigma);

	m_GUI->AddDirectionalLight(m_Scene->dirLight);

	m_GUI->AddPointLights(m_Scene->pointLights);

	
}

void GraphicsEngine::GetHwndSize(HWND hWnd, int & width, int & height)
{
	RECT rect;
	GetWindowRect(hWnd, &rect);
	
	width = rect.right - rect.left;
	height = rect.bottom - rect.top;
	
}
