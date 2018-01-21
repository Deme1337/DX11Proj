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
#include "PostProcessor.h"
#include "Terrain.h"
#include "TerrainShader.h"

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

	XMFLOAT4 subspectintani = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	XMFLOAT4 sheentintcleargloss = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);

	void Release();	
	double GeoBenchMarks[4];

	int viewPortOffSet = -330;
	std::string SceneName = ""; 

	CTerrain *terrain;
	CSkydome *m_SkyDome;

	std::vector<Actor*> m_Actors;
	std::vector<PointLight> pointLights;

	float maxObjectDrawDistance = 5000.0f;
	float minObjectDrawDistance = -5000.0f;

	bool DrawTerrain = false;
	 
	float BlurSigma = 19.0f;
	XMFLOAT2 ssaoBiasAndRadius;
	DirectionalLight dirLight;
	
	CSkydome* GetSkyDome() { return this->m_SkyDome; }
	FreeCamera* GetCamera() { return this->m_Camera; }
private:
	void RenderTerrainGPass(CDeviceClass *DevClass);

	void HandleSceneInput();

	int ResetBenchMark = 0;
	int Setting = 0;
	float renderScale = 1.0f;
	const int POINT_LIGHT_COUNT = 40;

	bool ShadowUseFrontCulling = false;
	bool UseOrthoCamera = false;

	CSkyDomeShader* m_SkyDomeShader;
	CTerrainShader *terrainShader;

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



	PostProcessor* postProcessor;
	CRenderToTexture* postProcessTexture;
	
	CTextureTA* environmentMap;
	CTextureTA* irradianceMap;
	CTextureTA* areaTexture;
	CTextureTA* searchTexture;
	CTextureTA* ssaoNoiseTexture;
};



#endif