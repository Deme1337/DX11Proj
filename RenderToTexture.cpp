#include "stdafx.h"
#include "RenderToTexture.h"




CRenderToTexture::CRenderToTexture()
{
	for (size_t i = 0; i < BUFFER_COUNT_PP; i++)
	{
		m_renderTargetTexture[i] = 0;
		m_renderTargetView[i] = 0;
		m_shaderResourceView[i] = 0;
	}

	m_depthStencilBuffer = 0;
	m_depthStencilView = 0;
}


CRenderToTexture::CRenderToTexture(const CRenderToTexture& other)
{
}


CRenderToTexture::~CRenderToTexture()
{
}


bool CRenderToTexture::Initialize(CDeviceClass* devclass, int textureWidth, int textureHeight, float screenDepth, float screenNear, int samplecount)
{
	D3D11_TEXTURE2D_DESC textureDesc;
	HRESULT result;
	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
	D3D11_TEXTURE2D_DESC depthBufferDesc;
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;




	// Initialize the render target texture description.
	ZeroMemory(&textureDesc, sizeof(textureDesc));


	// Setup the render target texture description.
	textureDesc.Width = textureWidth;
	textureDesc.Height = textureHeight;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	if (samplecount > 1)
	{
		textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	}
	else
	{
		//textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		textureDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	}

	textureDesc.SampleDesc.Count = samplecount;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	for (size_t i = 0; i < BUFFER_COUNT_PP; i++)
	{
		// Create the render target texture.
		result = devclass->GetDevice()->CreateTexture2D(&textureDesc, NULL, &m_renderTargetTexture[i]);
		if (FAILED(result))
		{
			return false;
		}
	}


	// Setup the description of the render target view.
	renderTargetViewDesc.Format = textureDesc.Format;
	if (samplecount > 1)
	{
		renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS;
	}
	else
	{
		renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	}
	renderTargetViewDesc.Texture2D.MipSlice = 0;


	for (size_t i = 0; i < BUFFER_COUNT_PP; i++)
	{
		// Create the render target view.
		result = devclass->GetDevice()->CreateRenderTargetView(m_renderTargetTexture[i], &renderTargetViewDesc, &m_renderTargetView[i]);
		if (FAILED(result))
		{
			return false;
		}
	}


	// Setup the description of the shader resource view.
	shaderResourceViewDesc.Format = textureDesc.Format;

	if (samplecount > 1)
	{
		shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMS;
	}
	else
	{
		shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	}
	
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;

	for (size_t i = 0; i < BUFFER_COUNT_PP; i++)
	{
		// Create the shader resource view.
		result = devclass->GetDevice()->CreateShaderResourceView(m_renderTargetTexture[i], &shaderResourceViewDesc, &m_shaderResourceView[i]);
		if (FAILED(result))
		{
			return false;
		}
	}


	// Initialize the description of the depth buffer.
	ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));

	// Set up the description of the depth buffer.
	depthBufferDesc.Width = textureWidth;
	depthBufferDesc.Height = textureHeight;
	depthBufferDesc.MipLevels = 1;
	depthBufferDesc.ArraySize = 1;
	depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthBufferDesc.SampleDesc.Count = samplecount;
	depthBufferDesc.SampleDesc.Quality = 0;
	depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthBufferDesc.CPUAccessFlags = 0;
	depthBufferDesc.MiscFlags = 0;

	// Create the texture for the depth buffer using the filled out description.
	result = devclass->GetDevice()->CreateTexture2D(&depthBufferDesc, NULL, &m_depthStencilBuffer);
	if (FAILED(result))
	{
		return false;
	}

	// Initailze the depth stencil view description.
	ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));

	// Set up the depth stencil view description.
	depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

	if (samplecount > 1)
	{
		depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
		depthStencilViewDesc.Texture2D.MipSlice = 1;
	}
	else
	{
		depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		depthStencilViewDesc.Texture2D.MipSlice = 0;
	}
	
	

	// Create the depth stencil view.
	result = devclass->GetDevice()->CreateDepthStencilView(m_depthStencilBuffer, &depthStencilViewDesc, &m_depthStencilView);
	if (FAILED(result))
	{
		return false;
	}

	// Setup the viewport for rendering.
	m_viewport.Width = (float)textureWidth;
	m_viewport.Height = (float)textureHeight;
	m_viewport.MinDepth = 0.0f;
	m_viewport.MaxDepth = 1.0f;
	m_viewport.TopLeftX = 0.0f;
	m_viewport.TopLeftY = 0.0f;

	// Setup the projection matrix.
	m_projectionMatrix = XMMatrixPerspectiveFovLH(XM_PI / 4.0f, ((float)textureWidth / (float)textureHeight), screenNear, screenDepth);
	// Create an orthographic projection matrix for 2D rendering.
	m_orthoMatrix = XMMatrixOrthographicLH((float)textureWidth, (float)textureHeight, screenNear, screenDepth);
	return true;
}


void CRenderToTexture::Shutdown()
{
	for (size_t i = 0; i < BUFFER_COUNT_PP; i++)
	{
		SafeRelease(m_renderTargetTexture[i]);
		SafeRelease(m_renderTargetView[i]);
		SafeRelease(m_shaderResourceView[i]);
	}
	SafeRelease(m_depthStencilBuffer);
	SafeRelease(m_depthStencilBuffer);
	SafeRelease(m_depthStencilView);

	return;
}


void CRenderToTexture::SetRenderTargets(ID3D11DeviceContext *)
{
}

void CRenderToTexture::SetRenderTarget(ID3D11DeviceContext* deviceContext)
{
	// Bind the render target view and depth stencil buffer to the output render pipeline.


	deviceContext->OMSetRenderTargets(BUFFER_COUNT_PP, m_renderTargetView, nullptr);

	// Set the viewport.
	deviceContext->RSSetViewports(1, &m_viewport);

	return;
}


void CRenderToTexture::ClearRenderTarget(ID3D11DeviceContext* deviceContext, float red, float green, float blue, float alpha)
{
	float color[4];


	// Setup the color to clear the buffer to.
	color[0] = red;
	color[1] = green;
	color[2] = blue;
	color[3] = alpha;

	// Clear the back buffer.
	for (size_t i = 0; i < BUFFER_COUNT_PP; i++)
	{
		deviceContext->ClearRenderTargetView(m_renderTargetView[i], color);

	}

	// Clear the depth buffer.
	//deviceContext->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

	return;
}


ID3D11ShaderResourceView* CRenderToTexture::GetShaderResourceView(int index)
{
	return m_shaderResourceView[index];
}



ID3D11Texture2D * CRenderToTexture::GetSubResource()
{
	return nullptr;
}

void CRenderToTexture::SetTexture(ID3D11Texture2D* texture)
{
	return;
}

void CRenderToTexture::GetProjectionMatrix(XMMATRIX& projectionMatrix)
{
	projectionMatrix = m_projectionMatrix;
	return;
}


void CRenderToTexture::GetOrthoMatrix(XMMATRIX& orthoMatrix)
{
	orthoMatrix = m_orthoMatrix;
	return;
}