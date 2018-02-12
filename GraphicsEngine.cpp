#include "stdafx.h"
#include "GraphicsEngine.h"
#include "Timer.h"
#include <string>
#include <time.h>
#include <Psapi.h>
#include <thread>
#include <sstream>
#include <future>
#include <algorithm>


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

	static auto vector_getter1 = [](void* vec, int idx, const char** out_text)
	{
		auto &vector = *static_cast<std::vector<Material*>*>(vec);
		if (idx < 0 || idx >= static_cast<int>(vector.size())) { return false; }
		*out_text = vector[idx]->matname.c_str();
		return true;
	};

	static auto vector_getterActors = [](void* vec, int idx, const char** out_text)
	{
		auto &vector = *static_cast<std::vector<Actor*>*>(vec);
		if (idx < 0 || idx >= static_cast<int>(vector.size())) { return false; }
		*out_text = vector[idx]->actorFile.c_str();
		return true;
	};

	bool Combo1(const char* label, int* currIndex, std::vector<std::string>& values)
	{
		if (values.empty()) { return false; }
		return Combo(label, currIndex, vector_getter,
			static_cast<void*>(&values), values.size());
	}


	bool Combo2(const char* label, int* currIndex, std::vector<Material*>& values)
	{
		if (values.empty()) { return false; }
		return Combo(label, currIndex, vector_getter1,
			static_cast<void*>(&values), values.size());
	}

	bool ListBoxActors(const char* label, int* currIndex, std::vector<Actor*>& values)
	{
		if (values.empty()) { return false; }
		return ListBox(label, currIndex, vector_getterActors,
			static_cast<void*>(&values), values.size());
	}

	bool ListBox(const char* label, int* currIndex, std::vector<Material*>& values)
	{
		if (values.empty()) { return false; }
		return ListBox(label, currIndex, vector_getter1,
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

	SaveScene = new SceneSaver();

	if (gUseTerrain)
	{
		GenerateFoliage();
		m_Scene->DrawTerrain = true;
	}

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
		ImGui::SliderFloat("ssao Blur sigma", &m_Scene->ssaoBlurSigma, 0.0f, 50.0f);
		ImGui::SliderFloat("Sky dome rotation speed: ", &m_Scene->skyDomeRotationSpeed, 0.0f, 1.0f, "%.7f");
		ImGui::Separator();

		//Just to keep index bound in actorlist size
		if (ObjectSelectedIndex >= m_Scene->m_Actors.size())
		{
			ObjectSelectedIndex = 0;
		}

		if (m_Scene->m_Actors.size() > 0 && m_Scene->m_Actors[ObjectSelectedIndex] != nullptr)
		{
			ImGui::ListBoxActors("Loaded Actors", &ObjectSelectedIndex, m_Scene->m_Actors);
			ImGui::Text(m_Scene->m_Actors[ObjectSelectedIndex]->actorFile.c_str());
			ImGui::SliderFloat("Actor roughness", &m_Scene->m_Actors[ObjectSelectedIndex]->actorMatrix.roughness, -1.0f, 1.0f);
			ImGui::SliderFloat("Metallic", &m_Scene->m_Actors[ObjectSelectedIndex]->actorMatrix.metallic, 0.0f, 1.0f);
			ImGui::InputFloat("Texture padding x", &m_Scene->m_Actors[ObjectSelectedIndex]->actorMatrix.texOffsetx, -20.0f, 20.0f);
			ImGui::InputFloat("Texture padding y", &m_Scene->m_Actors[ObjectSelectedIndex]->actorMatrix.texOffsety, -20.0f, 20.0f);
		}

		if (ImGui::Button("Save", ImVec2(100, 20)))
		{
			if (!SaveScene->SaveScene(m_Scene, "null"))
			{
				MessageBox(mainWindow, L"Cannot save file", L"ERROR", MB_OK);
			}

		}

		ImGui::InputFloat("Shadow bias: ", &m_Scene->dirLight.lightProperties.shadowBias, 0.001f, 0.01f);

		ImGui::Separator();

		ImGui::SliderFloat("Subsurface", &m_Scene->subspectintani.x, 0.0f, 1.0f);
		ImGui::SliderFloat("specular", &m_Scene->subspectintani.y, 0.0f, 1.0f);
		ImGui::SliderFloat("specularTint", &m_Scene->subspectintani.z, 0.0f, 1.0f);
		ImGui::SliderFloat("anisotropic", &m_Scene->subspectintani.w, 0.0f, 1.0f);
		ImGui::SliderFloat("sheen", &m_Scene->sheentintcleargloss.x, 0.0f, 1.0f);
		ImGui::SliderFloat("sheenTint", &m_Scene->sheentintcleargloss.y, 0.0f, 1.0f);
		ImGui::SliderFloat("clearcoat", &m_Scene->sheentintcleargloss.z, 0.0f, 1.0f);
		ImGui::SliderFloat("clearcoatGloss", &m_Scene->sheentintcleargloss.w, 0.0f, 1.0f);

		if (ImGui::CollapsingHeader("Sprite menu"))
		{

			if (m_Scene->m_Actors.size() > 0)
			{
				
				ImGui::Checkbox("Use sprite sheet:", &m_Scene->m_Actors[ObjectSelectedIndex]->UseAnimatedSpriteSheet);
			
				ImGui::LabelText("", "Use custom material");
				if (ImGui::Button("Set", ImVec2(100, 30)))
				{
					m_Scene->m_Actors[ObjectSelectedIndex]->UnSetMeshUseCustomMaterial();
				}
				if (ImGui::Button("Unset", ImVec2(100, 30)))
				{
					m_Scene->m_Actors[ObjectSelectedIndex]->SetMeshUseCustomMaterial();
				}
			}

		}
	
		ImGui::InputFloat("max obj dist: ", &m_Scene->maxObjectDrawDistance, 0.0f, 20000.0f);
		ImGui::InputFloat("min obj dist: ", &m_Scene->minObjectDrawDistance, -20000.0f, 0.0f);
		if(m_Scene->m_Actors.size() > 0) ImGui::InputInt("Sprite animation interval: ", (int*)&m_Scene->m_Actors[ObjectSelectedIndex]->SpriteAnimationInterval, 1, 400);


		if (ImGui::Button("Add spritesheet", ImVec2(120, 40)))
		{
			AddSpriteSheetToObject();
		}

		ImGui::End();


		ImGui::Begin("File");
		ImGui::SetWindowSize(ImVec2(300, 400));
		ImGui::SetWindowPos(ImVec2(1600, 400));

		if (ImGui::Button("Load Model"))
		{

			std::string Path = "";
			OpenFileDialog* ofd = new OpenFileDialog();
			ofd->Owner = mainWindow;
			if (ofd->ShowDialog() && ofd->FileName != nullptr)
			{
				std::wstring p(ofd->FileName);
				Path = ws2s(p);

				Actor* a = new Actor(Path.c_str(), m_D3DDevice);
				m_Scene->AddSceneActor(a, m_D3DDevice);
				LoadedActorList.push_back(a);
			}

			ofd = nullptr;
			delete ofd;
		}

		if (ImGui::Button("Delete Model"))
		{
			m_Scene->m_Actors[ObjectSelectedIndex]->Release();
			m_Scene->m_Actors.erase(m_Scene->m_Actors.begin() + ObjectSelectedIndex);
		}

		std::string sceneName = gSceneName;
		std::string saveScenePath = "D:\\Graphics-programming\\D3D_Template\\D3D_Template\\Scenes\\" + sceneName;
		ImGui::InputText("Scene name: ", sceneNameBuffer, strlen(sceneNameBuffer));


		if (ImGui::Button("Save Scene"))
		{
			SaveFile.open(saveScenePath);

			if (SaveFile.is_open())
			{

				for (size_t i = 0; i < m_Scene->m_Actors.size(); i++)
				{
					std::string aData = m_Scene->m_Actors[i]->ObjectTransmissionString() + "\n";
					SaveFile << aData;
				}

				SaveFile.close();
			}
			else
			{

				MessageBox(NULL, L"ERROR CANNOT OPEN FILE", L"ERROR", MB_OK);

				SaveFile.close();
			}
		}

		if (ImGui::Button("Load Scene"))
		{
			if (!LoadSceneFromFile())
			{
				MessageBox(NULL, L"ERROR CANNOT OPEN FILE", L"ERROR", MB_OK);
			}
		}

		CustomizeActorSpriteSheet();

		ImGui::End();

		SetImgui();

	

		//Debugging only
		//ImGui::SliderInt("Screen offset", &m_Scene->viewPortOffSet, -1000, 1000);

	}



	TimeVar shadowTime = timeNow();
	m_Scene->ShadowPass(m_D3DDevice);
	double sTime = duration(timeNow() - shadowTime) * pow(10, -6);

	TimeVar gbTime = timeNow();
	m_Scene->ssaoBiasAndRadius = XMFLOAT2(this->ssaoBias, this->ssaoRadius);
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
		this->SaveSceneS();
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

	std::string opt1 = "Press p to take a screen shot";

	std::string memoryUsage = "";

	DWORDLONG memUsage = GetTotalMemory() / 1024 / 1024;
	std::ostringstream memStream;
	memStream << memUsage;
	memoryUsage = memStream.str() + " mb";

	textContext.Print(5, 15, vpofs.c_str());
	textContext.Print(5, 35, GPUinfo.c_str());
	textContext.Print(5, 75, memoryUsage.c_str());





	if (LastObjectSelectedIndex != ObjectSelectedIndex)
	{
		this->PrepareTW();
		LastObjectSelectedIndex = ObjectSelectedIndex;
	}

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
			ImGui::Image(materiallist[materialIndex]->GetTexture("albedo")->GetTexture(), ImVec2(50, 50));
		}

		if (ImGui::Button("Create metallic/spec"))
		{
			LoadMaterialTexture("specular");
		}
		if (materiallist[materialIndex]->GetTexture("specular") != nullptr)
		{
			ImGui::Text(materiallist[materialIndex]->GetTexture("specular")->textureName.c_str());
			ImGui::Image(materiallist[materialIndex]->GetTexture("specular")->GetTexture(), ImVec2(50, 50));
		}

		if (ImGui::Button("Create normal"))
		{
			LoadMaterialTexture("normal");
		}
		if (materiallist[materialIndex]->GetTexture("normal") != nullptr)
		{
			ImGui::Text(materiallist[materialIndex]->GetTexture("normal")->textureName.c_str());
			ImGui::Image(materiallist[materialIndex]->GetTexture("normal")->GetTexture(), ImVec2(50, 50));
		}

		if (ImGui::Button("Create roughness"))
		{
			LoadMaterialTexture("roughness");
		}
		if (materiallist[materialIndex]->GetTexture("roughness") != nullptr)
		{
			ImGui::Text(materiallist[materialIndex]->GetTexture("roughness")->textureName.c_str());
			ImGui::Image(materiallist[materialIndex]->GetTexture("roughness")->GetTexture(), ImVec2(50, 50));
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


void GraphicsEngine::GenerateFoliage()
{
	//Template models
	Actor *a1 = new Actor("Models\\betula\\Models\\BL02m.obj", m_D3DDevice);
	a1->SetModelSize(XMVectorSet(0.1, 0.1, 0.1, 1.0));
	a1->SetModelPosition(XMVectorSet(1, 1, 1, 1.0f));
	a1->HasAlpha = true;
	a1->actorMatrix.roughness = 0.80f;
	a1->UseTextures = true;
	m_Scene->AddSceneActor(a1, m_D3DDevice);


	Actor *a2 = new Actor("Models\\betula\\Models\\BL02y.obj", m_D3DDevice);
	a2->SetModelSize(XMVectorSet(0.1, 0.1, 0.1, 1.0));
	a2->SetModelPosition(XMVectorSet(1, 1, 1, 1.0f));
	a2->HasAlpha = true;
	a2->actorMatrix.roughness = 0.80f;
	a2->UseTextures = true;
	m_Scene->AddSceneActor(a2, m_D3DDevice);



	m_Scene->terrain->terrainMatrix.terrainScale = XMFLOAT3(10.0f, 10.0f, 10.0f);
	m_Scene->terrain->terrainMatrix.terrainPosition = XMFLOAT3(0.0f, 0.0f, 0.0f);

	for (size_t i = 0; i < 50; i++)
	{
		for (size_t j = 0; j < 50; j++)
		{
			
			XMFLOAT3 locf;
			std::random_shuffle(m_Scene->terrain->terrainmodels.begin(), m_Scene->terrain->terrainmodels.end());

			XMStoreFloat3(&locf, m_Scene->terrain->terrainmodels[i]);
		

			locf.x = locf.x * m_Scene->terrain->terrainMatrix.terrainScale.x - m_Scene->terrain->terrainMatrix.terrainPosition.x;
			locf.y = locf.y * m_Scene->terrain->terrainMatrix.terrainScale.y - m_Scene->terrain->terrainMatrix.terrainPosition.y;
			locf.z = locf.z * m_Scene->terrain->terrainMatrix.terrainScale.z - m_Scene->terrain->terrainMatrix.terrainPosition.z;

			Actor *a = new Actor(*a1);
			a->SetModelSize(XMVectorSet(0.5, 0.5, 0.5, 1.0));
			a->SetModelPosition(XMVectorSet(locf.x, locf.y, locf.z , 1.0f));
			a->SetModelRotation(XMVectorSet(4.5f, 1.0f, 1.0f, 1.0f));
			a->HasAlpha = true;
			a->HasShadow = false;
			a->actorMatrix.roughness = 0.90f;
			a->UseTextures = true;

			Actor *aa = new Actor(*a2);
			aa->SetModelSize(XMVectorSet(1.0, 1.0, 1.0, 1.0));
			aa->SetModelPosition(XMVectorSet(locf.x + 25, locf.y, locf.z + i, 1.0f));
			aa->SetModelRotation(XMVectorSet(4.5f, 1.0f, 1.0f, 1.0f));
			aa->HasAlpha = true;
			aa->HasShadow = false;
			aa->actorMatrix.roughness = 0.90f;
			aa->UseTextures = true;

			m_Scene->AddSceneActor(a, m_D3DDevice);
			m_Scene->AddSceneActor(aa, m_D3DDevice);
		}
	}


}

SIZE_T GraphicsEngine::GetTotalMemory()
{
	PROCESS_MEMORY_COUNTERS pmc;
	GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc));
	SIZE_T virtualMemUsedByMe = pmc.WorkingSetSize;

	return virtualMemUsedByMe;
}

void GraphicsEngine::CustomizeActorSpriteSheet()
{

		ImGui::Begin("SpriteSheet");
		ImGui::SetWindowSize(ImVec2(300, 400));

		if (m_Scene->m_Actors.size() > 0 && ObjectSelectedIndex <= m_Scene->m_Actors.size()-1)
		{
			if (m_Scene->m_Actors[ObjectSelectedIndex]->UseAnimatedSpriteSheet)
			{
				ImGui::ListBox("Sprites", &SpriteSheetIndex, m_Scene->m_Actors[ObjectSelectedIndex]->spriteSheet);
				std::string spriteCount(std::to_string(m_Scene->m_Actors[ObjectSelectedIndex]->spriteSheet.size()));
				ImGui::LabelText(spriteCount.c_str(), "Sprite Count: ");

				ImGui::Text(m_Scene->m_Actors[ObjectSelectedIndex]->spriteSheet[SpriteSheetIndex]->GetTexture("albedo")->textureName.c_str());
				ImGui::Image(m_Scene->m_Actors[ObjectSelectedIndex]->spriteSheet[SpriteSheetIndex]->GetTexture("albedo")->GetTexture(), ImVec2(50, 50));

				if (ImGui::Button("Delete", ImVec2(100, 30)))
				{
					m_Scene->m_Actors[ObjectSelectedIndex]->spriteSheet[SpriteSheetIndex]->ReleaseMaterial();
					m_Scene->m_Actors[ObjectSelectedIndex]->spriteSheet.erase(m_Scene->m_Actors[ObjectSelectedIndex]->spriteSheet.begin() + SpriteSheetIndex);
					SpriteSheetIndex = 0;
				}

				//swap sprite places in the list
				if (ImGui::Button("--", ImVec2(40, 30)))
				{
					if (SpriteSheetIndex + 1 <= m_Scene->m_Actors[ObjectSelectedIndex]->spriteSheet.size())
					{
						std::iter_swap(m_Scene->m_Actors[ObjectSelectedIndex]->spriteSheet.begin() + SpriteSheetIndex,
							m_Scene->m_Actors[ObjectSelectedIndex]->spriteSheet.begin() + SpriteSheetIndex + 1);
						SpriteSheetIndex++;
					}
				}
				if (ImGui::Button("++", ImVec2(40, 30)))
				{
					if (SpriteSheetIndex - 1 >= 0)
					{
						std::iter_swap(m_Scene->m_Actors[ObjectSelectedIndex]->spriteSheet.begin() + SpriteSheetIndex,
							m_Scene->m_Actors[ObjectSelectedIndex]->spriteSheet.begin() + SpriteSheetIndex - 1);
						SpriteSheetIndex--;
					}

				}
			}

		}
		ImGui::End();


}

void GraphicsEngine::AddSpriteSheetToObject()
{
	std::wstring folder = BrowseFolder("D:\\Graphics-programming\\D3D_Template\\D3D_Template\\Textures\\");

	std::vector<std::string> getSpritePaths = GetFileNamesFromFolder(folder);

	if (folder.size() > 0)
	{
		m_Scene->m_Actors[ObjectSelectedIndex]->spriteSheet.clear();
		for (size_t i = 0; i < getSpritePaths.size(); i++)
		{
			m_Scene->m_Actors[ObjectSelectedIndex]->AppendSpriteSheet(getSpritePaths[i]);
			m_Scene->m_Actors[ObjectSelectedIndex]->UseAnimatedSpriteSheet = true;
			m_Scene->m_Actors[ObjectSelectedIndex]->SetMeshUseCustomMaterial();
		}
	}



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

void GraphicsEngine::SaveSceneS()
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

bool GraphicsEngine::LoadSceneFromFile()
{
	std::vector<std::string> lines;

	std::string Path = "";
	OpenFileDialog* ofd = new OpenFileDialog();
	ofd->Owner = mainWindow;
	if (ofd->ShowDialog() && ofd->FileName != nullptr)
	{
		std::wstring p(ofd->FileName);
		Path = ws2s(p);
		gSceneName = ws2s(p);
		sceneNameBuffer = &gSceneName[0];
		gSceneName = SplitPath(gSceneName, { '\\' }).back();
		std::ifstream inFile(Path);
		std::string line;
		while (std::getline(inFile, line))
		{
			lines.push_back(line);
		}

		m_Scene->m_Actors.clear();

		for (size_t i = 0; i < lines.size(); i++)
		{
			std::vector<std::string> astring = split(lines[i], '+');
			std::string::size_type sz;
			Actor *a = new Actor(astring[0].c_str(), m_D3DDevice);
			a->actorMatrix.position.x = std::stof(astring[1], &sz);
			a->actorMatrix.position.y = std::stof(astring[2], &sz);
			a->actorMatrix.position.z = std::stof(astring[3], &sz);

			a->actorMatrix.rotation.x = std::stof(astring[5], &sz);
			a->actorMatrix.rotation.y = std::stof(astring[6], &sz);
			a->actorMatrix.rotation.z = std::stof(astring[7], &sz);

			a->actorMatrix.size.x = std::stof(astring[9], &sz);
			a->actorMatrix.size.y = std::stof(astring[10], &sz);
			a->actorMatrix.size.z = std::stof(astring[11], &sz);

			m_Scene->AddSceneActor(a, m_D3DDevice);
		}

		ofd = 0;
		delete ofd;
		return true;
	}
	else
	{
		ofd = 0;
		delete ofd;
		return false;
	}
}

void GraphicsEngine::PrepareScene()
{

	a[0] = new Actor("Models\\Crytek\\Sponza\\sponza.obj", m_D3DDevice);
	a[0]->SetModelSize(XMVectorSet(0.1, 0.1, 0.1, 1.0));
	a[0]->SetModelPosition(XMVectorSet(0, 0, 0, 1.0f));
	a[0]->HasAlpha = true;
	a[0]->actorMatrix.roughness = 0.80f;
	a[0]->UseTextures = true;
	m_Scene->AddSceneActor(a[0],m_D3DDevice);


	a[1] = new Actor("Models\\plane.obj", m_D3DDevice);
	a[1]->SetModelSize(XMVectorSet(1, 1, 1, 1.0));
	a[1]->SetModelPosition(XMVectorSet(0, -10, 0, 1.0f));
	a[1]->HasAlpha = false;
	a[1]->actorMatrix.texOffsetx = 5.0f;
	a[1]->actorMatrix.texOffsety = 5.0f;
	a[1]->actorMatrix.roughness = 1.0f;
	a[1]->UseTextures = true;
	m_Scene->AddSceneActor(a[1], m_D3DDevice);


	m_Scene->BlurSigma = 0.0f;




	//a[1] = new Actor("Models\\plane.obj", m_D3DDevice);
	//a[1]->SetModelSize(XMVectorSet(0.1, 0.1, 0.1, 1.0));
	//a[1]->SetModelPosition(XMVectorSet(0, 5, 10, 1.0f));
	//a[1]->SetModelRotation(XMVectorSet(1.59f, 0.0f, 3.18f, 1.0f));
	//a[1]->HasAlpha = true;
	//a[1]->actorMatrix.roughness = 0.80f;
	//a[1]->UseTextures = true;
	//a[1]->SetMeshUseCustomMaterial();
	//a[1]->UseAnimatedSpriteSheet = true;
	//
	//std::vector<std::string> spritesPaths = GetFileNamesFromFolder(L"Textures\\SpriteSheet\\Braid");
	//
	//for (size_t i = 0; i < spritesPaths.size(); i++)
	//{
	//	a[1]->AppendSpriteSheet(spritesPaths[i]);
	//}
	//
	//
	//m_Scene->AddSceneActor(a[1], m_D3DDevice);
	//LoadedActorList.push_back(a[1]);
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
	m_GUI->AddVariableBoolean("Shadow map: ", m_Scene->m_Actors[ObjectSelectedIndex]->HasShadow);
	
	m_GUI->AddVariableXMfloat("Skydome center color: ", m_Scene->GetSkyDome()->m_centerColor);
	m_GUI->AddVariableXMfloat("Skydome apex color: ", m_Scene->GetSkyDome()->m_apexColor);
	m_GUI->AddVariableXMfloat("Camera RO Pos: ", m_Scene->GetCamera()->CameraPositionXF);
	m_GUI->AddVariableFloat("Sun Pow: ", m_Scene->dirLight.lightProperties.sunPower);
	m_GUI->AddVariableFloat("Sun scale1: ", m_Scene->dirLight.lightProperties.scale1);
	m_GUI->AddVariableFloat("Sun scale2: ", m_Scene->dirLight.lightProperties.scale2);
	m_GUI->AddVariableFloat("GlobalAmbient: ", m_Scene->dirLight.lightProperties.globalAmbient);
	m_GUI->AddVariableXMfloat("Terrain Pos: ", m_Scene->terrain->terrainMatrix.terrainPosition);
	m_GUI->AddVariableXMfloat("Terrain scale: ", m_Scene->terrain->terrainMatrix.terrainScale);
	m_GUI->AddVariableFloat("SSAO Bias: ", ssaoBias);
	m_GUI->AddVariableFloat("SSAO Radius: ", ssaoRadius);

	m_GUI->AddDirectionalLight(m_Scene->dirLight);
	
}

void GraphicsEngine::GetHwndSize(HWND hWnd, int & width, int & height)
{
	RECT rect;
	GetWindowRect(hWnd, &rect);
	
	width = rect.right - rect.left;
	height = rect.bottom - rect.top;
	
}
