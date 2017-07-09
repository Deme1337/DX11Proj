#include "stdafx.h"
#include "LightShader.h"


LightShader::LightShader()
{
}


LightShader::~LightShader()
{
}

bool LightShader::Initialize(CDeviceClass *devclass, WCHAR* vsFilename, WCHAR* psFilename)
{
	HRESULT result;
	ID3D10Blob* errorMessage;
	ID3D10Blob* vertexShaderBuffer;
	ID3D10Blob* pixelShaderBuffer;
	D3D11_INPUT_ELEMENT_DESC polygonLayout[2];
	unsigned int numElements;
	D3D11_SAMPLER_DESC samplerDesc;
	D3D11_BUFFER_DESC matrixBufferDesc;
	D3D11_BUFFER_DESC lightBufferDesc;
	D3D11_BUFFER_DESC pointLightBufferDesc;
	D3D11_BUFFER_DESC cameraBufferDesc;
	D3D11_BUFFER_DESC disneyBufferDesc;

	// Initialize the pointers this function will use to null.
	errorMessage = 0;
	vertexShaderBuffer = 0;
	pixelShaderBuffer = 0;


	vertexShaderBuffer = CDeviceClass::CompileShader(vsFilename, VertexShader, "LightVertexShader");
	pixelShaderBuffer = CDeviceClass::CompileShader(psFilename, PixelShader, "LightPixelShader");

	// Create the vertex shader from the buffer.
	result = devclass->GetDevice()->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL, &m_vertexShader);
	if (FAILED(result))
	{
		return false;
	}

	// Create the pixel shader from the buffer.


	result = devclass->GetDevice()->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, &m_pixelShader);
	if (FAILED(result))
	{
		return false;
	}

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

	// Release the vertex shader buffer and pixel shader buffer since they are no longer needed.
	vertexShaderBuffer->Release();
	vertexShaderBuffer = 0;

	pixelShaderBuffer->Release();
	pixelShaderBuffer = 0;

	// Create a texture sampler state description.
	//Linear works with nvidia gpus not amd so i heard: gdc 2011
	//samplerDesc.Filter =  D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	//samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
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



	// Create a clamp texture sampler state description.
	samplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_MIRROR;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_MIRROR;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;

	// Create the texture sampler state.
	result = devclass->GetDevice()->CreateSamplerState(&samplerDesc, &m_sampleClamp);
	if (FAILED(result))
	{
		return false;
	}


	// Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBuffer);
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

	// Setup the description of the light dynamic constant buffer that is in the pixel shader.
	lightBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	lightBufferDesc.ByteWidth = sizeof(LightBuffer);
	lightBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	lightBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	lightBufferDesc.MiscFlags = 0;
	lightBufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the pixel shader constant buffer from within this class.
	result = devclass->GetDevice()->CreateBuffer(&lightBufferDesc, NULL, &m_lightBuffer);
	if (FAILED(result))
	{
		return false;
	}


	//point light buffer desc
	pointLightBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	pointLightBufferDesc.ByteWidth = sizeof(PointLightBuffer);
	pointLightBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	pointLightBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	pointLightBufferDesc.MiscFlags = 0;
	pointLightBufferDesc.StructureByteStride = 0;

	result = devclass->GetDevice()->CreateBuffer(&pointLightBufferDesc, NULL, &m_PointLightBuffer);
	if (FAILED(result))
	{
		return false;
	}




	// Setup the description of the light dynamic constant buffer that is in the pixel shader.
	cameraBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	cameraBufferDesc.ByteWidth = sizeof(CameraBuffer);
	cameraBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cameraBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cameraBufferDesc.MiscFlags = 0;
	cameraBufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the pixel shader constant buffer from within this class.
	result = devclass->GetDevice()->CreateBuffer(&cameraBufferDesc, NULL, &m_cameraBuffer);
	if (FAILED(result))
	{
		return false;
	}


	disneyBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	disneyBufferDesc.ByteWidth = sizeof(DisneyParam);
	disneyBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	disneyBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	disneyBufferDesc.MiscFlags = 0;
	disneyBufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the pixel shader constant buffer from within this class.
	result = devclass->GetDevice()->CreateBuffer(&disneyBufferDesc, NULL, &m_disneyBuffer);
	if (FAILED(result))
	{
		return false;
	}
	return true;
}



void LightShader::UpdateDisneyBuffer(CDeviceClass * devclass, XMFLOAT4 f1, XMFLOAT4 f2)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	unsigned int bufferNumber;
	DisneyParam* dataPtr;

	result = devclass->GetDevCon()->Map(m_disneyBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	// Get a pointer to the data in the constant buffer.
	dataPtr = (DisneyParam*)mappedResource.pData;
	dataPtr->sheentintcleargloss = f2;
	dataPtr->subspectintani = f1;


	// Unlock the constant buffer.
	devclass->GetDevCon()->Unmap(m_disneyBuffer, 0);

	// Set the position of the light constant buffer in the pixel shader.
	bufferNumber = 2;

	// Finally set the light constant buffer in the pixel shader with the updated values.
	devclass->GetDevCon()->PSSetConstantBuffers(bufferNumber, 1, &m_disneyBuffer);
}

void LightShader::UpdateCameraPosition(CDeviceClass * devclass, XMVECTOR cp)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	unsigned int bufferNumber;
	CameraBuffer* dataPtr;

	result = devclass->GetDevCon()->Map(m_cameraBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	// Get a pointer to the data in the constant buffer.
	dataPtr = (CameraBuffer*)mappedResource.pData;
	dataPtr->CameraPosition = cp;


	// Unlock the constant buffer.
	devclass->GetDevCon()->Unmap(m_cameraBuffer, 0);

	// Set the position of the light constant buffer in the pixel shader.
	bufferNumber = 1;

	// Finally set the light constant buffer in the pixel shader with the updated values.
	devclass->GetDevCon()->VSSetConstantBuffers(bufferNumber, 1, &m_cameraBuffer);
}

void LightShader::UpdateTextureByIndex(CDeviceClass * devclass, ID3D11ShaderResourceView * tex, int index)
{
	devclass->GetDevCon()->PSSetShaderResources(index, 1, &tex);
}

void LightShader::UpdateShadowMap(CDeviceClass * devclass, ID3D11ShaderResourceView* shadowmap)
{
	devclass->GetDevCon()->PSSetShaderResources(7, 1, &shadowmap);
}

void LightShader::UpdateShaderParameters(CDeviceClass * devclass, XMMATRIX & worldMatrix, XMMATRIX & viewMatrix, XMMATRIX & projectionMatrix,
	ID3D11ShaderResourceView * colorTexture, ID3D11ShaderResourceView * normalTexture,
	ID3D11ShaderResourceView* specularTexture,  ID3D11ShaderResourceView* positionTexture, 
	ID3D11ShaderResourceView* roughnessTexture, ID3D11ShaderResourceView* tangentTexture, ID3D11ShaderResourceView* binormalTexture, DirectionalLight dlight, std::vector<PointLight> plights)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	unsigned int bufferNumber;
	MatrixBuffer* dataPtr;
	LightBuffer* dataPtr2;
	PointLightBuffer* dataPtr3;

	// Transpose the matrices to prepare them for the shader.
	worldMatrix = XMMatrixTranspose(worldMatrix);
	viewMatrix = XMMatrixTranspose(viewMatrix);
	projectionMatrix = XMMatrixTranspose(projectionMatrix);
	


	// Lock the constant buffer so it can be written to.
	result = devclass->GetDevCon()->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
	{
		return;
	}

	// Get a pointer to the data in the constant buffer.
	dataPtr = (MatrixBuffer*)mappedResource.pData;

	// Copy the matrices into the constant buffer.
	dataPtr->world = worldMatrix;
	dataPtr->view = viewMatrix;
	dataPtr->projection = projectionMatrix;

	// Unlock the constant buffer.
	devclass->GetDevCon()->Unmap(m_matrixBuffer, 0);

	// Set the position of the constant buffer in the vertex shader.
	bufferNumber = 0;

	// Now set the constant buffer in the vertex shader with the updated values.
	devclass->GetDevCon()->VSSetConstantBuffers(bufferNumber, 1, &m_matrixBuffer);

	// Set shader texture resources in the pixel shader.
	devclass->GetDevCon()->PSSetShaderResources(0, 1, &colorTexture);
	devclass->GetDevCon()->PSSetShaderResources(1, 1, &normalTexture);
	devclass->GetDevCon()->PSSetShaderResources(2, 1, &specularTexture);
	devclass->GetDevCon()->PSSetShaderResources(3, 1, &positionTexture);
	devclass->GetDevCon()->PSSetShaderResources(4, 1, &roughnessTexture);
	devclass->GetDevCon()->PSSetShaderResources(5, 1, &tangentTexture);
	devclass->GetDevCon()->PSSetShaderResources(6, 1, &binormalTexture);

	// Lock the light constant buffer so it can be written to.
	result = devclass->GetDevCon()->Map(m_lightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
	{
		return;
	}

	// Get a pointer to the data in the constant buffer.
	dataPtr2 = (LightBuffer*)mappedResource.pData;

	// Copy the lighting variables into the constant buffer.
	XMVECTOR pos = XMLoadFloat4(&dlight.lightProperties.Position);
	XMVECTOR col = XMLoadFloat4(&dlight.lightProperties.Color);
	dataPtr2->LightPosition = pos;
	dataPtr2->LightColor	= col;
	dataPtr2->lightViewMatrix = XMMatrixTranspose(dlight.GetLightViewMatrix());
	dataPtr2->lightProjectionMatrix = XMMatrixTranspose(dlight.GetLightProjectionMatrix());
	dataPtr2->GlobalAmbient = dlight.lightProperties.globalAmbient;

	// Unlock the constant buffer.
	devclass->GetDevCon()->Unmap(m_lightBuffer, 0);

	// Set the position of the light constant buffer in the pixel shader.
	bufferNumber = 0;

	// Finally set the light constant buffer in the pixel shader with the updated values.
	devclass->GetDevCon()->PSSetConstantBuffers(bufferNumber, 1, &m_lightBuffer);


	
	//Lock pointlight constantbuffer
	result = devclass->GetDevCon()->Map(m_PointLightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
	{
		return;
	}

	// Get a pointer to the data in the constant buffer.
	dataPtr3 = (PointLightBuffer*)mappedResource.pData;

	// Copy the lighting variables into the constant buffer.
	for (size_t i = 0; i < POINT_LIGHTS; i++)
	{
		dataPtr3->PointLightColor[i] = XMLoadFloat4(&plights[i].lightProperties.Color);
		dataPtr3->PointLightPosition[i] = XMLoadFloat4(&plights[i].lightProperties.Position);
	}


	// Unlock the constant buffer.
	devclass->GetDevCon()->Unmap(m_PointLightBuffer, 0);

	// Set the position of the light constant buffer in the pixel shader.
	bufferNumber = 1;

	// Finally set the light constant buffer in the pixel shader with the updated values.
	devclass->GetDevCon()->PSSetConstantBuffers(bufferNumber, 1, &m_PointLightBuffer);
	
}

void LightShader::UpdateShaderParameters(CDeviceClass * devclass, XMMATRIX & worldMatrix, XMMATRIX & viewMatrix, XMMATRIX & projectionMatrix, DeferredBuffersClass*  defBuf, DirectionalLight dlight, std::vector<PointLight> plights)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	unsigned int bufferNumber;
	MatrixBuffer* dataPtr;
	LightBuffer* dataPtr2;
	PointLightBuffer* dataPtr3;

	// Transpose the matrices to prepare them for the shader.
	worldMatrix = XMMatrixTranspose(worldMatrix);
	viewMatrix = XMMatrixTranspose(viewMatrix);
	projectionMatrix = XMMatrixTranspose(projectionMatrix);



	// Lock the constant buffer so it can be written to.
	result = devclass->GetDevCon()->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
	{
		return;
	}

	// Get a pointer to the data in the constant buffer.
	dataPtr = (MatrixBuffer*)mappedResource.pData;

	// Copy the matrices into the constant buffer.
	dataPtr->world = worldMatrix;
	dataPtr->view = viewMatrix;
	dataPtr->projection = projectionMatrix;

	// Unlock the constant buffer.
	devclass->GetDevCon()->Unmap(m_matrixBuffer, 0);

	// Set the position of the constant buffer in the vertex shader.
	bufferNumber = 0;

	// Now set the constant buffer in the vertex shader with the updated values.
	devclass->GetDevCon()->VSSetConstantBuffers(bufferNumber, 1, &m_matrixBuffer);

	ID3D11ShaderResourceView* tempCol   = defBuf->GetShaderResourceView(0);
	ID3D11ShaderResourceView* tempNorm  = defBuf->GetShaderResourceView(1);
	ID3D11ShaderResourceView* tempSpec  = defBuf->GetShaderResourceView(2);
	ID3D11ShaderResourceView* tempPos   = defBuf->GetShaderResourceView(3);
	ID3D11ShaderResourceView* tempRough = defBuf->GetShaderResourceView(4);

	// Set shader texture resources in the pixel shader.
	devclass->GetDevCon()->PSSetShaderResources(0, 1, &tempCol);
	devclass->GetDevCon()->PSSetShaderResources(1, 1, &tempNorm);
	devclass->GetDevCon()->PSSetShaderResources(2, 1, &tempSpec);
	devclass->GetDevCon()->PSSetShaderResources(3, 1, &tempPos);
	devclass->GetDevCon()->PSSetShaderResources(4, 1, &tempRough);

	SafeRelease(tempCol);
	SafeRelease(tempNorm);
	SafeRelease(tempSpec);
	SafeRelease(tempPos);
	SafeRelease(tempRough);
	// Lock the light constant buffer so it can be written to.
	result = devclass->GetDevCon()->Map(m_lightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
	{
		return;
	}

	// Get a pointer to the data in the constant buffer.
	dataPtr2 = (LightBuffer*)mappedResource.pData;

	// Copy the lighting variables into the constant buffer.
	XMVECTOR pos = XMLoadFloat4(&dlight.lightProperties.Position);
	XMVECTOR col = XMLoadFloat4(&dlight.lightProperties.Color);
	dataPtr2->LightPosition = -XMVector4Normalize(pos);
	dataPtr2->LightColor = col;
	dataPtr2->lightViewMatrix = XMMatrixTranspose(dlight.GetLightViewMatrix());
	dataPtr2->lightProjectionMatrix = XMMatrixTranspose(dlight.GetLightProjectionMatrix());
	dataPtr2->GlobalAmbient = dlight.lightProperties.globalAmbient;

	// Unlock the constant buffer.
	devclass->GetDevCon()->Unmap(m_lightBuffer, 0);

	// Set the position of the light constant buffer in the pixel shader.
	bufferNumber = 0;

	// Finally set the light constant buffer in the pixel shader with the updated values.
	devclass->GetDevCon()->PSSetConstantBuffers(bufferNumber, 1, &m_lightBuffer);



	//Lock pointlight constantbuffer
	result = devclass->GetDevCon()->Map(m_PointLightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
	{
		return;
	}

	// Get a pointer to the data in the constant buffer.
	dataPtr3 = (PointLightBuffer*)mappedResource.pData;

	// Copy the lighting variables into the constant buffer.
	for (size_t i = 0; i < POINT_LIGHTS; i++)
	{
		dataPtr3->PointLightColor[i] = XMLoadFloat4(&plights[i].lightProperties.Color);
		dataPtr3->PointLightPosition[i] = XMLoadFloat4(&plights[i].lightProperties.Position);
	}


	// Unlock the constant buffer.
	devclass->GetDevCon()->Unmap(m_PointLightBuffer, 0);

	// Set the position of the light constant buffer in the pixel shader.
	bufferNumber = 1;

	// Finally set the light constant buffer in the pixel shader with the updated values.
	devclass->GetDevCon()->PSSetConstantBuffers(bufferNumber, 1, &m_PointLightBuffer);
}

void LightShader::Release()
{
	// Release the light constant buffer.
	if (m_lightBuffer)
	{
		m_lightBuffer->Release();
		m_lightBuffer = 0;
	}

	// Release the matrix constant buffer.
	if (m_matrixBuffer)
	{
		m_matrixBuffer->Release();
		m_matrixBuffer = 0;
	}

	// Release the sampler state.
	if (m_sampleState)
	{
		m_sampleState->Release();
		m_sampleState = 0;
	}

	// Release the layout.
	if (m_layout)
	{
		m_layout->Release();
		m_layout = 0;
	}

	// Release the pixel shader.
	if (m_pixelShader)
	{
		m_pixelShader->Release();
		m_pixelShader = 0;
	}

	// Release the vertex shader.
	if (m_vertexShader)
	{
		m_vertexShader->Release();
		m_vertexShader = 0;
	}
}

void LightShader::Update(CDeviceClass *devclass, int indexCount)
{
	// Set the vertex input layout.
	devclass->GetDevCon()->IASetInputLayout(m_layout);

	// Set the vertex and pixel shaders that will be used to render.
	devclass->GetDevCon()->VSSetShader(m_vertexShader, NULL, 0);
	devclass->GetDevCon()->PSSetShader(m_pixelShader, NULL, 0);

	// Set the sampler state in the pixel shader.
	devclass->GetDevCon()->PSSetSamplers(0, 1, &m_sampleState);
	devclass->GetDevCon()->PSSetSamplers(1, 1, &m_sampleClamp);

	// Render the geometry.
	devclass->GetDevCon()->DrawIndexed(indexCount, 0, 0);
}

void LightShader::OutputErrorMessage(ID3D10Blob * errorMessage, HWND hWnd, WCHAR * shaderFilename)
{

	char* compileErrors;
	unsigned long bufferSize, i;
	std::ofstream fout;


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
	MessageBox(NULL, L"Error compiling shader.  Check shader-error.txt for message.", shaderFilename, MB_OK);

	return;
}
