#pragma once


#ifndef SCENE_H
#define SCENE_H
#include "DeviceClass.h"
#include "Actor.h"
#include "DeferredBufferClass.h"
#include "DeferredShader.h"
#include "LightShader.h"
#include "FreeCamera.h"
#include "OrthoWindow.h"
#include "RenderToTexture.h"
#include "TextureRenderShader.h"
#include "Lights.h"
#include "ShadowMapRenderTarget.h"
#include "DepthShader.h"
#include "Skydome.h"
#include "SkyDomeShader.h"

#include <vector>

class SceneClass
{
public:
	SceneClass();
	~SceneClass();

	void InitializeScene(CDeviceClass *DevClass, int scenewidth, int sceneheight, HWND hWnd);

	void ShadowPass(CDeviceClass *DevClass);
	void GeometryPass(CDeviceClass *DevClass);
	void LightPass(CDeviceClass *DevClass);

	void AddSceneActor(Actor* a, CDeviceClass* devc);



	void Release();	
	double GeoBenchMarks[4];

	int viewPortOffSet = -330;
	std::string SceneName = ""; 

	std::vector<Actor*> m_Actors;
	std::vector<PointLight> pointLights;

	 
	float BlurSigma = 11.0f;
	DirectionalLight dirLight;
	
	CSkydome* GetSkyDome() { return this->m_SkyDome; }
	FreeCamera* GetCamera() { return this->m_Camera; }
private:

	void HandleSceneInput();

	int ResetBenchMark = 0;
	int Setting = 0;
	const int POINT_LIGHT_COUNT = 40;

	bool ShadowUseFrontCulling = false;

	CSkydome* m_SkyDome;
	CSkyDomeShader* m_SkyDomeShader;

	ShadowMapRenderTarget* shadowMap;

	CDepthShader *m_ShadowShader;
	DeferredBuffersClass* m_DeferredBuffer;
	DeferredShader* m_DeferredShader;
	LightShader* m_LightShader;
	
	FreeCamera* m_Camera;
	
	CTextureRenderShader* textureShader;
	CRenderToTexture* specularHighLight;

	COrthoWindow* m_Window;
	HWND mainWindow;

	bool ApplyPostProcess = true;
	int _sceneWidth, _sceneHeight;	


	CRenderToTexture* postProcessTexture;

};



#endif