#include "stdafx.h"
#include "SkyDomeShader.h"

#include <cstdint>
#include <iostream>
#include <iomanip>
#include <random>
#include <algorithm> // notice this
using namespace std;




float PhysicalSunSize = XMConvertToRadians(0.27f);
float CosPhysicalSunSize = std::cos(PhysicalSunSize);


static float AngleBetween(const XMVECTOR& dir0, const XMVECTOR& dir1)
{
	XMFLOAT3 diRdot;
	XMStoreFloat3(&diRdot, XMVector3Dot(dir0, dir1));
	return acos(max((float)diRdot.x, 0.00001f));
}

// Returns the result of performing a irradiance integral over the portion
// of the hemisphere covered by a region with angular radius = theta
static float IrradianceIntegral(float theta)
{
	float sinTheta = std::sin(theta);
	return Pi * sinTheta * sinTheta;
}

CSkyDomeShader::CSkyDomeShader()
{
	m_vertexShader = 0;
	m_pixelShader = 0;
	m_layout = 0;
	m_matrixBuffer = 0;
	m_colorBuffer = 0;
}


CSkyDomeShader::CSkyDomeShader(const CSkyDomeShader& other)
{
}


CSkyDomeShader::~CSkyDomeShader()
{
}


bool CSkyDomeShader::Initialize(ID3D11Device* device, HWND hwnd)
{
	bool result;


	// Initialize the vertex and pixel shaders.
	result = InitializeShader(device, hwnd, L"SkydomeVS.hlsl", L"SkyDomePS.hlsl");
	if (!result)
	{
		return false;
	}

	return true;
}


void CSkyDomeShader::Shutdown()
{
	// Shutdown the vertex and pixel shaders as well as the related objects.
	ShutdownShader();

	return;
}


bool CSkyDomeShader::Update(ID3D11DeviceContext* deviceContext, int indexCount, XMMATRIX &worldMatrix, XMMATRIX &viewMatrix,
	XMMATRIX &projectionMatrix, XMFLOAT4 &apexColor, XMFLOAT4 &centerColor, DirectionalLight &dlight, FreeCamera* cam)
{
	bool result;


	// Set the shader parameters that it will use for rendering.
	result = SetShaderParameters(deviceContext, worldMatrix, viewMatrix, projectionMatrix, apexColor, centerColor, dlight, cam);
	if (!result)
	{
		return false;
	}

	// Now render the prepared buffers with the shader.
	RenderShader(deviceContext, indexCount);

	return true;
}

void CSkyDomeShader::SetSkyDomeTexture(ID3D11DeviceContext* devcon, ID3D11ShaderResourceView * tex, int index)
{
	devcon->PSSetShaderResources(index, 1, &tex);
}


bool CSkyDomeShader::InitializeShader(ID3D11Device* device, HWND hwnd, WCHAR* vsFilename, WCHAR* psFilename)
{
	HRESULT result;
	ID3D10Blob* errorMessage;
	ID3D10Blob* vertexShaderBuffer;
	ID3D10Blob* pixelShaderBuffer;
	D3D11_INPUT_ELEMENT_DESC polygonLayout[3];
	unsigned int numElements;
	D3D11_BUFFER_DESC matrixBufferDesc;
	D3D11_BUFFER_DESC colorBufferDesc;


	// Initialize the pointers this function will use to null.
	errorMessage = 0;
	vertexShaderBuffer = 0;
	pixelShaderBuffer = 0;

	// Compile the vertex shader code.
	result = D3DCompileFromFile(vsFilename, NULL, NULL, "SkyDomeVertexShader", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0,
		&vertexShaderBuffer, &errorMessage);
	if (FAILED(result))
	{
		// If the shader failed to compile it should have writen something to the error message.
		if (errorMessage)
		{
			OutputShaderErrorMessage(errorMessage, hwnd, vsFilename);
		}
		// If there was nothing in the error message then it simply could not find the shader file itself.
		else
		{
			MessageBox(hwnd, vsFilename, L"Missing Shader File", MB_OK);
		}

		return false;
	}

	// Compile the pixel shader code.
	result = D3DCompileFromFile(psFilename, NULL, NULL, "SkyDomePixelShader", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0,
		&pixelShaderBuffer, &errorMessage);
	if (FAILED(result))
	{
		// If the shader failed to compile it should have writen something to the error message.
		if (errorMessage)
		{
			OutputShaderErrorMessage(errorMessage, hwnd, psFilename);
		}
		// If there was nothing in the error message then it simply could not find the file itself.
		else
		{
			MessageBox(hwnd, psFilename, L"Missing Shader File", MB_OK);
		}

		return false;
	}

	// Create the vertex shader from the buffer.
	result = device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL, &m_vertexShader);
	if (FAILED(result))
	{
		return false;
	}

	// Create the pixel shader from the buffer.
	result = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, &m_pixelShader);
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

	// Get a count of the elements in the layout.
	numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

	// Create the vertex input layout.
	result = device->CreateInputLayout(polygonLayout, numElements, vertexShaderBuffer->GetBufferPointer(),
		vertexShaderBuffer->GetBufferSize(), &m_layout);
	if (FAILED(result))
	{
		return false;
	}

	// Release the vertex shader buffer and pixel shader buffer since they are no longer needed.
	vertexShaderBuffer->Release();
	vertexShaderBuffer = 0;

	pixelShaderBuffer->Release();
	pixelShaderBuffer = 0;

	// Setup the description of the dynamic constant buffer that is in the vertex shader.
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	result = device->CreateBuffer(&matrixBufferDesc, NULL, &m_matrixBuffer);
	if (FAILED(result))
	{
		return false;
	}

	// Setup the description of the dynamic pixel constant buffer that is in the pixel shader.
	colorBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	colorBufferDesc.ByteWidth = sizeof(ColorBufferType);
	colorBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	colorBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	colorBufferDesc.MiscFlags = 0;
	colorBufferDesc.StructureByteStride = 0;

	// Create the pixel constant buffer pointer so we can access the pixel shader constant buffer from within this class.
	result = device->CreateBuffer(&colorBufferDesc, NULL, &m_colorBuffer);
	if (FAILED(result))
	{
		return false;
	}

	return true;
}


void CSkyDomeShader::ShutdownShader()
{
	// Release the pixel constant buffer.
	if (m_colorBuffer)
	{
		m_colorBuffer->Release();
		m_colorBuffer = 0;
	}

	// Release the matrix constant buffer.
	if (m_matrixBuffer)
	{
		m_matrixBuffer->Release();
		m_matrixBuffer = 0;
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

	return;
}


void CSkyDomeShader::OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, WCHAR* shaderFilename)
{
	char* compileErrors;
	unsigned __int64 bufferSize, i;
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


bool CSkyDomeShader::SetShaderParameters(ID3D11DeviceContext* deviceContext, XMMATRIX &worldMatrix, XMMATRIX &viewMatrix,
	XMMATRIX &projectionMatrix, XMFLOAT4 &apexColor, XMFLOAT4 &centerColor, DirectionalLight &dlight, FreeCamera* cam)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* dataPtr;
	unsigned int bufferNumber;
	ColorBufferType* dataPtr2;


	// Transpose the matrices to prepare them for the shader.
	worldMatrix = XMMatrixTranspose(worldMatrix);
	viewMatrix = XMMatrixTranspose(viewMatrix);
	projectionMatrix = XMMatrixTranspose(projectionMatrix);

	// Lock the matrix constant buffer so it can be written to.
	result = deviceContext->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
	{
		return false;
	}

	// Get a pointer to the data in the matrix constant buffer.
	dataPtr = (MatrixBufferType*)mappedResource.pData;

	// Copy the matrices into the matrix constant buffer.
	dataPtr->world = worldMatrix;
	dataPtr->view = viewMatrix;
	dataPtr->projection = projectionMatrix;
	dataPtr->sunPosition = XMLoadFloat4(&dlight.lightProperties.Position);

	// Unlock the matrix constant buffer.
	deviceContext->Unmap(m_matrixBuffer, 0);

	// Set the position of the constant buffer in the vertex shader.
	bufferNumber = 0;

	// Now set the matrix constant buffer in the vertex shader with the updated values.
	deviceContext->VSSetConstantBuffers(bufferNumber, 1, &m_matrixBuffer);

	// Lock the color constant buffer so it can be written to.
	result = deviceContext->Map(m_colorBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
	{
		return false;
	}

	// Get a pointer to the data in the color constant buffer.
	dataPtr2 = (ColorBufferType*)mappedResource.pData;

	// Copy the color data into the color constant buffer.
	dataPtr2->apexColor = apexColor;
	dataPtr2->centerColor = centerColor;
	dataPtr2->sunPosition = XMLoadFloat4(&dlight.lightProperties.Position);
	dataPtr2->cameraPosition = cam->GetCameraPosition();
	dataPtr2->sunSize = dlight.lightProperties.size;
	dataPtr2->sunPower = dlight.lightProperties.sunPower;
	dataPtr2->scale1 = dlight.lightProperties.scale1;
	dataPtr2->scale1 = dlight.lightProperties.scale2;

	dataPtr2->radiance = SampleSky(this->skyModel, XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f)).x;

	// Unlock the color constant buffer.
	deviceContext->Unmap(m_colorBuffer, 0);

	// Set the position of the color constant buffer in the pixel shader.
	bufferNumber = 0;

	// Now set the color constant buffer in the pixel shader with the updated color values.
	deviceContext->PSSetConstantBuffers(bufferNumber, 1, &m_colorBuffer);

	return true;
}


void CSkyDomeShader::RenderShader(ID3D11DeviceContext* deviceContext, int indexCount)
{
	// Set the vertex input layout.
	deviceContext->IASetInputLayout(m_layout);

	// Set the vertex and pixel shaders that will be used to render the triangles.
	deviceContext->VSSetShader(m_vertexShader, NULL, 0);
	deviceContext->PSSetShader(m_pixelShader, NULL, 0);

	// Render the triangles.
	deviceContext->DrawIndexed(indexCount, 0, 0);

	return;
}

XMFLOAT3 CSkyDomeShader::SampleSky(const SkyModelData & data, XMVECTOR sampleDir)
{


	float gamma = AngleBetween(sampleDir, XMLoadFloat3(&data.SunDirection));
	float theta = AngleBetween(sampleDir, XMVectorSet(0, 1, 0, 1));

	XMFLOAT3 radiance;

	radiance.x = float(arhosek_tristim_skymodel_radiance(data.StateR, theta, gamma, 0));
	radiance.y = float(arhosek_tristim_skymodel_radiance(data.StateG, theta, gamma, 1));
	radiance.z = float(arhosek_tristim_skymodel_radiance(data.StateB, theta, gamma, 2));

	// Multiply by standard luminous efficacy of 683 lm/W to bring us in line with the photometric
	// units used during rendering
	radiance.x *= 683.0f;
	radiance.y *= 683.0f;
	radiance.z *= 683.0f;

	radiance.x *= FP16Scale;
	radiance.y *= FP16Scale;
	radiance.z *= FP16Scale;

	return radiance;
}


float clip(float n, float lower, float upper)
{
	return std::max(lower, std::min(n, upper));
}

XMFLOAT3 Saturate(XMFLOAT3 val)
{
	XMFLOAT3 result;
	result.x = clip(val.x, 0.0f, 1.0f);
	result.y = clip(val.y, 0.0f, 1.0f);
	result.z = clip(val.z, 0.0f, 1.0f);
	return result;
}

float Saturate(float val)
{
	return clip(val, 0.0f, 1.0f);
}


void CSkyDomeShader::SkyModelData::Init(XMFLOAT3 sunDirection, XMFLOAT3 groundAlbedo, float turbidity)
{


	sunDirection.y = Saturate(sunDirection.y);
	XMVECTOR sunDirV = XMLoadFloat3(&sunDirection);
	sunDirV = XMVector3Normalize(sunDirV);
    turbidity = clip(turbidity, 1.0f, 32.0f);
    groundAlbedo = Saturate(groundAlbedo);


    float thetaS = AngleBetween(sunDirV, XMVectorSet(0, 1, 0, 1.0f));
    float elevation = Pi_2 - thetaS;
    StateR = arhosek_rgb_skymodelstate_alloc_init(turbidity, groundAlbedo.x, elevation);
    StateG = arhosek_rgb_skymodelstate_alloc_init(turbidity, groundAlbedo.y, elevation);
    StateB = arhosek_rgb_skymodelstate_alloc_init(turbidity, groundAlbedo.z, elevation);

    Albedo = groundAlbedo;
    Elevation = elevation;
    SunDirection = sunDirection;
    Turbidity = turbidity;
}


