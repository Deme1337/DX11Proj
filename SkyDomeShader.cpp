#include "stdafx.h"
#include "SkyDomeShader.h"
#include "Math.h"


#include <cstdint>
#include <iostream>
#include <iomanip>
#include <random>
#include <algorithm> // notice this
using namespace std;

// Utility function to map a XY + Side coordinate to a direction vector
XMVECTOR CSkyDomeShader::MapXYSToDirection(unsigned int x, unsigned int y, unsigned int s, unsigned int width, unsigned int height)
{
	float u = ((x + 0.5f) / float(width)) * 2.0f - 1.0f;
	float v = ((y + 0.5f) / float(height)) * 2.0f - 1.0f;
	v *= -1.0f;

	XMVECTOR dir = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);

	// +x, -x, +y, -y, +z, -z
	switch (s) 
	{
	case 0:
		dir = XMVector3Normalize(XMVectorSet(1.0f, v, -u, 1.0f));
		break;
	case 1:
		dir = XMVector3Normalize(XMVectorSet(-1.0f, v, u, 1.0f));
		break;
	case 2:
		dir = XMVector3Normalize(XMVectorSet(u, 1.0f, -v, 1.0f));
		break;
	case 3:
		dir = XMVector3Normalize(XMVectorSet(u, -1.0f, v, 1.0f));
		break;
	case 4:
		dir = XMVector3Normalize(XMVectorSet(u, v, 1.0f, 1.0f));
		break;
	case 5:
		dir = XMVector3Normalize(XMVectorSet(-u, v, -1.0f, 1.0f));
		break;
	}

	return dir;
}



float PhysicalSunSize = XMConvertToRadians(0.27f);
float CosPhysicalSunSize = std::cos(PhysicalSunSize);


float AngleBetween(const XMVECTOR& dir0, const XMVECTOR& dir1)
{
	return	std::acos(std::max(XMVectorGetX(XMVector3Dot(dir0, dir1)), 0.00001f));
}

// Returns the result of performing a irradiance integral over the portion
// of the hemisphere covered by a region with angular radius = theta
float IrradianceIntegral(float theta)
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


bool CSkyDomeShader::Update(ID3D11DeviceContext* deviceContext, ID3D11Device* dev, int indexCount, XMMATRIX &worldMatrix, XMMATRIX &viewMatrix,
	XMMATRIX &projectionMatrix, XMFLOAT4 &apexColor, XMFLOAT4 &centerColor, DirectionalLight &dlight, FreeCamera* cam)
{
	bool result;


	// Set the shader parameters that it will use for rendering.
	result = SetShaderParameters(deviceContext, dev,  worldMatrix, viewMatrix, projectionMatrix, apexColor, centerColor, dlight, cam);
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
	skyModel.ShutDown();
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


bool CSkyDomeShader::SetShaderParameters(ID3D11DeviceContext* deviceContext, ID3D11Device* dev, XMMATRIX &worldMatrix, XMMATRIX &viewMatrix,
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

	XMFLOAT3 sunDirection; XMStoreFloat3(&sunDirection, XMVector3Normalize(XMLoadFloat4(&dlight.lightProperties.Position)));



	//Create skycache
	//if (sunPosMoved)
	//{
	//	skyModel.ShutDown();
	//	skyModel.Init(sunDirection, XMFLOAT3(0.5f, 0.5f, 0.5f), 8.0f);
	//	unsigned int CubeMapRes = 128;
	//	std::vector<XMFLOAT4> texels;
	//	texels.resize(CubeMapRes * CubeMapRes * 6);
	//
	//	for (int i = 0; i < 6; ++i)
	//	{
	//		for (int x = 0; x < CubeMapRes; ++x)
	//		{
	//			for (int y = 0; y < CubeMapRes; ++y)
	//			{
	//				XMFLOAT3 radiance = SampleSky(this->skyModel, this->MapXYSToDirection(x, y, i, CubeMapRes, CubeMapRes));
	//
	//				unsigned int idx = (i * CubeMapRes * CubeMapRes) + (y * CubeMapRes) + x;
	//
	//
	//				texels[idx].x = radiance.x;
	//				texels[idx].y = radiance.y;
	//				texels[idx].z = radiance.z;
	//				texels[idx].w = 1.0f;
	//			}
	//		}
	//	}
	//
	//	std::vector<std::string> cmdata;
	//	//DEBUG
	//	for (size_t i = 0; i < texels.size(); i++)
	//	{
	//		std::string d = std::to_string(texels[i].x) + "  " + std::to_string(texels[i].y) + "  " + std::to_string(texels[i].z);
	//		cmdata.push_back(d);
	//	}
	//
	//	writeToFile(cmdata);
	//
	//	D3D11_TEXTURE2D_DESC desc;
	//	desc.Width = CubeMapRes;
	//	desc.Height = CubeMapRes;
	//	desc.ArraySize = 6;
	//	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	//	desc.CPUAccessFlags = 0;
	//	desc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	//	desc.MipLevels = 1;
	//	desc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
	//	desc.SampleDesc.Count = 1;
	//	desc.SampleDesc.Quality = 0;
	//	desc.Usage = D3D11_USAGE_IMMUTABLE;
	//
	//	D3D11_SUBRESOURCE_DATA resData[6];
	//	for (unsigned int i = 0; i < 6; ++i)
	//	{
	//		resData[i].pSysMem = &texels[i * CubeMapRes * CubeMapRes];
	//		resData[i].SysMemPitch = sizeof(texels[0]) * CubeMapRes;
	//		resData[i].SysMemSlicePitch = 0;
	//	}
	//
	//	ID3D11Texture2D* texArray;
	//	dev->CreateTexture2D(&desc, resData, &texArray);
	//
	//	// Create a resource view to the texture array.
	//	D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;
	//	viewDesc.Format = desc.Format;
	//	viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	//	viewDesc.TextureCube.MostDetailedMip = 0;
	//	viewDesc.TextureCube.MipLevels = desc.MipLevels;
	//	dev->CreateShaderResourceView(texArray, &viewDesc, &skyModel.CubeMap);
	//
	//
	//
	//	sunPosMoved = false;
	//}
	

	//this->SetSkyDomeTexture(deviceContext, skyModel.CubeMap, 8);

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
	float theta = AngleBetween(sampleDir, XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f));

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



XMFLOAT3 Saturate(XMFLOAT3 val)
{
	XMFLOAT3 result;
	result.x = clamp(val.x, 0.0f, 1.0f);
	result.y = clamp(val.y, 0.0f, 1.0f);
	result.z = clamp(val.z, 0.0f, 1.0f);
	return result;
}

float Saturate(float val)
{
	return clamp(val, 0.0f, 1.0f);
}


void CSkyDomeShader::SkyModelData::Init(XMFLOAT3 sunDirection, XMFLOAT3 groundAlbedo, float turbidity)
{
	XMFLOAT3 sunDirection_, groundAlbedo_;
	float turb;

	sunDirection_.y = Saturate(sunDirection.y);
	XMVECTOR sunDirV = XMVector3Normalize(XMLoadFloat3(&sunDirection_));
	turb = clamp(turbidity, 1.0f, 32.0f);
	groundAlbedo_ = Saturate(groundAlbedo);

	ShutDown();

	sunDirection_.y = Saturate(sunDirection.y);
	sunDirV = XMVector3Normalize(XMLoadFloat3(&sunDirection_));
	turb = clamp(turbidity, 1.0f, 32.0f);
	groundAlbedo_ = Saturate(groundAlbedo);

    float thetaS = AngleBetween(sunDirV, XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f));
	//float thetaS = std::acos(1.0f - sunDirection.y);
    float elevation = Pi_2 - thetaS;
    StateR = arhosek_rgb_skymodelstate_alloc_init(turb, groundAlbedo_.x, elevation);
    StateG = arhosek_rgb_skymodelstate_alloc_init(turb, groundAlbedo_.y, elevation);
    StateB = arhosek_rgb_skymodelstate_alloc_init(turb, groundAlbedo_.z, elevation);

    Albedo = groundAlbedo;
    Elevation = elevation;
    SunDirection = sunDirection;
    Turbidity = turbidity;
}

void CSkyDomeShader::SkyModelData::ShutDown()
{
	if (StateR != nullptr)
	{
		arhosekskymodelstate_free(StateR);
		StateR = nullptr;
	}

	if (StateG != nullptr)
	{
		arhosekskymodelstate_free(StateG);
		StateG = nullptr;
	}

	if (StateB != nullptr)
	{
		arhosekskymodelstate_free(StateB);
		StateB = nullptr;
	}

	CubeMap = nullptr;
	Turbidity = 0.0f;
	Elevation = 0.0f;
}


