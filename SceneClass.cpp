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
	
	m_DeferredBuffer->Initialize(DevClass->GetDevice(),scenewidth*2, sceneheight*2, 10, 0.1);


	m_DeferredShader = new DeferredShader();
	
	m_DeferredShader->Initialize(DevClass, L"deferredvs.hlsl", L"deferredps.hlsl");

	
	m_LightShader = new LightShader();
	
	m_LightShader->Initialize(DevClass, L"lightvs.hlsl", L"lightps.hlsl");


	m_Window = new COrthoWindow();

	m_Window->Initialize(DevClass->GetDevice(), scenewidth, sceneheight);


	postProcessTexture = new CRenderToTexture();

	postProcessTexture->Initialize(DevClass, scenewidth, sceneheight, 10.0, 0.1f, 1);



	textureShader = new CTextureRenderShader();

	textureShader->Initialize(DevClass->GetDevice(), hWnd);
	textureShader->Exposure = 5.5;

	//Lights and shadow map rt
	dirLight.lightProperties.Position = XMFLOAT4(300.0f, 1500.0f, -400.0f, 1.0f);
	dirLight.CalcLightViewMatrix();
	dirLight.CalcProjectionMatrix();
	dirLight.lightProperties.Color = XMFLOAT4(0.984, 0.946, 0.89, 1.0f);

	shadowMap = new ShadowMapRenderTarget();
	
	if (!shadowMap->Initialize(DevClass, 2048*2, 2048*2, 10000.0f, 0.1f))
	{
		MessageBox(NULL, L"Shadow map creation error", L"ERROR", MB_OK);
	}

	m_ShadowShader = new CDepthShader();

	m_ShadowShader->Initialize(DevClass->GetDevice(), NULL);

	m_SkyDome = new CSkydome();

	m_SkyDome->Initialize(DevClass->GetDevice());
	

	m_SkyDomeShader = new CSkyDomeShader();

	m_SkyDomeShader->Initialize(DevClass->GetDevice(), hWnd);


	m_Camera = new FreeCamera(hWnd);
	m_Camera->SetCameraPosition(XMVectorSet(0, 10, -10, 1.0));


	for (size_t i = 0; i < POINT_LIGHT_COUNT; i++)
	{
		PointLight p = PointLight();
		p.lightProperties.Position = XMFLOAT4(50 * i, 10, i * 20, 1.0f);
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
		worlMatrix = XMMatrixIdentity();
		viewMatrix = dirLight.GetLightViewMatrix();
		projectionMatrix = dirLight.GetLightProjectionMatrix();

		m_ShadowShader->Render(DevClass->GetDevCon(), 0, m_Actors[i]->modelMatrix, viewMatrix, projectionMatrix);
		m_Actors[i]->RenderShadowMap(DevClass, m_ShadowShader);
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
	view = m_Camera->GetCameraView();

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

		if (!m_SkyDomeShader->Update(DevClass->GetDevCon(), m_SkyDome->GetIndexCount(),
			worldSphere, view, projection, m_SkyDome->GetApexColor(), m_SkyDome->GetCenterColor(), dirLight))
		{
			MessageBox(NULL, L"Error skydome rendering", L"ERROR", MB_OK);
		}
	}
	DevClass->TurnCullingOn();
	DevClass->TurnZBufferOn();

	projection = DevClass->GetProjectionMatrix();
	view = m_Camera->GetCameraView();

	DevClass->TurnCullingOn();
	DevClass->TurnZBufferOn();

	TimeVar time2 = timeNow();
	for (size_t i = 0; i < m_Actors.size(); i++)
	{

		TimeVar time3 = timeNow();
		m_DeferredShader->UpdateShader(DevClass, m_Actors[i]->modelMatrix, view, projection, m_Actors[i]->HasAlpha);
		double geo2New = duration(timeNow() - time3);
		GeoBenchMarks[2] = geo2New;
		

		m_Actors[i]->RenderModel(DevClass, m_DeferredShader);
		
		
		projection = DevClass->GetProjectionMatrix();
		view = m_Camera->GetCameraView();
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
	
	m_LightShader->UpdateShadowMap(DevClass, shadowMap->GetShaderResourceView());
	
	//Settings to show all textures passed 
	if (Setting == 0)
	{
		m_LightShader->UpdateCameraPosition(DevClass, m_Camera->GetCameraPosition());
		m_LightShader->UpdateShaderParameters(DevClass, worldMatrix, baseViewMatrix, orthoMatrix, m_DeferredBuffer->GetShaderResourceView(0), m_DeferredBuffer->GetShaderResourceView(1), m_DeferredBuffer->GetShaderResourceView(2), m_DeferredBuffer->GetShaderResourceView(3),m_DeferredBuffer->GetShaderResourceView(4), dirLight, pointLights);

	}
	if (Setting == 1)
	{
		m_LightShader->UpdateCameraPosition(DevClass, m_Camera->GetCameraPosition());
		m_LightShader->UpdateShaderParameters(DevClass, worldMatrix, baseViewMatrix, orthoMatrix, m_DeferredBuffer->GetShaderResourceView(1), m_DeferredBuffer->GetShaderResourceView(1), m_DeferredBuffer->GetShaderResourceView(2), m_DeferredBuffer->GetShaderResourceView(3), m_DeferredBuffer->GetShaderResourceView(4), dirLight, pointLights);
	}

	if (Setting == 2)
	{
		m_LightShader->UpdateCameraPosition(DevClass, m_Camera->GetCameraPosition());
		m_LightShader->UpdateShaderParameters(DevClass, worldMatrix, baseViewMatrix, orthoMatrix, m_DeferredBuffer->GetShaderResourceView(2), m_DeferredBuffer->GetShaderResourceView(1), m_DeferredBuffer->GetShaderResourceView(2), m_DeferredBuffer->GetShaderResourceView(3), m_DeferredBuffer->GetShaderResourceView(4), dirLight, pointLights);
	}

	if (Setting == 3)
	{
		m_LightShader->UpdateCameraPosition(DevClass, m_Camera->GetCameraPosition());
		m_LightShader->UpdateShaderParameters(DevClass, worldMatrix, baseViewMatrix, orthoMatrix, m_DeferredBuffer->GetShaderResourceView(3), m_DeferredBuffer->GetShaderResourceView(1), m_DeferredBuffer->GetShaderResourceView(2), m_DeferredBuffer->GetShaderResourceView(3), m_DeferredBuffer->GetShaderResourceView(4), dirLight, pointLights);
	}

	if (Setting == 4)
	{
		m_LightShader->UpdateCameraPosition(DevClass, m_Camera->GetCameraPosition());
		m_LightShader->UpdateShaderParameters(DevClass, worldMatrix, baseViewMatrix, orthoMatrix, m_DeferredBuffer->GetShaderResourceView(4), m_DeferredBuffer->GetShaderResourceView(1), m_DeferredBuffer->GetShaderResourceView(2), m_DeferredBuffer->GetShaderResourceView(3), m_DeferredBuffer->GetShaderResourceView(4), dirLight, pointLights);
	}
	

	m_Window->Render(DevClass->GetDevCon());
	m_LightShader->Update(DevClass, m_Window->m_indexCount);


	DevClass->ResetViewPort();


	//Postprocess 
	if (ApplyPostProcess)
	{

		DevClass->Begin();
		DevClass->SetBackBufferRenderTarget();

		m_Window->UpdateWindow(DevClass->GetDevCon(), viewPortOffSet, 0);
		m_Window->Render(DevClass->GetDevCon());


		worldMatrix = XMMatrixIdentity();
		orthoMatrix = DevClass->GetOrthoMatrix();
		baseViewMatrix = XMMatrixLookAtLH(XMVectorSet(0.0f, 0.0f, -1.0f, 1.0f), XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f), XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f));
		
		if (Setting == 5)
		{
			textureShader->Exposure = BlurSigma;
			textureShader->SetSpecularHighLights(DevClass->GetDevCon(), nullptr);

			textureShader->Render(DevClass->GetDevCon(), m_Window->m_indexCount, worldMatrix, baseViewMatrix, orthoMatrix, shadowMap->GetShaderResourceView(), XMFLOAT2(_sceneWidth, _sceneHeight));
			textureShader->RenderShader(DevClass->GetDevCon(), m_Window->m_indexCount);
		}
		else
		{
			textureShader->Exposure = BlurSigma;
			textureShader->SetSpecularHighLights(DevClass->GetDevCon(), postProcessTexture->GetShaderResourceView(1));

			textureShader->Render(DevClass->GetDevCon(), m_Window->m_indexCount, worldMatrix, baseViewMatrix, orthoMatrix, postProcessTexture->GetShaderResourceView(0), XMFLOAT2(_sceneWidth, _sceneHeight));
			textureShader->RenderShader(DevClass->GetDevCon(), m_Window->m_indexCount);
		}
	
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
	if (Keys::key(VKEY_LEFT_ARROW))
	{
		viewPortOffSet += 1.0;
	}
	if (Keys::key(VKEY_RIGHT_ARROW))
	{
		viewPortOffSet -= 1.0;
	}
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

}
