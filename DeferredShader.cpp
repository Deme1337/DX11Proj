#include "stdafx.h"
#include "DeferredShader.h"


DeferredShader::DeferredShader()
{
}


DeferredShader::~DeferredShader()
{
}



bool DeferredShader::Initialize(CDeviceClass * devclass, WCHAR* vsFilename, WCHAR* psFilename)
{
	HRESULT result;
	ID3D10Blob* errorMessage;
	ID3D10Blob* vertexShaderBuffer;
	ID3D10Blob* pixelShaderBuffer;
	D3D11_INPUT_ELEMENT_DESC polygonLayout[5];
	unsigned int numElements;
	D3D11_SAMPLER_DESC samplerDesc;
	D3D11_BUFFER_DESC matrixBufferDesc;
	D3D11_BUFFER_DESC objDataDesc;

	// Initialize the pointers this function will use to null.
	errorMessage = 0;
	vertexShaderBuffer = 0;
	pixelShaderBuffer = 0;

	// Compile the vertex shader code.
	result = D3DCompileFromFile(vsFilename, NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, "DeferredVertexShader", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0,
		&vertexShaderBuffer, &errorMessage);
	if (FAILED(result))
	{
		// If the shader failed to compile it should have writen something to the error message.
		if (errorMessage)
		{
			OutputShaderErrorMessage(errorMessage, NULL, vsFilename);
		}
		// If there was nothing in the error message then it simply could not find the shader file itself.
		else
		{
			MessageBox(NULL, vsFilename, L"Missing Shader File", MB_OK);
		}

		return false;
	}

	// Compile the pixel shader code.
	result = D3DCompileFromFile(psFilename, NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, "DeferredPixelShader", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0,
		&pixelShaderBuffer, &errorMessage);
	if (FAILED(result))
	{
		// If the shader failed to compile it should have writen something to the error message.
		if (errorMessage)
		{
			OutputShaderErrorMessage(errorMessage, NULL, psFilename);
		}
		// If there was nothing in the error message then it simply could not find the file itself.
		else
		{
			MessageBox(NULL, psFilename, L"Missing Shader File", MB_OK);
		}

		return false;
	}

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

	polygonLayout[2].SemanticName = "NORMAL";
	polygonLayout[2].SemanticIndex = 0;
	polygonLayout[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	polygonLayout[2].InputSlot = 0;
	polygonLayout[2].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[2].InstanceDataStepRate = 0;

	polygonLayout[3].SemanticName = "TANGENT";
	polygonLayout[3].SemanticIndex = 0;
	polygonLayout[3].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	polygonLayout[3].InputSlot = 0;
	polygonLayout[3].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[3].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[3].InstanceDataStepRate = 0;

	polygonLayout[4].SemanticName = "BINORMAL";
	polygonLayout[4].SemanticIndex = 0;
	polygonLayout[4].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	polygonLayout[4].InputSlot = 0;
	polygonLayout[4].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[4].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[4].InstanceDataStepRate = 0;

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

	// Create a wrap texture sampler state description.
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
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
	result = devclass->GetDevice()->CreateSamplerState(&samplerDesc, &m_sampleStateWrap);
	if (FAILED(result))
	{
		return false;
	}

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

	//Set obj data desc
	objDataDesc.Usage = D3D11_USAGE_DYNAMIC;
	objDataDesc.ByteWidth = sizeof(ObjectData)*4;
	objDataDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	objDataDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	objDataDesc.MiscFlags = 0;
	objDataDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	result = devclass->GetDevice()->CreateBuffer(&objDataDesc, NULL, &m_ObjDataBuffer);
	if (FAILED(result))
	{
		return false;
	}

	return true;
}

void DeferredShader::UpdateTexture(CDeviceClass * devclass, ID3D11ShaderResourceView * texture)
{
	// Set shader texture resource in the pixel shader.
	devclass->GetDevCon()->PSSetShaderResources(0, 1, &texture);
}

void DeferredShader::UpdateTextureBump(CDeviceClass * devclass, ID3D11ShaderResourceView * texture)
{
	// Set shader texture resource in the pixel shader.
	devclass->GetDevCon()->PSSetShaderResources(1, 1, &texture);
}

void DeferredShader::UpdateTextureSpecular(CDeviceClass * devclass, ID3D11ShaderResourceView * texture)
{
	// Set shader texture resource in the pixel shader.
	devclass->GetDevCon()->PSSetShaderResources(2, 1, &texture);
}

void DeferredShader::UpdateTextureRough(CDeviceClass * devclass, ID3D11ShaderResourceView * texture)
{
	// Set shader texture resource in the pixel shader.
	devclass->GetDevCon()->PSSetShaderResources(3, 1, &texture);
}

void DeferredShader::UpdateShader(CDeviceClass * devclass,XMMATRIX & world, XMMATRIX & view, XMMATRIX & projection, bool HasAlpha, XMFLOAT2 texOffSets)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	unsigned int bufferNumber;
	MatrixBufferType* dataPtr;


	// Transpose the matrices to prepare them for the shader.
	world = XMMatrixTranspose(world);
	view= XMMatrixTranspose(view);
	projection = XMMatrixTranspose(projection);

	// Lock the constant buffer so it can be written to.
	result = devclass->GetDevCon()->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
	{
		return;
	}

	// Get a pointer to the data in the constant buffer.
	dataPtr = (MatrixBufferType*)mappedResource.pData;

	// Copy the matrices into the constant buffer.
	dataPtr->world = world;
	dataPtr->view = XMMatrixMultiply(projection, view);
	dataPtr->projection = projection;
	dataPtr->HasAlpha = HasAlpha;
	dataPtr->texOffSetX = texOffSets.x;
	dataPtr->texOffSetY = texOffSets.y;
	//dataPtr->projection = projection;

	// Unlock the constant buffer.
	devclass->GetDevCon()->Unmap(m_matrixBuffer, 0);

	// Set the position of the constant buffer in the vertex shader.
	bufferNumber = 0;

	// Now set the constant buffer in the vertex shader with the updated values.
	devclass->GetDevCon()->VSSetConstantBuffers(bufferNumber, 1, &m_matrixBuffer);

}

void DeferredShader::SetObjectData(CDeviceClass * devclass, XMFLOAT4 data, XMFLOAT4 objcolor)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	unsigned int bufferNumber;
	ObjectData* dataPtr;

	

	result = devclass->GetDevCon()->Map(m_ObjDataBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
	{
		return;
	}

	dataPtr = (ObjectData*)mappedResource.pData;

	dataPtr->UseTextures = data.x;
	dataPtr->roughnessOffset = data.y;
	dataPtr->metallic = data.z;
	dataPtr->objColor = objcolor;
	dataPtr->IsPaper = ObjectIs2DAnimated;

	devclass->GetDevCon()->Unmap(m_ObjDataBuffer, 0);

	bufferNumber = 0;

	
	devclass->GetDevCon()->PSSetConstantBuffers(bufferNumber, 1, &m_ObjDataBuffer);

	dataPtr = 0;
	delete dataPtr;
}

void DeferredShader::RenderShader(CDeviceClass * devclass, int indexCount)
{
	// Set the vertex input layout.
	devclass->GetDevCon()->IASetInputLayout(m_layout);

	// Set the vertex and pixel shaders that will be used to render.
	devclass->GetDevCon()->VSSetShader(m_vertexShader, NULL, 0);
	devclass->GetDevCon()->PSSetShader(m_pixelShader, NULL, 0);

	// Set the sampler states in the pixel shader.
	devclass->GetDevCon()->PSSetSamplers(0, 1, &m_sampleStateWrap);

	// Render the geometry.
	devclass->GetDevCon()->DrawIndexed(indexCount, 0, 0);

}

void DeferredShader::Release()
{
	SafeRelease(m_ObjDataBuffer);

	if (m_matrixBuffer)
	{
		m_matrixBuffer->Release();
		m_matrixBuffer = 0;
	}

	// Release the sampler state.
	if (m_sampleStateWrap)
	{
		m_sampleStateWrap->Release();
		m_sampleStateWrap = 0;
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

void DeferredShader::OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, WCHAR* shaderFilename)
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
	MessageBox(hwnd, L"Error compiling shader.  Check shader-error.txt for message.", shaderFilename, MB_OK);
}
