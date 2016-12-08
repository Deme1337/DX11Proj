#include "stdafx.h"
#include "TextureRenderShader.h"

using namespace std;



CTextureRenderShader::CTextureRenderShader()
{
	m_vertexShader = 0;
	m_pixelShader = 0;
	m_layout = 0;
	m_matrixBuffer = 0;
	m_sampleState = 0;
}


CTextureRenderShader::CTextureRenderShader(const CTextureRenderShader& other)
{
}


CTextureRenderShader::~CTextureRenderShader()
{
}


bool CTextureRenderShader::Initialize(CDeviceClass *devclass, HWND hwnd)
{
	bool result;


	// Initialize the vertex and pixel shaders.
	result = InitializeShader(devclass, hwnd, L"TextureVS.hlsl", L"TexturePS.hlsl");
	if (!result)
	{
		return false;
	}

	return true;
}


void CTextureRenderShader::Shutdown()
{
	// Shutdown the vertex and pixel shaders as well as the related objects.
	ShutdownShader();

	return;
}

void CTextureRenderShader::UpdateTextureIndex(ID3D11DeviceContext * devcon, ID3D11ShaderResourceView * tex, int index)
{
	// Set shader texture resource in the pixel shader.
	devcon->PSSetShaderResources(index, 1, &tex);
}

void CTextureRenderShader::SetSpecularHighLights(ID3D11DeviceContext* devcon,ID3D11ShaderResourceView * tex)
{
	// Set shader texture resource in the pixel shader.
	devcon->PSSetShaderResources(1, 1, &tex);
}


bool CTextureRenderShader::Render(ID3D11DeviceContext* deviceContext, int indexCount, XMMATRIX& worldMatrix, XMMATRIX& viewMatrix,
	XMMATRIX& projectionMatrix, ID3D11ShaderResourceView* texture, XMFLOAT2 swh)
{
	bool result;


	// Set the shader parameters that it will use for rendering.
	std::vector<XMVECTOR> nullvec;
	result = SetShaderParameters(deviceContext, worldMatrix, viewMatrix, projectionMatrix, texture, swh,nullvec);
	if (!result)
	{
		return false;
	}

	return true;
}



bool CTextureRenderShader::InitializeShader(CDeviceClass *devclass, HWND hwnd, WCHAR* vsFilename, WCHAR* psFilename)
{
	HRESULT result;
	ID3D10Blob* errorMessage;
	ID3D10Blob* vertexShaderBuffer; ID3D10Blob* vertexShaderBufferSMAAED; ID3D10Blob* vertexShaderBufferSMAAEC; ID3D10Blob* vertexShaderBufferSMAAE;
	ID3D10Blob* pixelShaderBuffer; ID3D10Blob* pixelShaderBufferBlurV; ID3D10Blob* pixelShaderBufferBlurH; ID3D10Blob* pixelShaderBufferBloom; ID3D10Blob* pixelShaderBufferColor;
	ID3D10Blob* pixelshaderBufferSSAO;
	ID3D10Blob* pixelShaderBufferSMAAED; ID3D10Blob* pixelShaderBufferSMAAEC; ID3D10Blob* pixelShaderBufferSMAAE;
	D3D11_INPUT_ELEMENT_DESC polygonLayout[2];
	unsigned int numElements; 
	D3D11_BUFFER_DESC matrixBufferDesc, postProcessBufferDesc;
	D3D11_SAMPLER_DESC samplerDesc;

	this->devclass = devclass;

	// Initialize the pointers this function will use to null.
	errorMessage = 0;
	vertexShaderBuffer = 0;
	pixelShaderBuffer = 0;

	pixelShaderBufferColor = CDeviceClass::CompileShader(psFilename, PixelShader, "ReturnTexture");
	pixelshaderBufferSSAO  = CDeviceClass::CompileShader(psFilename, PixelShader, "SSAO");
	pixelShaderBuffer	   = CDeviceClass::CompileShader(psFilename, PixelShader, "Combine");
	pixelShaderBufferBlurV = CDeviceClass::CompileShader(psFilename, PixelShader, "BlurVertical");
	pixelShaderBufferBlurH = CDeviceClass::CompileShader(psFilename, PixelShader, "BlurHorizontal");
	pixelShaderBufferBloom = CDeviceClass::CompileShader(psFilename, PixelShader, "BloomColors");
	vertexShaderBuffer     = CDeviceClass::CompileShader(vsFilename, VertexShader, "TextureVertexShader");

#ifdef SMAA_1

	vertexShaderBufferSMAAE  = CDeviceClass::CompileShader(vsFilename, VertexShader, "SMAAEdgeDetectionVS1");
	vertexShaderBufferSMAAED = CDeviceClass::CompileShader(vsFilename, VertexShader, "SMAABlendingWeightCalculationVS1");
	vertexShaderBufferSMAAEC = CDeviceClass::CompileShader(vsFilename, VertexShader, "SMAANeighborhoodBlendingVS1");

	pixelShaderBufferSMAAE  = CDeviceClass::CompileShader(psFilename, PixelShader, "SMAALumaEdgeDetectionPS");
	pixelShaderBufferSMAAED = CDeviceClass::CompileShader(psFilename, PixelShader, "SMAAColorEdgeDetectionPS");
	pixelShaderBufferSMAAEC = CDeviceClass::CompileShader(psFilename, PixelShader, "SMAADepthEdgeDetectionPS");

#endif

	// Create the vertex shader from the buffer.
	result = devclass->GetDevice()->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL, &m_vertexShader);
	if (FAILED(result))
	{
		return false;
	}

#ifdef SMAA_1
	//Smaa vertex shader
	{
		result = devclass->GetDevice()->CreateVertexShader(vertexShaderBufferSMAAE->GetBufferPointer(), vertexShaderBufferSMAAE->GetBufferSize(), NULL, &m_SMAALumaEdgeVS);
		if (FAILED(result))
		{
			return false;
		}

		result = devclass->GetDevice()->CreateVertexShader(vertexShaderBufferSMAAEC->GetBufferPointer(), vertexShaderBufferSMAAEC->GetBufferSize(), NULL, &m_SMAAColorEdgeVS);
		if (FAILED(result))
		{
			return false;
		}

		result = devclass->GetDevice()->CreateVertexShader(vertexShaderBufferSMAAED->GetBufferPointer(), vertexShaderBufferSMAAED->GetBufferSize(), NULL, &m_SMAAColorEdgeVS);
		if (FAILED(result))
		{
			return false;
		}
	}
#endif

	// Create the pixel shader from the buffer.
	result = devclass->GetDevice()->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, &m_pixelShader);
	if (FAILED(result))
	{
		return false;
	}

	// Create the pixel shader from the buffer.
	result = devclass->GetDevice()->CreatePixelShader(pixelShaderBufferColor->GetBufferPointer(), pixelShaderBufferColor->GetBufferSize(), NULL, &m_pixelShaderColor);
	if (FAILED(result))
	{
		return false;
	}

	//blur vertical shader
	result = devclass->GetDevice()->CreatePixelShader(pixelShaderBufferBlurV->GetBufferPointer(), pixelShaderBufferBlurV->GetBufferSize(), NULL, &m_pixelShaderBlurV);
	if (FAILED(result))
	{
		return false;
	}

	//blur horizontal shader
	result = devclass->GetDevice()->CreatePixelShader(pixelShaderBufferBlurH->GetBufferPointer(), pixelShaderBufferBlurH->GetBufferSize(), NULL, &m_pixelShaderBlurH);
	if (FAILED(result))
	{
		return false;
	}

	//bloom shader
	result = devclass->GetDevice()->CreatePixelShader(pixelShaderBufferBloom->GetBufferPointer(), pixelShaderBufferBloom->GetBufferSize(), NULL, &m_pixelShaderBloom);
	if (FAILED(result))
	{
		return false;
	}

	// SSAO
	result = devclass->GetDevice()->CreatePixelShader(pixelshaderBufferSSAO->GetBufferPointer(), pixelshaderBufferSSAO->GetBufferSize(), NULL, &m_pixelShaderSSAO);
	if (FAILED(result))
	{
		return false;
	}

#ifdef SMAA_1
	//SMAA Pixel Shaders
	{
		//Smaa luma edge detection
		result = devclass->GetDevice()->CreatePixelShader(pixelShaderBufferSMAAE, pixelShaderBufferSMAAE->GetBufferSize(), NULL, &m_SMAALumaEdgePS);
		if (FAILED(result))
		{
			return false;
		}



		//Smaa ColorEdgeDetectionPS
		result = devclass->GetDevice()->CreatePixelShader(pixelShaderBufferSMAAEC->GetBufferPointer(), pixelShaderBufferSMAAEC->GetBufferSize(), NULL, &m_SMAAColorEdgePS);
		if (FAILED(result))
		{
			return false;
		}

		//SMaa DepthEdgeDetectionPS
		result = devclass->GetDevice()->CreatePixelShader(pixelShaderBufferSMAAED->GetBufferPointer(), pixelShaderBufferSMAAED->GetBufferSize(), NULL, &m_SMAALumaEdgePS);
		if (FAILED(result))
		{
			return false;
		}
	}
#endif


	// Create the vertex input layout description.
	polygonLayout[0].SemanticName = "POSITION";
	polygonLayout[0].SemanticIndex = 0;
	polygonLayout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	polygonLayout[0].InputSlot = 0;
	polygonLayout[0].AlignedByteOffset = 0;
	polygonLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[0].InstanceDataStepRate = 0;

	polygonLayout[1].SemanticName = "TEXCOORD";
	polygonLayout[1].SemanticIndex = 0;
	polygonLayout[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	polygonLayout[1].InputSlot = 0;
	polygonLayout[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[1].InstanceDataStepRate = 0;

	// Get a count of the elements in the layout.
	numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

	// Create the vertex input layout.
	result = devclass->GetDevice()->CreateInputLayout(polygonLayout, numElements, vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(),
		&m_layout);
	if (FAILED(result))
	{
		return false;
	}

#ifdef SMAA_1
	//Smaa Vertex shaders input layout
	{
		// Create the vertex input layout.
		result = devclass->GetDevice()->CreateInputLayout(polygonLayout, numElements, vertexShaderBufferSMAAE->GetBufferPointer(), vertexShaderBufferSMAAE->GetBufferSize(),
			&m_layout);
		if (FAILED(result))
		{
			return false;
		}

		// Create the vertex input layout.
		result = devclass->GetDevice()->CreateInputLayout(polygonLayout, numElements, vertexShaderBufferSMAAEC->GetBufferPointer(), vertexShaderBufferSMAAEC->GetBufferSize(),
			&m_layout);
		if (FAILED(result))
		{
			return false;
		}

		// Create the vertex input layout.
		result = devclass->GetDevice()->CreateInputLayout(polygonLayout, numElements, vertexShaderBufferSMAAED->GetBufferPointer(), vertexShaderBufferSMAAED->GetBufferSize(),
			&m_layout);
		if (FAILED(result))
		{
			return false;
		}
	}

#endif
	// Release the vertex shader buffer and pixel shader buffer since they are no longer needed.
	vertexShaderBuffer->Release();
	vertexShaderBuffer = 0;

	pixelShaderBuffer->Release();
	pixelShaderBuffer = 0;

	SafeRelease(pixelShaderBufferBloom);
	SafeRelease(pixelShaderBufferBlurH);
	SafeRelease(pixelShaderBufferBlurV);
	SafeRelease(pixelShaderBufferColor);

#ifdef SMAA_1
	SafeRelease(pixelShaderBufferSMAAE);
	SafeRelease(pixelShaderBufferSMAAEC);
	SafeRelease(pixelShaderBufferSMAAED);

	SafeRelease(vertexShaderBufferSMAAE);
	SafeRelease(vertexShaderBufferSMAAEC);
	SafeRelease(vertexShaderBufferSMAAED);
#endif

	// Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	result = devclass->GetDevice()->CreateBuffer(&matrixBufferDesc, NULL, &m_matrixBuffer);
	if (FAILED(result))
	{
		return false;
	}
	
	// Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
	postProcessBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	postProcessBufferDesc.ByteWidth = sizeof(PostProcessData) *4;
	postProcessBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	postProcessBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	postProcessBufferDesc.MiscFlags = 0;
	postProcessBufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	result = devclass->GetDevice()->CreateBuffer(&postProcessBufferDesc, NULL, &m_PostProcess);
	if (FAILED(result))
	{
		return false;
	}

	// Create a texture sampler state description.
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	// Create the texture sampler state.
	result = devclass->GetDevice()->CreateSamplerState(&samplerDesc, &m_sampleState);
	if (FAILED(result))
	{
		return false;
	}

	return true;
}


void CTextureRenderShader::ShutdownShader()
{
	
	SafeRelease(m_sampleState);
	SafeRelease(m_matrixBuffer);
	SafeRelease(m_layout);
	SafeRelease(m_pixelShader);
	SafeRelease(m_vertexShader);
	SafeRelease(m_pixelShaderBloom);
	SafeRelease(m_pixelShaderBlurV);
	SafeRelease(m_pixelShaderBlurH);

	SafeRelease(m_SMAAColorEdgePS);
	SafeRelease(m_SMAAColorEdgePS);
	SafeRelease(m_SMAALumaEdgePS);
	SafeRelease(m_SMAALumaEdgeVS);
	SafeRelease(m_SMAADepthEdgePS);
	SafeRelease(m_SMAADepthEdgeVS);

	return;
}


void CTextureRenderShader::OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, WCHAR* shaderFilename)
{
	char* compileErrors;
	size_t bufferSize, i;
	ofstream fout;


	// Get a pointer to the error message text buffer.
	compileErrors = (char*)(errorMessage->GetBufferPointer());

	// Get the length of the message.
	bufferSize = errorMessage->GetBufferSize();

	// Open a file to write the error message to.
	fout.open("shader-error.txt");

	// Write out the error message.
	for (i = 0; i<bufferSize; i++)
	{
		fout << compileErrors[i];
	}

	// Close the file.
	fout.close();

	// Release the error message.
	errorMessage->Release();
	errorMessage = 0;

	// Pop a message up on the screen to notify the user to check the text file for compile errors.
	MessageBox(hwnd, L"Error compiling shader.  Check shader-error.txt for message.", shaderFilename, MB_OK);

	return;
}


bool CTextureRenderShader::SetShaderParameters(ID3D11DeviceContext* deviceContext, XMMATRIX& worldMatrix, XMMATRIX& viewMatrix,
	XMMATRIX& projectionMatrix, ID3D11ShaderResourceView* texture, XMFLOAT2 swh, std::vector<XMVECTOR> ssaoSampl)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* dataPtr; PostProcessData* dataPtr1;
	unsigned int bufferNumber;


	// Transpose the matrices to prepare them for the shader.
	worldMatrix = XMMatrixTranspose(worldMatrix);
	viewMatrix = XMMatrixTranspose(viewMatrix);
	projectionMatrix = XMMatrixTranspose(projectionMatrix);
	XMMATRIX projVV = XMMatrixTranspose(devclass->GetProjectionMatrix());

	// Lock the constant buffer so it can be written to.
	result = deviceContext->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
	{
		return false;
	}

	// Get a pointer to the data in the constant buffer.
	dataPtr = (MatrixBufferType*)mappedResource.pData;

	// Copy the matrices into the constant buffer.
	dataPtr->world = worldMatrix;
	dataPtr->view = viewMatrix;
	dataPtr->projection = projectionMatrix;
	dataPtr->projV = projVV;
	// Unlock the constant buffer.
	deviceContext->Unmap(m_matrixBuffer, 0);

	// Set the position of the constant buffer in the vertex shader.
	bufferNumber = 0;

	// Now set the constant buffer in the vertex shader with the updated values.
	deviceContext->VSSetConstantBuffers(bufferNumber, 1, &m_matrixBuffer);


	result = deviceContext->Map(m_PostProcess, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
	{
		return false;
	}

	// Get a pointer to the data in the constant buffer.
	dataPtr1 = (PostProcessData*)mappedResource.pData;

	// Copy the matrices into the constant buffer.
	dataPtr1->expa = Exposure;
	dataPtr1->screenWH = swh;

	
	for (size_t i = 0; i < ssaoSampl.size(); i++)
	{
		dataPtr1->ssaoSampl[i] = ssaoSampl[i];
	}

	// Unlock the constant buffer.
	deviceContext->Unmap(m_PostProcess, 0);

	// Set the position of the constant buffer in the vertex shader.
	bufferNumber = 0;

	// Now set the constant buffer in the vertex shader with the updated values.
	deviceContext->PSSetConstantBuffers(bufferNumber, 1, &m_PostProcess);


	// Set shader texture resource in the pixel shader.
	deviceContext->PSSetShaderResources(0, 1, &texture);

	return true;
}


void CTextureRenderShader::RenderShader(ID3D11DeviceContext* deviceContext, int indexCount)
{
	// Set the vertex input layout.
	deviceContext->IASetInputLayout(m_layout);

	// Set the vertex and pixel shaders that will be used to render this triangle.
	deviceContext->VSSetShader(m_vertexShader, NULL, 0);
	deviceContext->PSSetShader(m_pixelShader, NULL, 0);

	// Set the sampler state in the pixel shader.
	deviceContext->PSSetSamplers(0, 1, &m_sampleState);

	// Render the triangle.
	deviceContext->DrawIndexed(indexCount, 0, 0);

	return;
}


void CTextureRenderShader::RenderShaderBlurV(ID3D11DeviceContext* deviceContext, int indexCount)
{
	// Set the vertex input layout.
	deviceContext->IASetInputLayout(m_layout);

	// Set the vertex and pixel shaders that will be used to render this triangle.
	deviceContext->VSSetShader(m_vertexShader, NULL, 0);
	deviceContext->PSSetShader(m_pixelShaderBlurV, NULL, 0);

	// Set the sampler state in the pixel shader.
	deviceContext->PSSetSamplers(0, 1, &m_sampleState);

	// Render the triangle.
	deviceContext->DrawIndexed(indexCount, 0, 0);

	return;
}

void CTextureRenderShader::RenderShaderBlurH(ID3D11DeviceContext* deviceContext, int indexCount)
{
	// Set the vertex input layout.
	deviceContext->IASetInputLayout(m_layout);

	// Set the vertex and pixel shaders that will be used to render this triangle.
	deviceContext->VSSetShader(m_vertexShader, NULL, 0);
	deviceContext->PSSetShader(m_pixelShaderBlurH, NULL, 0);

	// Set the sampler state in the pixel shader.
	deviceContext->PSSetSamplers(0, 1, &m_sampleState);

	// Render the triangle.
	deviceContext->DrawIndexed(indexCount, 0, 0);

	return;
}

void CTextureRenderShader::RenderShaderBloom(ID3D11DeviceContext* deviceContext, int indexCount)
{
	// Set the vertex input layout.
	deviceContext->IASetInputLayout(m_layout);

	// Set the vertex and pixel shaders that will be used to render this triangle.
	deviceContext->VSSetShader(m_vertexShader, NULL, 0);
	deviceContext->PSSetShader(m_pixelShaderBloom, NULL, 0);

	// Set the sampler state in the pixel shader.
	deviceContext->PSSetSamplers(0, 1, &m_sampleState);

	// Render the triangle.
	deviceContext->DrawIndexed(indexCount, 0, 0);

	return;
}

void CTextureRenderShader::RenderShaderCombine(ID3D11DeviceContext* deviceContext, int indexCount)
{
	// Set the vertex input layout.
	deviceContext->IASetInputLayout(m_layout);

	// Set the vertex and pixel shaders that will be used to render this triangle.
	deviceContext->VSSetShader(m_vertexShader, NULL, 0);
	deviceContext->PSSetShader(m_pixelShader, NULL, 0);

	// Set the sampler state in the pixel shader.
	deviceContext->PSSetSamplers(0, 1, &m_sampleState);

	// Render the triangle.
	deviceContext->DrawIndexed(indexCount, 0, 0);

	return;
}

void CTextureRenderShader::RenderShaderColor(ID3D11DeviceContext* deviceContext, int indexCount)
{
	// Set the vertex input layout.
	deviceContext->IASetInputLayout(m_layout);

	// Set the vertex and pixel shaders that will be used to render this triangle.
	deviceContext->VSSetShader(m_vertexShader, NULL, 0);
	deviceContext->PSSetShader(m_pixelShaderColor, NULL, 0);

	// Set the sampler state in the pixel shader.
	deviceContext->PSSetSamplers(0, 1, &m_sampleState);

	// Render the triangle.
	deviceContext->DrawIndexed(indexCount, 0, 0);

	return;
}



void CTextureRenderShader::PPSSAO(ID3D11DeviceContext* deviceContext, int indexCount)
{
	// Set the vertex input layout.
	deviceContext->IASetInputLayout(m_layout);

	// Set the vertex and pixel shaders that will be used to render this triangle.
	deviceContext->VSSetShader(m_vertexShader, NULL, 0);
	deviceContext->PSSetShader(m_pixelShaderSSAO, NULL, 0);

	// Set the sampler state in the pixel shader.
	deviceContext->PSSetSamplers(0, 1, &m_sampleState);

	// Render the triangle.
	deviceContext->DrawIndexed(indexCount, 0, 0);

}
