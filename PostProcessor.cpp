#include "stdafx.h"
#include "PostProcessor.h"
#include "FreeCamera.h"
#include <random>

PostProcessor::PostProcessor()
{
}


PostProcessor::~PostProcessor()
{
}

bool PostProcessor::InitializePostProcessor(CDeviceClass * devclass, int windowwidth, int windowheight)
{
	_width = windowwidth;
	_height = windowheight;
	this->devclass = devclass;


	bloom = new RenderTarget();
	if (!bloom->Initialize(devclass, windowwidth/2, windowheight/2, 10.0, 0.1, 0, DXGI_FORMAT_R16G16B16A16_FLOAT))
	{
		MessageBox(devclass->_mainWindow, L"Cannot init bloom rt", L"ERROR", MB_OK);
		return false;
	}

	combine = new RenderTarget();
	if (!combine->Initialize(devclass, windowwidth, windowheight, 10.0, 0.1, 0, DXGI_FORMAT_R16G16B16A16_FLOAT))
	{
		MessageBox(devclass->_mainWindow, L"Cannot init combined rt", L"ERROR", MB_OK);
		return false;
	}

	blurV = new RenderTarget();
	if (!blurV->Initialize(devclass, windowwidth/4, windowheight/4, 10.0, 0.1, 0, DXGI_FORMAT_R16G16B16A16_FLOAT))
	{
		MessageBox(devclass->_mainWindow, L"Cannot init blurv rt", L"ERROR", MB_OK);
		return false;
	}

	blurH = new RenderTarget();
	if (!blurH->Initialize(devclass, windowwidth/4, windowheight/4, 10.0, 0.1, 0, DXGI_FORMAT_R16G16B16A16_FLOAT))
	{
		MessageBox(devclass->_mainWindow, L"Cannot init blurh rt", L"ERROR", MB_OK);
		return false;
	}

	SSAO = new RenderTarget();
	if (!SSAO->Initialize(devclass, windowwidth, windowheight, 100.0, 0.1, 0, DXGI_FORMAT_R16G16B16A16_FLOAT))
	{
		MessageBox(devclass->_mainWindow, L"Cannot init SSAO rt", L"ERROR", MB_OK);
		return false;
	}

	toneMap = new RenderTarget();
	if (!toneMap->Initialize(devclass, windowwidth, windowheight, 100.0, 0.1, 0, DXGI_FORMAT_R16G16B16A16_FLOAT))
	{
		MessageBox(devclass->_mainWindow, L"Cannot init SSAO blur rt", L"ERROR", MB_OK);
		return false;
	}

	rtShader = new CTextureRenderShader();
	if (!rtShader->Initialize(devclass,devclass->_mainWindow))
	{
		MessageBox(devclass->_mainWindow, L"Cannot init rt shader", L"ERROR", MB_OK);
		return false;
	}



	//SMAA textures
	edgesTex = new RenderTarget();
	if (!edgesTex->Initialize(devclass, windowwidth, windowheight, 10.0, 0.1, 0, DXGI_FORMAT_R8G8B8A8_UNORM))
	{
		MessageBox(devclass->_mainWindow, L"Cannot init edgetex rt", L"ERROR", MB_OK);
		return false;
	}

	blendTex = new RenderTarget();
	if (!blendTex->Initialize(devclass, windowwidth, windowheight, 10.0, 0.1, 0, DXGI_FORMAT_R8G8B8A8_UNORM))
	{
		MessageBox(devclass->_mainWindow, L"Cannot init blend tex rt", L"ERROR", MB_OK);
		return false;
	}

	smaaResultTex = new RenderTarget();
	if (!smaaResultTex->Initialize(devclass, windowwidth, windowheight, 10.0, 0.1, 0, DXGI_FORMAT_R16G16B16A16_FLOAT))
	{
		MessageBox(devclass->_mainWindow, L"Cannot init blurh rt", L"ERROR", MB_OK);
		return false;
	}
	
	smaaFinalizeTex = new RenderTarget();
	if (!smaaFinalizeTex->Initialize(devclass, windowwidth, windowheight, 10.0, 0.1, 0, DXGI_FORMAT_R16G16B16A16_FLOAT))
	{
		MessageBox(devclass->_mainWindow, L"Cannot init blurh rt", L"ERROR", MB_OK);
		return false;
	}


	GenerateSSAOSamples();

	return true;
}

void PostProcessor::ClearRenderTarget()
{
}

void PostProcessor::SetPostProcessInputs(ID3D11ShaderResourceView* color, ID3D11ShaderResourceView* light, COrthoWindow* window, float exposure)
{
	this->color = color;
	devclass->ResetViewPort();
	
	//Bloom pass
	{
		bloom->ClearRenderTarget(devclass->GetDevCon(), 0.0, 0.0, 0.0, 1.0);
		bloom->SetRenderTarget(devclass->GetDevCon());

		UpdatePostProcessorMatrices();
		window->Render(devclass->GetDevCon());
	
	

		rtShader->Exposure = exposure;
		rtShader->SetSpecularHighLights(devclass->GetDevCon(), light);


		rtShader->Render(devclass->GetDevCon(), window->m_indexCount, worldMatrix, baseViewMatrix, orthoMatrix, XMFLOAT2(_width, _height));
		rtShader->RenderShaderBloom(devclass->GetDevCon(), window->m_indexCount);

	}
	
	//Blur vertical pass
	{
		devclass->ResetViewPort();
		blurV->ClearRenderTarget(devclass->GetDevCon(), 0.0, 0.0, 0.0, 1.0);
		blurV->SetRenderTarget(devclass->GetDevCon());

		UpdatePostProcessorMatrices();
		window->Render(devclass->GetDevCon());

		rtShader->SetSpecularHighLights(devclass->GetDevCon(), bloom->GetShaderResourceView(0));
		rtShader->Render(devclass->GetDevCon(), window->m_indexCount, worldMatrix, baseViewMatrix, orthoMatrix, XMFLOAT2(_width, _height));
		rtShader->RenderShaderBlurV(devclass->GetDevCon(), window->m_indexCount);

	}

	//Blur horizontal pass
	{
		devclass->ResetViewPort();
		blurH->ClearRenderTarget(devclass->GetDevCon(), 0.0, 0.0, 0.0, 1.0);
		blurH->SetRenderTarget(devclass->GetDevCon());

		UpdatePostProcessorMatrices();
		window->Render(devclass->GetDevCon());

		rtShader->SetSpecularHighLights(devclass->GetDevCon(), blurV->GetShaderResourceView(0));
		rtShader->Render(devclass->GetDevCon(), window->m_indexCount, worldMatrix, baseViewMatrix, orthoMatrix,  XMFLOAT2(_width, _height));
		rtShader->RenderShaderBlurH(devclass->GetDevCon(), window->m_indexCount);
		
	}


	//Combine the blurred and not blurred image and prepare for smaa
	{
		devclass->ResetViewPort();
		smaaFinalizeTex->ClearRenderTarget(devclass->GetDevCon(), 0.0, 0.0, 0.0, 1.0);
		smaaFinalizeTex->SetRenderTarget(devclass->GetDevCon());

		UpdatePostProcessorMatrices();
		window->Render(devclass->GetDevCon());

		rtShader->UpdateTextureIndex(devclass->GetDevCon(), color, 0);
		rtShader->UpdateTextureIndex(devclass->GetDevCon(), blurH->GetShaderResourceView(0), 1);
		
		rtShader->Render(devclass->GetDevCon(), window->m_indexCount, worldMatrix, baseViewMatrix, orthoMatrix, XMFLOAT2(_width, _height));
		rtShader->RenderShader(devclass->GetDevCon(), window->m_indexCount);
	}
}

ID3D11ShaderResourceView * PostProcessor::SmaaProcess(COrthoWindow * window, ID3D11ShaderResourceView* input, ID3D11ShaderResourceView* areaTex, ID3D11ShaderResourceView* searchTex)
{


	/*EDGE DETECTION PHASE*/
	{
		devclass->ResetViewPort();
		edgesTex->ClearRenderTarget(devclass->GetDevCon(), 0.0, 0.0, 0.0, 1.0);
		edgesTex->SetRenderTarget(devclass->GetDevCon());

		UpdatePostProcessorMatrices();
		window->Render(devclass->GetDevCon());


		rtShader->UpdateTextureIndex(devclass->GetDevCon(), input, 0);
		rtShader->Render(devclass->GetDevCon(), window->m_indexCount, worldMatrix, baseViewMatrix, orthoMatrix, XMFLOAT2(_width, _height));
		rtShader->RenderWithShaders(devclass->GetDevCon(), window->m_indexCount, rtShader->m_SMAALumaEdgeVS, rtShader->m_SMAALumaEdgePS);
	}

	/*BLENDING WEIGHTS*/
	{
		devclass->ResetViewPort();
		blendTex->ClearRenderTarget(devclass->GetDevCon(), 0.0, 0.0, 0.0, 1.0);
		blendTex->SetRenderTarget(devclass->GetDevCon());

		UpdatePostProcessorMatrices();
		window->Render(devclass->GetDevCon());

		rtShader->UpdateTextureIndex(devclass->GetDevCon(), input, 0);
		rtShader->UpdateTextureIndex(devclass->GetDevCon(), edgesTex->GetShaderResourceView(0), 6);
		rtShader->UpdateTextureIndex(devclass->GetDevCon(), areaTex, 2);
		rtShader->UpdateTextureIndex(devclass->GetDevCon(), searchTex, 5);
		rtShader->Render(devclass->GetDevCon(), window->m_indexCount, worldMatrix, baseViewMatrix, orthoMatrix, XMFLOAT2(_width, _height));
		rtShader->RenderWithShaders(devclass->GetDevCon(), window->m_indexCount, rtShader->m_SmaaBlendingWeightVS, rtShader->m_SMAABlendingWeightPS);
	}

	/*BLENDING WEIGHTS*/
	{
		devclass->ResetViewPort();
		smaaResultTex->ClearRenderTarget(devclass->GetDevCon(), 0.0, 0.0, 0.0, 1.0);
		smaaResultTex->SetRenderTarget(devclass->GetDevCon());

		UpdatePostProcessorMatrices();
		window->Render(devclass->GetDevCon());

		rtShader->UpdateTextureIndex(devclass->GetDevCon(), input, 0);
		rtShader->UpdateTextureIndex(devclass->GetDevCon(), blendTex->GetShaderResourceView(0), 7);
		rtShader->Render(devclass->GetDevCon(), window->m_indexCount, worldMatrix, baseViewMatrix, orthoMatrix, XMFLOAT2(_width, _height));
		rtShader->RenderWithShaders(devclass->GetDevCon(), window->m_indexCount, rtShader->m_SMAANeighborhoodBlendVS, rtShader->m_SMAANeighborhoodBlendPS);
	}



	
	return smaaResultTex->GetShaderResourceView(0);
}

void PostProcessor::PostProcess(COrthoWindow* window, ID3D11ShaderResourceView* colors)
{
	devclass->Begin();
	devclass->SetBackBufferRenderTarget();

	devclass->ResetViewPort();
	UpdatePostProcessorMatrices();
	window->Render(devclass->GetDevCon());

	if (useSmaa) rtShader->UpdateTextureIndex(devclass->GetDevCon(), colors, 8);
	else rtShader->UpdateTextureIndex(devclass->GetDevCon(), this->color, 8);

	
	rtShader->Render(devclass->GetDevCon(), window->m_indexCount, worldMatrix, baseViewMatrix, orthoMatrix, XMFLOAT2(_width, _height));
	rtShader->RenderWithShaders(devclass->GetDevCon(), window->m_indexCount, rtShader->m_vertexShader, rtShader->m_pixelShader);
}

ID3D11ShaderResourceView* PostProcessor::prepareSmaa(COrthoWindow * window, ID3D11ShaderResourceView* input, ID3D11ShaderResourceView * areaTex, ID3D11ShaderResourceView * searchTex)
{
	if (useSmaa)
	{
		return SmaaProcess(window, input, areaTex, searchTex);
	}
	else // If not smaa then return param color
	{
		return color;
	}
}

ID3D11ShaderResourceView * PostProcessor::CreateSSAO(CDeviceClass * devclass, COrthoWindow * window, ID3D11ShaderResourceView * pos, ID3D11ShaderResourceView * normal, 
													 ID3D11ShaderResourceView* ssaoNoise, float expss, ID3D11ShaderResourceView* tangentTexture, ID3D11ShaderResourceView* bitangentTexture,
														XMMATRIX& lightView, XMMATRIX& lightProjection, FreeCamera *cam)
{
	
	devclass->ResetViewPort();
	SSAO->ClearRenderTarget(devclass->GetDevCon(), 0.0, 0.0, 0.0, 1.0);
	SSAO->SetRenderTarget(devclass->GetDevCon());

	UpdatePostProcessorMatrices();
	window->Render(devclass->GetDevCon());
	
	rtShader->cameraPosition = cam->GetCameraPosition();
	rtShader->lightViewMatrix = XMMatrixTranspose(lightView);
	rtShader->lightProjectionMatrix = XMMatrixTranspose(lightProjection);
	rtShader->ssaoBiasAndRadius = this->ssaoBiasAndRadius;
	rtShader->Exposure = expss;
	rtShader->UpdateTextureIndex(devclass->GetDevCon(), ssaoNoise, 2);
	rtShader->UpdateTextureIndex(devclass->GetDevCon(), pos, 3); 
	rtShader->UpdateTextureIndex(devclass->GetDevCon(), normal, 4);
	rtShader->UpdateTextureIndex(devclass->GetDevCon(), tangentTexture, 9);
	rtShader->UpdateTextureIndex(devclass->GetDevCon(), bitangentTexture, 10);
	rtShader->SetShaderParameters(devclass->GetDevCon(), worldMatrix, baseViewMatrix, orthoMatrix, XMFLOAT2(_width, _height), ssaoKernel);
	rtShader->PPSSAO(devclass->GetDevCon(), window->m_indexCount);


	//Blur
	devclass->ResetViewPort();
	toneMap->ClearRenderTarget(devclass->GetDevCon(), 0.0, 0.0, 0.0, 1.0);
	toneMap->SetRenderTarget(devclass->GetDevCon());
	rtShader->Exposure = expss;
	UpdatePostProcessorMatrices();
	window->Render(devclass->GetDevCon());


	rtShader->SetSpecularHighLights(devclass->GetDevCon(), SSAO->GetShaderResourceView(0));
	rtShader->SetShaderParameters(devclass->GetDevCon(), worldMatrix, baseViewMatrix, orthoMatrix, XMFLOAT2(_width, _height), ssaoKernel);
	rtShader->RenderShaderToneMap(devclass->GetDevCon(), window->m_indexCount);


	//BlurH
	//devclass->ResetViewPort();
	//blurH->ClearRenderTarget(devclass->GetDevCon(), 0.0, 0.0, 0.0, 1.0);
	//blurH->SetRenderTarget(devclass->GetDevCon());
	//rtShader->Exposure = expss;
	//UpdatePostProcessorMatrices();
	//window->Render(devclass->GetDevCon());
	//
	//rtShader->SetSpecularHighLights(devclass->GetDevCon(), blurV->GetShaderResourceView(0));
	//rtShader->SetShaderParameters(devclass->GetDevCon(), worldMatrix, baseViewMatrix, orthoMatrix, XMFLOAT2(_width, _height), ssaoKernel);
	//rtShader->RenderShaderBlurH(devclass->GetDevCon(), window->m_indexCount);

	return toneMap->GetShaderResourceView(0);
}

ID3D11ShaderResourceView * PostProcessor::BlurShadows(CDeviceClass * devclass, COrthoWindow * window, ID3D11ShaderResourceView * pos)
{

	//Blur vertical pass
	{
		devclass->ResetViewPort();
		blurV->ClearRenderTarget(devclass->GetDevCon(), 0.0, 0.0, 0.0, 1.0);
		blurV->SetRenderTarget(devclass->GetDevCon());

		UpdatePostProcessorMatrices();
		window->Render(devclass->GetDevCon());
		rtShader->Exposure = 1.0;
		rtShader->SetSpecularHighLights(devclass->GetDevCon(), pos);
		rtShader->Render(devclass->GetDevCon(), window->m_indexCount, worldMatrix, baseViewMatrix, orthoMatrix, XMFLOAT2(_width, _height));
		rtShader->RenderShaderBlurV(devclass->GetDevCon(), window->m_indexCount);

	}

	//Blur horizontal pass
	{
		devclass->ResetViewPort();
		blurH->ClearRenderTarget(devclass->GetDevCon(), 0.0, 0.0, 0.0, 1.0);
		blurH->SetRenderTarget(devclass->GetDevCon());
		rtShader->Exposure = 1.0;
		UpdatePostProcessorMatrices();
		window->Render(devclass->GetDevCon());

		rtShader->SetSpecularHighLights(devclass->GetDevCon(), blurV->GetShaderResourceView(0));
		rtShader->Render(devclass->GetDevCon(), window->m_indexCount, worldMatrix, baseViewMatrix, orthoMatrix, XMFLOAT2(_width, _height));
		rtShader->RenderShaderBlurH(devclass->GetDevCon(), window->m_indexCount);

	}

	return blurH->GetShaderResourceView(0);
}

void PostProcessor::DebugGBufferTextures(DeferredBuffersClass * buf, CDeviceClass * devclass, COrthoWindow * window)
{
}



void PostProcessor::Release()
{
	bloom->Shutdown();
	blurH->Shutdown();
	blurV->Shutdown();
	blendTex->Shutdown();
	edgesTex->Shutdown();
	smaaFinalizeTex->Shutdown();
	smaaResultTex->Shutdown();
	toneMap->Shutdown();
	color->Release();
	combine->Shutdown();
	SSAO->Shutdown();
	rtShader->Shutdown();
	//ssaoRand->Shutdown();
}

void PostProcessor::GenerateSSAOSamples()
{
	std::uniform_real_distribution<float> randomFloats(0.0f, 1.0f);
	std::default_random_engine gen;

	for (size_t i = 0; i < 64; i++)
	{
		XMFLOAT4 samplef;
		XMVECTOR sample = XMVectorSet(randomFloats(gen) * 2.0 - 1.0, randomFloats(gen) * 2.0 - 1.0, randomFloats(gen), 1.0f);
		sample = XMVector3Normalize(sample);
		
		XMStoreFloat4(&samplef, sample);
		
		float rnd = randomFloats(gen);

		samplef.x *= rnd;
		samplef.y *= rnd;
		samplef.z *= rnd;
		samplef.w *= rnd;

		float scale = (float)i / 64.0f;

		scale = lerp(0.1f, 1.0f, scale* scale);

		samplef.x *= scale;
		samplef.y *= scale;
		samplef.z *= scale;
		samplef.w *= scale;

		ssaoKernel.push_back(samplef);
	}
	int iasd = 0;
}

void PostProcessor::UpdatePostProcessorMatrices()
{
	worldMatrix = XMMatrixIdentity();
	orthoMatrix = devclass->GetOrthoMatrix();
	baseViewMatrix = XMMatrixLookAtLH(XMVectorSet(0.0f, 0.0f, -1.0f, 1.0f), XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f), XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f));
	rtShader->SetProjectionMatix(devclass->GetProjectionMatrix());
}


