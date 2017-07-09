#include "stdafx.h"
#include "GraphicsEngine.h"
#include "Timer.h"
#include <string>
#include <time.h>
#include <Psapi.h>
#include <thread>
#include <sstream>
#include <future>


#include "imgui-master\imgui_impl_dx11.h"




namespace ImGui
{
	static auto vector_getter = [](void* vec, int idx, const char** out_text)
	{
		auto& vector = *static_cast<std::vector<std::string>*>(vec);
		if (idx < 0 || idx >= static_cast<int>(vector.size())) { return false; }
		*out_text = vector.at(idx).c_str();
		return true;
	};

	bool Combo1(const char* label, int* currIndex, std::vector<std::string>& values)
	{
		if (values.empty()) { return false; }
		return Combo(label, currIndex, vector_getter,
			static_cast<void*>(&values), values.size());
	}


	bool ListBox(const char* label, int* currIndex, std::vector<std::string>& values)
	{
		if (values.empty()) { return false; }
		return ListBox(label, currIndex, vector_getter,
			static_cast<void*>(&values), values.size());
	}
}

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
	FullScreen = false;
	m_D3DDevice->SetAntiAliasing(_sampleCount);
	m_D3DDevice->fullscreen = FullScreen;

	this->mainWindow = hWnd;
	
	int wwidth, wheight;

	if (!FullScreen)
	{
		this->GetHwndSize(hWnd, wwidth, wheight);
		m_Scene->viewPortOffSet = -420;
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

	//IMGUI test
	{
		ImGui_ImplDX11_Init(hWnd, m_D3DDevice->GetDevice(), m_D3DDevice->GetDevCon());
		ImVec4 clear_col = ImColor(114, 144, 154);
	}
	
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


	//Imgui rendering
	{
		ImGui_ImplDX11_NewFrame();
		ImGui::Begin("Tools");
		ImGui::SetWindowSize(ImVec2(350, 500));
		ImGui::SliderFloat("Blur sigma", &m_Scene->BlurSigma, 0.0f, 50.0f);

		ImGui::Separator();

		//Just to keep index bound in actorlist size
		if (ObjectSelectedIndex >= m_Scene->m_Actors.size())
		{
			ObjectSelectedIndex = 0;
		}

		if (m_Scene->m_Actors.size() > 0 && m_Scene->m_Actors[ObjectSelectedIndex] != nullptr)
		{
			ImGui::Text(m_Scene->m_Actors[ObjectSelectedIndex]->actorFile.c_str());
			ImGui::SliderFloat("Actor roughness", &m_Scene->m_Actors[ObjectSelectedIndex]->actorMatrix.roughness, -1.0f, 1.0f);
			ImGui::SliderFloat("Metallic", &m_Scene->m_Actors[ObjectSelectedIndex]->actorMatrix.metallic, 0.0f, 1.0f);
			ImGui::InputFloat("Texture padding", &m_Scene->m_Actors[ObjectSelectedIndex]->actorMatrix.texOffset, -20.0f, 20.0f);
		}

		ImGui::SliderFloat("Subsurface", &m_Scene->subspectintani.x, 0.0f, 1.0f);
		ImGui::SliderFloat("specular", &m_Scene->subspectintani.y, 0.0f, 1.0f);
		ImGui::SliderFloat("specularTint", &m_Scene->subspectintani.z, 0.0f, 1.0f);
		ImGui::SliderFloat("anisotropic", &m_Scene->subspectintani.w, 0.0f, 1.0f);
		ImGui::SliderFloat("sheen", &m_Scene->sheentintcleargloss.x, 0.0f, 1.0f);
		ImGui::SliderFloat("sheenTint", &m_Scene->sheentintcleargloss.y, 0.0f, 1.0f);
		ImGui::SliderFloat("clearcoat", &m_Scene->sheentintcleargloss.z, 0.0f, 1.0f);
		ImGui::SliderFloat("clearcoatGloss", &m_Scene->sheentintcleargloss.w, 0.0f, 1.0f);

		ImGui::End();

		SetImgui();

	

		//Debugging only
		//ImGui::SliderInt("Screen offset", &m_Scene->viewPortOffSet, -1000, 1000);

	}



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

	std::string opt1 = "Press p to take a screen shot";

	std::string memoryUsage = "";

	DWORDLONG memUsage = GetTotalMemory() / 1024 / 1024;
	std::ostringstream memStream;
	memStream << memUsage;
	memoryUsage = memStream.str() + " mb";

	textContext.Print(5, 15, vpofs.c_str());
	textContext.Print(5, 35, GPUinfo.c_str());
	textContext.Print(5, 55, SceneInitTime.c_str());
	textContext.Print(5, 75, memoryUsage.c_str());







	//Imgui rendering
	{
		ImGui::Render();
	}


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
	ImGui::Shutdown();
	m_Scene->Release();
	for (size_t i = 0; i < materiallist.size(); i++)
	{
		materiallist[i]->ReleaseMaterial();
	}
	
}

void GraphicsEngine::UpdateWindow(int x, int y)
{
	m_D3DDevice->UpdateViewPort(x, y);
}

void GraphicsEngine::SetImgui()
{
	ImGui::SetNextWindowSize(ImVec2(350, 300));
	ImGui::SetNextWindowPos(ImVec2(10, 710));
	ImGui::Begin("Materials");

	if (ImGui::Button("Create new Material"))
	{
		Material *mat = new Material();
		materiallist.push_back(mat);
		materialNames.clear();
		for (size_t i = 0; i < materiallist.size(); i++)
		{
			materialNames.push_back(materiallist[i]->matname);
		}
	}


	if (materiallist.size() > 0)
	{
		
		ImGui::Combo1("Materials", &materialIndex, materialNames);
	
		
		

		if (ImGui::Button("Create albedo"))
		{
			LoadMaterialTexture("albedo");
		}
		if (materiallist[materialIndex]->GetTexture("albedo") != nullptr)
		{
			ImGui::Text(materiallist[materialIndex]->GetTexture("albedo")->textureName.c_str());
		}

		if (ImGui::Button("Create metallic/spec"))
		{
			LoadMaterialTexture("specular");
		}
		if (materiallist[materialIndex]->GetTexture("specular") != nullptr)
		{
			ImGui::Text(materiallist[materialIndex]->GetTexture("specular")->textureName.c_str());
		}

		if (ImGui::Button("Create normal"))
		{
			LoadMaterialTexture("normal");
		}
		if (materiallist[materialIndex]->GetTexture("normal") != nullptr)
		{
			ImGui::Text(materiallist[materialIndex]->GetTexture("normal")->textureName.c_str());
		}

		if (ImGui::Button("Create roughness"))
		{
			LoadMaterialTexture("roughness");
		}
		if (materiallist[materialIndex]->GetTexture("roughness") != nullptr)
		{
			ImGui::Text(materiallist[materialIndex]->GetTexture("roughness")->textureName.c_str());
			
		}


		if (ImGui::Button("Set material") )
		{
			m_Scene->m_Actors[ObjectSelectedIndex]->SetMaterial(materiallist[materialIndex]);
		}

		if (ImGui::Button("UnSet material"))
		{
			m_Scene->m_Actors[ObjectSelectedIndex]->UnsetMaterial();
		}

	}


	
	ImGui::End();
}

SIZE_T GraphicsEngine::GetTotalMemory()
{
	PROCESS_MEMORY_COUNTERS pmc;
	GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc));
	SIZE_T virtualMemUsedByMe = pmc.WorkingSetSize;

	return virtualMemUsedByMe;
}


void GraphicsEngine::LoadMaterialTexture(const char* type)
{
	OpenFileDialog* ofd = new OpenFileDialog();
	
	ofd->Owner = this->mainWindow;
	if (ofd->ShowDialog() && ofd->FileName != nullptr)
	{
		std::string Path;
		std::wstring p(ofd->FileName);
		Path = ws2s(p);
		
		materiallist[materialIndex]->LoadTexture(m_D3DDevice, Path.c_str(), type);
	}

	else
	{

		return;
	}


	ofd = nullptr;
	delete ofd;
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
	a[0]->actorMatrix.roughness = 0.80f;
	a[0]->UseTextures = true;
	m_Scene->AddSceneActor(a[0],m_D3DDevice);


	m_Scene->BlurSigma = 0.0f;


}

void GraphicsEngine::PrepareTW()
{
	TwRemoveAllVars(m_GUI->testBar);
	m_GUI->SelectedIndex = ObjectSelectedIndex;
	m_GUI->AddVariableString("Model: ", m_Scene->m_Actors[ObjectSelectedIndex]->actorFile);

	m_GUI->AddVariableXMfloat("Position: ", m_Scene->m_Actors[ObjectSelectedIndex]->actorMatrix.position);
	m_GUI->AddVariableXMfloat("Rotation: ", m_Scene->m_Actors[ObjectSelectedIndex]->actorMatrix.rotation);
	m_GUI->AddVariableXMfloat("Scale: ", m_Scene->m_Actors[ObjectSelectedIndex]->actorMatrix.size);
	m_GUI->AddVariableXMfloat("Color: ", m_Scene->m_Actors[ObjectSelectedIndex]->actorMatrix.objColor);
	m_GUI->AddVariableBoolean("Alpha cull: ", m_Scene->m_Actors[ObjectSelectedIndex]->HasAlpha);
	m_GUI->AddVariableBoolean("Use textures: ", m_Scene->m_Actors[ObjectSelectedIndex]->UseTextures);
	
	m_GUI->AddVariableXMfloat("Skydome center color: ", m_Scene->GetSkyDome()->m_centerColor);
	m_GUI->AddVariableXMfloat("Skydome apex color: ", m_Scene->GetSkyDome()->m_apexColor);
	m_GUI->AddVariableXMfloat("Camera RO Pos: ", m_Scene->GetCamera()->CameraPositionXF);
	m_GUI->AddVariableFloat("Sun Pow: ", m_Scene->dirLight.lightProperties.sunPower);
	m_GUI->AddVariableFloat("Sun scale1: ", m_Scene->dirLight.lightProperties.scale1);
	m_GUI->AddVariableFloat("Sun scale2: ", m_Scene->dirLight.lightProperties.scale2);
	m_GUI->AddVariableFloat("GlobalAmbient: ", m_Scene->dirLight.lightProperties.globalAmbient);
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
