#include "stdafx.h"
#include "SceneClass.h"
#include <fstream>



SceneClass::SceneClass()
{
}


SceneClass::~SceneClass()
{
}

void SceneClass::InitializeScene(CDeviceClass * DevClass, int scenewidth, int sceneheight, HWND hWnd)
{
	this->_sceneWidth = scenewidth;
	this->_sceneHeight = sceneheight;
	this->mainWindow = hWnd;


	m_DeferredBuffer = new DeferredBuffersClass();
	
	renderScale = 2.0f;

	m_DeferredBuffer->Initialize(DevClass->GetDevice(),scenewidth*renderScale, sceneheight * renderScale, 10.0, 0.1, DXGI_FORMAT_R16G16B16A16_FLOAT);


	m_DeferredShader = new DeferredShader();
	
	m_DeferredShader->Initialize(DevClass, L"deferredvs.hlsl", L"deferredps.hlsl");

	
	m_LightShader = new LightShader();
	
	m_LightShader->Initialize(DevClass, L"lightvs.hlsl", L"lightps.hlsl");


	m_Window = new COrthoWindow();

	m_Window->Initialize(DevClass->GetDevice(), scenewidth, sceneheight);


	postProcessTexture = new CRenderToTexture();

	postProcessTexture->Initialize(DevClass, scenewidth, sceneheight, 10.0, 0.1f, 1);


	postProcessor = new PostProcessor();
	postProcessor->useSmaa = true;
	postProcessor->InitializePostProcessor(DevClass, scenewidth, sceneheight);

	std::vector<std::string> images;
	//Environment map textures
	{
		
		std::string im1 = "Textures\\TropicalSunnyDay\\TropicalSunnyDayBack2048.png";
		std::string im2 = "Textures\\TropicalSunnyDay\\TropicalSunnyDayDown2048.png";
		std::string im3 = "Textures\\TropicalSunnyDay\\TropicalSunnyDayFront2048.png";
		std::string im4 = "Textures\\TropicalSunnyDay\\TropicalSunnyDayLeft2048.png";
		std::string im5 = "Textures\\TropicalSunnyDay\\TropicalSunnyDayRight2048.png";
		std::string im6 = "Textures\\TropicalSunnyDay\\TropicalSunnyDayUp2048.png";


		//std::string im1 = "Textures\\yoko\\negz.jpg";
		//std::string im2 = "Textures\\yoko\\negy.jpg";
		//std::string im3 = "Textures\\yoko\\posz.jpg";
		//std::string im4 = "Textures\\yoko\\posx.jpg";
		//std::string im5 = "Textures\\yoko\\negx.jpg";
		//std::string im6 = "Textures\\yoko\\posy.jpg";

		//Need to write order down
		images.push_back(im4);
		images.push_back(im5);
		images.push_back(im2);
		images.push_back(im6);
		images.push_back(im3);
		images.push_back(im1);

	}

	environmentMap = new CTextureTA();

	//environmentMap->LoadFreeImage(DevClass->GetDevice(), DevClass->GetDevCon(), "Textures\\env1.jpg");
	environmentMap->LoadCubeMap(DevClass->GetDevice(), DevClass->GetDevCon(), images);

	irradianceMap = new CTextureTA();

	irradianceMap->LoadFreeImage(DevClass->GetDevice(), DevClass->GetDevCon(), "Textures\\Irradiance.dds");

	areaTexture = new CTextureTA();

	areaTexture->LoadFreeImage(DevClass->GetDevice(), DevClass->GetDevCon(), "Textures\\AreaTexDX10.dds");

	searchTexture = new CTextureTA();

	searchTexture->LoadFreeImage(DevClass->GetDevice(), DevClass->GetDevCon(), "Textures\\SearchTex.dds");

	ssaoNoiseTexture = new CTextureTA();

	if (!ssaoNoiseTexture->LoadFreeImage(DevClass->GetDevice(), DevClass->GetDevCon(), "Textures\\randomtexture.jpg"))
	{
		MessageBox(hWnd, L"Cannot init ssao noise texture", L"ERROR!", MB_OK);
	}

	//Terrain init
	terrain = new CTerrain();
	if (!terrain->Initialize(DevClass->GetDevice(), "Models\\Terrain\\setup.txt"))
	{
		MessageBox(hWnd, L"Could not initialize terrain.", L"Error", MB_OK);
	}
	terrain->SetTerrainTextures(DevClass, "Models\\Terrain\\textures\\dirt01d.tga", "Models\\Terrain\\textures\\dirt01n.tga");

	terrainShader = new CTerrainShader();
	if (!terrainShader->Initialize(DevClass->GetDevice(), hWnd))
	{
		MessageBox(hWnd, L"Could not initialize terrain shader.", L"Error", MB_OK);
	}


	textureShader = new CTextureRenderShader();

	textureShader->Initialize(DevClass, hWnd);
	textureShader->Exposure = 22.0;

	//Lights and shadow map rt
	dirLight.lightProperties.Position = XMFLOAT4(300.0f, 2200.0f, -400.0f, 1.0f);
	dirLight.CalcLightViewMatrix();
	dirLight.CalcProjectionMatrix();
	dirLight.lightProperties.Color = XMFLOAT4(0.5, 0.5, 0.5, 1.0f);
	dirLight.lightProjectionF = XMFLOAT4(5000.0f, 700.0f, 700.0f, 1.0f);
	dirLight.lightProperties.size = 29; //very tricky to get right.. Gotta fix shader sometime

	shadowMap = new ShadowMapRenderTarget();
	
	if (!shadowMap->Initialize(DevClass, 2048*2, 2048*2, 10000.0f, 0.1f))
	{
		MessageBox(NULL, L"Shadow map creation error", L"ERROR", MB_OK);
	}

	m_ShadowShader = new CDepthShader();

	m_ShadowShader->Initialize(DevClass->GetDevice(), NULL);

	m_SkyDome = new CSkydome();

	m_SkyDome->Initialize(DevClass->GetDevice());
	
	m_SkyDome->LoadTexture(DevClass, "Textures\\sunsetpier.jpg");
	m_SkyDome->textureSDcube = environmentMap;

	m_SkyDomeShader = new CSkyDomeShader();

	m_SkyDomeShader->Initialize(DevClass->GetDevice(), hWnd);


	m_Camera = new FreeCamera(hWnd);
	m_Camera->SetCameraPosition(XMVectorSet(0, 10, -15, 1.0));


	for (size_t i = 0; i < POINT_LIGHT_COUNT; i++)
	{
		PointLight p = PointLight();
		p.lightProperties.Position = XMFLOAT4(50 * std::sin(i), 10, std::sin(i) * 30, 1.0f);
		if (i > 2)
		{
			p.lightProperties.Color = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
		}
		pointLights.push_back(p);
	}



	m_Camera->UpdateCamera();
}

//Render shadow map to RT
void SceneClass::ShadowPass(CDeviceClass * DevClass)
{
	DevClass->TurnCullingOn();
	DevClass->TurnZBufferOn();
	XMMATRIX worlMatrix, viewMatrix, projectionMatrix;

	shadowMap->SetRenderTarget(DevClass->GetDevCon());
	shadowMap->ClearRenderTarget(DevClass->GetDevCon(),0.0f, 0.0f, 0.0f, 1.0f);

	if (ShadowUseFrontCulling)
	{
		DevClass->TurnCullingFront();
	}
	
	dirLight.CalcLightViewMatrix();
	dirLight.CalcProjectionMatrix();

	for (size_t i = 0; i < this->m_Actors.size(); i++)
	{
		if (m_Actors[i]->HasShadow)
		{
			worlMatrix = XMMatrixIdentity();
			viewMatrix = dirLight.GetLightViewMatrix();
			projectionMatrix = dirLight.GetLightProjectionMatrix();

			m_ShadowShader->Render(DevClass->GetDevCon(), 0, m_Actors[i]->modelMatrix, viewMatrix, projectionMatrix);
			m_Actors[i]->RenderShadowMap(DevClass, m_ShadowShader);
		}

	}

	DevClass->ResetViewPort();
	DevClass->TurnCullingOn();
}

void SceneClass::GeometryPass(CDeviceClass * DevClass)
{
	
	m_Camera->UpdateCamera();

	DevClass->TurnCullingOn();
	DevClass->TurnZBufferOn();

	XMMATRIX projection, view;

	projection = DevClass->GetProjectionMatrix();

	if (!UseOrthoCamera)	view = m_Camera->GetCameraView();
	else view = m_Camera->GetBaseViewMatrix();


	TimeVar time1 = timeNow();
	m_DeferredBuffer->SetRenderTargets(DevClass->GetDevCon());
	m_DeferredBuffer->ClearRenderTargets(DevClass->GetDevCon(), 0.0, 0.0, 0.0, 1.0);

	GeoBenchMarks[0] = duration(timeNow() - time1);

	
	//Skydome
	{
		XMMATRIX worldSphere = XMMatrixIdentity();

		worldSphere = XMMatrixScalingFromVector(XMVectorSet(4000.0f, 4000.0f, 4000.0f, 1.0f));

		projection = DevClass->GetProjectionMatrix();
		view = m_Camera->GetCameraView();
		DevClass->TurnCullingFront();
		DevClass->TurnZBufferOff();

		m_SkyDome->Render(DevClass->GetDevCon());
		//m_SkyDomeShader->SetSkyDomeTexture(DevClass->GetDevCon(), m_SkyDome->textureSD->GetTexture(), 0);
		m_SkyDomeShader->SetSkyDomeTexture(DevClass->GetDevCon(), environmentMap->cubeGetTexture(), 1);
		if (!m_SkyDomeShader->Update(DevClass->GetDevCon(), m_SkyDome->GetIndexCount(),
			worldSphere, view, projection, m_SkyDome->GetApexColor(), m_SkyDome->GetCenterColor(), dirLight, m_Camera))
		{
			MessageBox(NULL, L"Error skydome rendering", L"ERROR", MB_OK);
		}
	}

	//Terrain
	if (DrawTerrain)
	{
		RenderTerrainGPass(DevClass);
	}

	projection = DevClass->GetProjectionMatrix();
	if (!UseOrthoCamera)	view = m_Camera->GetCameraView();
	else view = m_Camera->GetBaseViewMatrix();
	m_LightShader->tempViewMatrix = view;
	postProcessor->ssaoBiasAndRadius = this->ssaoBiasAndRadius;


	DevClass->TurnCullingOn();
	DevClass->TurnZBufferOn();

	TimeVar time2 = timeNow();
	for (size_t i = 0; i < m_Actors.size(); i++)
	{
		XMVECTOR actorPosXMV = XMLoadFloat4(&m_Actors[i]->actorMatrix.position);
		XMFLOAT4 distanceToCamera; XMStoreFloat4(&distanceToCamera, XMVector4Length(XMVectorSubtract(actorPosXMV, m_Camera->GetCameraPosition())));
		//XMFLOAT4 isBehindCamera; XMStoreFloat4(&isBehindCamera, XMVector4Dot(XMVector4Normalize(m_Camera->lookAt), XMVector4Normalize(actorPosXMV)));

		if ((distanceToCamera.x >= minObjectDrawDistance && distanceToCamera.x <= maxObjectDrawDistance))
		{
			TimeVar time3 = timeNow();
			m_DeferredShader->UpdateShader(DevClass, m_Actors[i]->modelMatrix, view, projection, m_Actors[i]->HasAlpha, XMFLOAT2(m_Actors[i]->actorMatrix.texOffsetx, m_Actors[i]->actorMatrix.texOffsety));
			double geo2New = duration(timeNow() - time3);
			GeoBenchMarks[2] = geo2New;
		
		

			m_Actors[i]->RenderModel(DevClass, m_DeferredShader);
		}
		
		projection = DevClass->GetProjectionMatrix();
		if (!UseOrthoCamera)	view = m_Camera->GetCameraView();
		else view = m_Camera->GetBaseViewMatrix();
	}
	double geo3New = duration(timeNow() - time2);
	GeoBenchMarks[1] = geo3New;



}

void SceneClass::LightPass(CDeviceClass * DevClass)
{
	
	XMMATRIX worldMatrix, baseViewMatrix, orthoMatrix;
	
	HandleSceneInput();

	DevClass->TurnZBufferOff();
	m_Window->UpdateWindow(DevClass->GetDevCon(), viewPortOffSet, 0);

	//ID3D11ShaderResourceView* shadowRes = postProcessor->BlurShadows(DevClass, m_Window, shadowMap->GetShaderResourceView());

	DevClass->ResetViewPort();
	if (ApplyPostProcess)
	{
		postProcessTexture->ClearRenderTarget(DevClass->GetDevCon(), 0.0, 0.0, 0.0, 1.0);
		postProcessTexture->SetRenderTarget(DevClass->GetDevCon());
	}
	else
	{
		DevClass->SetBackBufferRenderTarget();
	}

	worldMatrix = XMMatrixIdentity();
	orthoMatrix = DevClass->GetOrthoMatrix();
	baseViewMatrix = XMMatrixLookAtLH(XMVectorSet(0.0f, 0.0f, -1.0f, 1.0f), XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f), XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f));
	


	m_LightShader->UpdateDisneyBuffer(DevClass, subspectintani, sheentintcleargloss);
	m_LightShader->UpdateShadowMap(DevClass, shadowMap->GetShaderResourceView());
	m_LightShader->UpdateTextureByIndex(DevClass, environmentMap->cubeGetTexture(), 8);
	m_LightShader->UpdateTextureByIndex(DevClass, irradianceMap->GetTexture(),9);
	//m_LightShader->UpdateTextureByIndex(DevClass, ssaoRes, 10);

	//Settings to show all textures passed 
	if (Setting == 0)
	{
		m_LightShader->UpdateCameraPosition(DevClass, m_Camera->GetCameraPosition());
		m_LightShader->UpdateShaderParameters(DevClass, worldMatrix, baseViewMatrix, orthoMatrix, m_DeferredBuffer->GetShaderResourceView(0), m_DeferredBuffer->GetShaderResourceView(1), m_DeferredBuffer->GetShaderResourceView(2), m_DeferredBuffer->GetShaderResourceView(3),m_DeferredBuffer->GetShaderResourceView(4), m_DeferredBuffer->GetShaderResourceView(5), m_DeferredBuffer->GetShaderResourceView(6), dirLight, pointLights);
	}
	if (Setting == 1)
	{
		m_LightShader->UpdateCameraPosition(DevClass, m_Camera->GetCameraPosition());
		m_LightShader->UpdateShaderParameters(DevClass, worldMatrix, baseViewMatrix, orthoMatrix, m_DeferredBuffer->GetShaderResourceView(1), m_DeferredBuffer->GetShaderResourceView(1), m_DeferredBuffer->GetShaderResourceView(2), m_DeferredBuffer->GetShaderResourceView(3), m_DeferredBuffer->GetShaderResourceView(4), m_DeferredBuffer->GetShaderResourceView(5), m_DeferredBuffer->GetShaderResourceView(6), dirLight, pointLights);
	}

	if (Setting == 2)
	{
		m_LightShader->UpdateCameraPosition(DevClass, m_Camera->GetCameraPosition());
		m_LightShader->UpdateShaderParameters(DevClass, worldMatrix, baseViewMatrix, orthoMatrix, m_DeferredBuffer->GetShaderResourceView(2), m_DeferredBuffer->GetShaderResourceView(1), m_DeferredBuffer->GetShaderResourceView(2), m_DeferredBuffer->GetShaderResourceView(3), m_DeferredBuffer->GetShaderResourceView(4), m_DeferredBuffer->GetShaderResourceView(5), m_DeferredBuffer->GetShaderResourceView(6), dirLight, pointLights);
	}

	if (Setting == 3)
	{
		m_LightShader->UpdateCameraPosition(DevClass, m_Camera->GetCameraPosition());
		m_LightShader->UpdateShaderParameters(DevClass, worldMatrix, baseViewMatrix, orthoMatrix, m_DeferredBuffer->GetShaderResourceView(5), m_DeferredBuffer->GetShaderResourceView(1), m_DeferredBuffer->GetShaderResourceView(2), m_DeferredBuffer->GetShaderResourceView(3), m_DeferredBuffer->GetShaderResourceView(4), m_DeferredBuffer->GetShaderResourceView(5), m_DeferredBuffer->GetShaderResourceView(6), dirLight, pointLights);
	}

	if (Setting == 4)
	{
		m_LightShader->UpdateCameraPosition(DevClass, m_Camera->GetCameraPosition());
		m_LightShader->UpdateShaderParameters(DevClass, worldMatrix, baseViewMatrix, orthoMatrix, m_DeferredBuffer->GetShaderResourceView(6), m_DeferredBuffer->GetShaderResourceView(1), m_DeferredBuffer->GetShaderResourceView(2), m_DeferredBuffer->GetShaderResourceView(3), m_DeferredBuffer->GetShaderResourceView(4), m_DeferredBuffer->GetShaderResourceView(5), m_DeferredBuffer->GetShaderResourceView(6), dirLight, pointLights);
	}
	

	m_Window->Render(DevClass->GetDevCon());
	m_LightShader->Update(DevClass, m_Window->m_indexCount);


	DevClass->ResetViewPort();

	// Post process the outputs of light shader
	if (ApplyPostProcess)
	{
		
		postProcessor->SetPostProcessInputs(postProcessTexture->GetShaderResourceView(0), postProcessTexture->GetShaderResourceView(1), m_Window, BlurSigma);

		//Check texture
		if (Setting == 5)
		{
			postProcessor->SetPostProcessInputs(shadowMap->GetShaderResourceView(), nullptr, m_Window, BlurSigma);
		}
		if (Setting == 8)
		{
			//postProcessor->SetPostProcessInputs(ssaoRes, nullptr, m_Window, BlurSigma);
		}


		ID3D11ShaderResourceView* smaaTexture = postProcessor->prepareSmaa(m_Window, postProcessor->smaaFinalizeTex->GetShaderResourceView(0), areaTexture->GetTexture(), searchTexture->GetTexture());


		
		postProcessor->PostProcess(m_Window, smaaTexture);

		
	}

	
}

void SceneClass::AddSceneActor(Actor * a, CDeviceClass* devc)
{
	
	this->m_Actors.push_back(a);
	devc->ResetViewPort();
	devc->UpdateViewPort(1920,1080);
	
}


void SceneClass::Release()
{
	for (size_t i = 0; i < m_Actors.size(); i++)
	{
		m_Actors[i]->Release();
	}

	m_DeferredBuffer->Shutdown();
	m_DeferredShader->Release();

	postProcessTexture->Shutdown();
	m_LightShader->Release();
	m_Window->Shutdown();
	m_Camera->~FreeCamera();
	m_SkyDome->Shutdown();
	m_SkyDomeShader->Shutdown();
	postProcessor->Release();
	irradianceMap->Shutdown();
	environmentMap->Shutdown();
	terrain->Shutdown();
	terrainShader->Shutdown();
	ssaoNoiseTexture->Shutdown(); 
}

void SceneClass::RenderTerrainGPass(CDeviceClass *DevClass)
{
	DevClass->TurnCullingOn();
	DevClass->TurnZBufferOn();

	XMMATRIX projection, view, worldMatrix;

	worldMatrix = XMMatrixIdentity();
	projection = DevClass->GetProjectionMatrix();
	view = m_Camera->GetCameraView();


	XMMATRIX TerrainScale = XMMatrixScalingFromVector(XMLoadFloat3(&terrain->terrainMatrix.terrainScale));
	XMMATRIX TerrainLocation = XMMatrixTranslationFromVector(XMLoadFloat3(&terrain->terrainMatrix.terrainPosition));
	worldMatrix = TerrainScale * TerrainLocation;

	terrain->Render(DevClass->GetDevCon());

	bool res = terrainShader->Render(DevClass->GetDevCon(), terrain->GetIndexCount(), worldMatrix, view, projection,
		terrain->m_TerrainTexture[0]->GetTexture(), terrain->m_TerrainTexture[1]->GetTexture(), XMFLOAT3(0,0,0), XMFLOAT4(0,0,0, 1.0f));

}

void SceneClass::HandleSceneInput()
{
	if (Keys::key(VKEY_H))
	{
		ShadowUseFrontCulling = true;
	}
	if (Keys::key(VKEY_J))
	{
		ShadowUseFrontCulling = false;
	}
	//if (Keys::key(VKEY_LEFT_ARROW))
	//{
	//	viewPortOffSet += 1.0;
	//}
	//if (Keys::key(VKEY_RIGHT_ARROW))
	//{
	//	viewPortOffSet -= 1.0;
	//}
	if (Keys::key(VKEY_F1))
	{
		Setting = 0;
	}
	if (Keys::key(VKEY_F2))
	{
		Setting = 1;
	}
	if (Keys::key(VKEY_F3))
	{
		Setting = 2;
	}
	if (Keys::key(VKEY_F4))
	{
		ApplyPostProcess = false;
	}
	if (Keys::key(VKEY_F5))
	{
		ApplyPostProcess = true;
	}
	if (Keys::key(VKEY_F6))
	{
		Setting = 3;
	}
	if (Keys::key(VKEY_F7))
	{
		Setting = 4;
	}
	if (Keys::key(VKEY_G))
	{
		Setting = 5;
	}

	if (Keys::key(VKEY_B))
	{
		Setting = 8;
	}

	if (Keys::key(VKEY_NUM9))
	{
		if (!UseOrthoCamera) UseOrthoCamera = true;
		else UseOrthoCamera = false;
	}

}
