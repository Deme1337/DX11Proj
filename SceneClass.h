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

	void AddSceneActor(Actor* a) { m_Actors.push_back(a); }

	void Release();	
	double GeoBenchMarks[4];

	int viewPortOffSet = -330;

	std::vector<Actor*> m_Actors;
private:

	void HandleSceneInput();

	int ResetBenchMark = 0;
	int Setting = 0;

	DirectionalLight dirLight;

	ShadowMapRenderTarget* shadowMap;

	CDepthShader *m_ShadowShader;
	DeferredBuffersClass* m_DeferredBuffer;
	DeferredShader* m_DeferredShader;
	LightShader* m_LightShader;
	FreeCamera* m_Camera;

	CRenderToTexture* postProcessTexture;
	CTextureRenderShader* textureShader;
	CRenderToTexture* specularHighLight;

	COrthoWindow* m_Window;

	bool ApplyPostProcess = true;
	int _sceneWidth, _sceneHeight;	



	//XMVECTOR lightPosition = XMVectorSet(10, 500, 10, 1.0);

};



#endif