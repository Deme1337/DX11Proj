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


	
	m_Scene->InitializeScene(m_D3DDevice,wwidth,wheight, hWnd);

	GuiMessager = TcpClass();
	
	bool result = GuiMessager.InitializeTcpClass();

	this->PrepareScene();
}


//Frame update, first geometry pass and then light pass
void GraphicsEngine::UpdateEngine()
{
	
	m_D3DDevice->Begin();
	m_D3DDevice->AlphaBlendingOff();

	m_Scene->ShadowPass(m_D3DDevice);

	clock_t gclock1 = clock();
	m_Scene->GeometryPass(m_D3DDevice);
	clock_t gclock2 = clock();

	double gTime = gclock2 - gclock1;

	clock_t tclock1 = clock();
	m_Scene->LightPass(m_D3DDevice);
	clock_t tclock2 = clock();

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



	double lTime = tclock2 - tclock1;

	double deltaTime = Timer::GetDeltaTime();
	int fps = 1/deltaTime;
	std::string vpofs = "Frame s:  " + std::to_string(deltaTime) + " FPS: " + std::to_string(fps) + " Pass1: " + std::to_string(gTime) + " Pass2: " + std::to_string(lTime);
	
	SceneInitTime = std::to_string(m_Scene->GeoBenchMarks[0] * pow(10,-6)) + " :: " + std::to_string(m_Scene->GeoBenchMarks[1] * pow(10, -6)) + " :: " + std::to_string(m_Scene->GeoBenchMarks[2] * pow(10, -6));

	textContext.Print(5,15,vpofs.c_str());
	textContext.Print(5, 35, GPUinfo.c_str());
	textContext.Print(5, 55, SceneInitTime.c_str());
	if (GuiMessager.Connected)
	{
		textContext.Print(5, 75, "Connected to UI");
	}
	else
	{
		textContext.Print(5, 75, "Disconnected from UI, press F9 to try connecting");
	}

	textContext.Render();


	m_D3DDevice->GetSwapChain()->Present(0, 0);
}

void GraphicsEngine::Release()
{
	GuiMessager.Release();
	m_D3DDevice->Release();
	m_Scene->Release();
}

void GraphicsEngine::UpdateWindow(int x, int y)
{
	m_D3DDevice->UpdateViewPort(x, y);
}

void GraphicsEngine::PrepareScene()
{

	a[0] = new Actor("Models\\Crytek\\sponza\\sponza.obj", m_D3DDevice);
	a[0]->SetModelSize(XMVectorSet(0.1, 0.1, 0.1, 1.0));
	a[0]->SetModelPosition(XMVectorSet(1, 1, 1, 1.0f));
	m_Scene->AddSceneActor(a[0]);

	//a[1] = new Actor("C:\\models\\cerberus\\cerberus.obj", m_D3DDevice);
	//a[1]->SetModelSize(XMVectorSet(50.0, 50.0, 50.0, 1.0));
	//a[1]->SetModelPosition(XMVectorSet(40, 10, 1, 1.0f));
	//m_Scene->AddSceneActor(a[1]);

}

void GraphicsEngine::GetHwndSize(HWND hWnd, int & width, int & height)
{
	RECT rect;
	GetWindowRect(hWnd, &rect);
	
	width = rect.right - rect.left;
	height = rect.bottom - rect.top;
	
}
