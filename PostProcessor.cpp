#include "stdafx.h"
#include "PostProcessor.h"


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
	if (!bloom->Initialize(devclass, windowwidth/8, windowheight/8, 10.0, 0.1, 0, DXGI_FORMAT_R16G16B16A16_FLOAT))
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
	if (!blurV->Initialize(devclass, windowwidth/8, windowheight/8, 10.0, 0.1, 0, DXGI_FORMAT_R16G16B16A16_FLOAT))
	{
		MessageBox(devclass->_mainWindow, L"Cannot init blurv rt", L"ERROR", MB_OK);
		return false;
	}

	blurH = new RenderTarget();
	if (!blurH->Initialize(devclass, windowwidth/8, windowheight/8, 10.0, 0.1, 0, DXGI_FORMAT_R16G16B16A16_FLOAT))
	{
		MessageBox(devclass->_mainWindow, L"Cannot init blurh rt", L"ERROR", MB_OK);
		return false;
	}


	rtShader = new CTextureRenderShader();
	if (!rtShader->Initialize(devclass,devclass->_mainWindow))
	{
		MessageBox(devclass->_mainWindow, L"Cannot init rt shader", L"ERROR", MB_OK);
		return false;
	}

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


		rtShader->Render(devclass->GetDevCon(), window->m_indexCount, worldMatrix, baseViewMatrix, orthoMatrix, color, XMFLOAT2(_width, _height));
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
		rtShader->Render(devclass->GetDevCon(), window->m_indexCount, worldMatrix, baseViewMatrix, orthoMatrix, color, XMFLOAT2(_width, _height));
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
		rtShader->Render(devclass->GetDevCon(), window->m_indexCount, worldMatrix, baseViewMatrix, orthoMatrix, color, XMFLOAT2(_width, _height));
		rtShader->RenderShaderBlurH(devclass->GetDevCon(), window->m_indexCount);

	}


	
}

void PostProcessor::PostProcess(COrthoWindow* window)
{
	//Final result
	
	devclass->ResetViewPort();
	UpdatePostProcessorMatrices();
	window->Render(devclass->GetDevCon());

	rtShader->SetSpecularHighLights(devclass->GetDevCon(), blurH->GetShaderResourceView(0));
	rtShader->Render(devclass->GetDevCon(), window->m_indexCount, worldMatrix, baseViewMatrix, orthoMatrix, this->color, XMFLOAT2(_width, _height));
	rtShader->RenderShader(devclass->GetDevCon(), window->m_indexCount);
	
}

void PostProcessor::Release()
{
	bloom->Shutdown();
	blurH->Shutdown();
	blurV->Shutdown();
	combine->Shutdown();
	
	rtShader->Shutdown();
}

void PostProcessor::UpdatePostProcessorMatrices()
{
	worldMatrix = XMMatrixIdentity();
	orthoMatrix = devclass->GetOrthoMatrix();
	baseViewMatrix = XMMatrixLookAtLH(XMVectorSet(0.0f, 0.0f, -1.0f, 1.0f), XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f), XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f));

}


