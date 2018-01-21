#include "stdafx.h"
#include "TextureTA.h"
#include "Ext\FreeImage\Dist\FreeImage.h"


#pragma comment(lib, "FreeImage.lib")
#include <iostream>
#include <random>

CTextureTA::CTextureTA()
{
	textureData = 0;
	m_texture = 0;
	m_textureView = 0;
}


CTextureTA::CTextureTA(const CTextureTA& other)
{
}


CTextureTA::~CTextureTA()
{
}

bool CTextureTA::LoadFreeImage(ID3D11Device* device, ID3D11DeviceContext* deviceContext, const char* filename)
{
	int height, width;
	D3D11_TEXTURE2D_DESC textureDesc;
	HRESULT hResult;
	unsigned int rowPitch;
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;

	FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;
	FIBITMAP *dib(0);


	//memcpy(&textureName, filename, sizeof(filename));

	this->textureName = filename;

	FreeImage_FlipVertical(dib);

	fif = FreeImage_GetFileType(filename, 0);

	if (fif == FIF_UNKNOWN)
		fif = FreeImage_GetFIFFromFilename(filename);

	if (fif == FIF_UNKNOWN)
		return false;

	// Check if the plugin has reading capabilities and load the file
	if (FreeImage_FIFSupportsReading(fif)) 
		dib = FreeImage_Load(fif, filename);
	if (!dib)
		return false;

	FIBITMAP* temp = dib;
	dib = FreeImage_ConvertTo32Bits(dib);
	FreeImage_Unload(temp);

	height = FreeImage_GetHeight(dib);
	width = FreeImage_GetWidth(dib);
	textureData = FreeImage_GetBits(dib);
	int bPP = FreeImage_GetBPP(dib);



	// Setup the description of the texture.
	textureDesc.Height = height;
	textureDesc.Width = width;
	textureDesc.MipLevels = 0;
	textureDesc.ArraySize = 1;
	const char *format = FreeImage_GetFormatFromFIF(fif);
	if (format == "BMP" || format == "TGA")
	{
		textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	}
	else
	{
		
		textureDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		
	}
	
	if (textureDesc.Format != 0)
	{
		textureDesc.SampleDesc.Count = 1;
		textureDesc.SampleDesc.Quality = 0;
		textureDesc.Usage = D3D11_USAGE_DEFAULT;
		textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
		textureDesc.CPUAccessFlags = 0;
		textureDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

		rowPitch = FreeImage_GetPitch(dib);
		
		D3D11_SUBRESOURCE_DATA initData;
		initData.pSysMem = textureData;
		initData.SysMemPitch = width *4;
		initData.SysMemSlicePitch = width * height*4;

		// Create the empty texture.
		hResult = device->CreateTexture2D(&textureDesc, NULL, &m_texture);
		if (FAILED(hResult))
		{
			return false;
		}
		
		
		// Copy the targa image data into the texture.
		deviceContext->UpdateSubresource(m_texture, 0, NULL, textureData, rowPitch, 0);
		

		memset(&srvDesc, 0, sizeof(srvDesc));
		// Setup the shader resource view description.
		srvDesc.Format = textureDesc.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = -1;
	
		// Create the shader resource view for the texture.
		hResult = device->CreateShaderResourceView(m_texture, &srvDesc, &m_textureView);
		if (FAILED(hResult))
		{
			return false;
		}
	

		// Generate mipmaps for this texture.
		deviceContext->GenerateMips(m_textureView);

		FreeImage_Unload(dib);
		return true;
	}
	else
	{
		FreeImage_Unload(dib);
		//Image could not be loaded
		return false;
	}


	
}

bool CTextureTA::LoadCubeMap(ID3D11Device * dev, ID3D11DeviceContext * devcon, std::vector<std::string> images)
{
	CTextureTA* tex[6];
	for (size_t i = 0; i < 6; i++)
	{
		tex[i] = new CTextureTA();
		if (!tex[i]->LoadFreeImage(dev, devcon, images[i].c_str()))
		{
			MessageBox(NULL, L"Texture path incorrect", L"ERROR", MB_OK);
			return false;
		}
		srcTex[i] = tex[i]->m_texture;

	}

	

	D3D11_TEXTURE2D_DESC texElementDesc;
	((ID3D11Texture2D*)srcTex[0])->GetDesc(&texElementDesc);
	

	


	D3D11_TEXTURE2D_DESC texArrayDesc;
	texArrayDesc.Width = texElementDesc.Width;
	texArrayDesc.Height = texElementDesc.Height;
	texArrayDesc.MipLevels = texElementDesc.MipLevels;
	texArrayDesc.ArraySize = 6;
	texArrayDesc.Format = texElementDesc.Format;
	texArrayDesc.SampleDesc.Count = 1;
	texArrayDesc.SampleDesc.Quality = 0;
	texArrayDesc.Usage = D3D11_USAGE_DEFAULT;
	texArrayDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texArrayDesc.CPUAccessFlags = 0;
	texArrayDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

	ID3D11Texture2D* texArray = 0;
	if (FAILED(dev->CreateTexture2D(&texArrayDesc, 0, &texArray)))
		return false;

	// Copy individual texture elements into texture array.
	D3D11_BOX sourceRegion;

	//Here i copy the mip map levels of the textures
	for (UINT x = 0; x < 6; x++)
	{
		for (UINT mipLevel = 0; mipLevel < texArrayDesc.MipLevels; mipLevel++)
		{
			sourceRegion.left = 0;
			sourceRegion.right = (texArrayDesc.Width >> mipLevel);
			sourceRegion.top = 0;
			sourceRegion.bottom = (texArrayDesc.Height >> mipLevel);
			sourceRegion.front = 0;
			sourceRegion.back = 1;

			//test for overflow
			if (sourceRegion.bottom == 0 || sourceRegion.right == 0)
				break;

			devcon->CopySubresourceRegion(texArray, D3D11CalcSubresource(mipLevel, x, texArrayDesc.MipLevels), 0, 0, 0, srcTex[x], mipLevel, &sourceRegion);
		}
	}

	// Create a resource view to the texture array.
	D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;
	viewDesc.Format = texArrayDesc.Format;
	viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	viewDesc.TextureCube.MostDetailedMip = 0;
	viewDesc.TextureCube.MipLevels = texArrayDesc.MipLevels;

	if (FAILED(dev->CreateShaderResourceView(texArray, &viewDesc, &srvCubeMap)))
		return false;


	for (int i = 0; i < 6; i++)
	{
		tex[i]->Shutdown();
		delete tex[i];
	}

	return true;
}

bool CTextureTA::Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, char* filename)
{
	bool result;
	int height, width;
	D3D11_TEXTURE2D_DESC textureDesc;
	HRESULT hResult;
	unsigned int rowPitch;
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;


	// Load the targa image data into memory.
	result = LoadTarga(filename, height, width);
	if (!result)
	{
		return false;
	}

	// Setup the description of the texture.
	textureDesc.Height = height;
	textureDesc.Width = width;
	textureDesc.MipLevels = 0;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	//textureDesc.Format = DXGI_FORMAT_R32G32B32A32_UINT;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

	// Create the empty texture.
	hResult = device->CreateTexture2D(&textureDesc, NULL, &m_texture);
	if (FAILED(hResult))
	{
		return false;
	}

	// Set the row pitch of the targa image data.
	rowPitch = (width * 4) * sizeof(unsigned char);
	
	// Copy the targa image data into the texture.
	deviceContext->UpdateSubresource(m_texture, 0, NULL, textureData, rowPitch, 0);

	// Setup the shader resource view description.
	srvDesc.Format = textureDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = -1;

	// Create the shader resource view for the texture.
	hResult = device->CreateShaderResourceView(m_texture, &srvDesc, &m_textureView);
	if (FAILED(hResult))
	{
		return false;
	}

	// Generate mipmaps for this texture.
	deviceContext->GenerateMips(m_textureView);

	// Release the targa image data now that the image data has been loaded into the texture.
	delete[] textureData;
	textureData = 0;

	return true;
}

bool CTextureTA::NoiseTexture(ID3D11Device * dev, ID3D11DeviceContext * devcon)
{
	D3D11_TEXTURE2D_DESC textureDesc;
	HRESULT hResult;
	unsigned int rowPitch;
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;

	std::uniform_real_distribution<float> randomFloats(0.0f, 1.0f);
	std::default_random_engine gen;

	std::vector<XMFLOAT3> ssaoNoise;
	for (unsigned int i = 0; i < 16; i++)
	{
		 
		XMFLOAT3 noise = XMFLOAT3(randomFloats(gen) * 2.0 - 1.0, randomFloats(gen) * 2.0 - 1.0, 0.0f);
		ssaoNoise.push_back(noise);
	}


	// Setup the description of the texture.
	textureDesc.Height = 4;
	textureDesc.Width = 4;
	textureDesc.MipLevels = 0;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

	// Create the empty texture.
	hResult = dev->CreateTexture2D(&textureDesc, NULL, &m_texture);
	if (FAILED(hResult))
	{
		return false;
	}

	// Set the row pitch of the targa image data.
	rowPitch = (4 * 4) * sizeof(unsigned char);

	// Copy the targa image data into the texture.
	devcon->UpdateSubresource(m_texture, 0, NULL, &ssaoNoise[0], rowPitch, 0);

	// Setup the shader resource view description.
	srvDesc.Format = textureDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = -1;

	// Create the shader resource view for the texture.
	hResult = dev->CreateShaderResourceView(m_texture, &srvDesc, &m_textureView);
	if (FAILED(hResult))
	{
		return false;
	}

	// Generate mipmaps for this texture.
	devcon->GenerateMips(m_textureView);


	return true;
}


void CTextureTA::Shutdown()
{
	// Release the texture view resource.
	if (m_textureView != NULL)
	{
		m_textureView->Release();
		m_textureView = 0;
	}

	// Release the texture.
	if (m_texture != NULL)
	{
		m_texture->Release();
		m_texture = 0;
	}

	// Release the targa data.
	if (textureData != NULL)
	{
		textureData = 0;
	}

	if (srvCubeMap != NULL)
	{
		srvCubeMap->Release();
		srvCubeMap = 0;
	}

	return;
}


ID3D11ShaderResourceView* CTextureTA::GetTexture()
{
	return m_textureView;
}


bool CTextureTA::LoadTarga(char* filename, int& height, int& width)
{
	int error, bpp, imageSize, index, i, j, k;
	FILE* filePtr;
	unsigned int count;
	TargaHeader targaFileHeader;
	unsigned char* targaImage;


	// Open the targa file for reading in binary.
	error = fopen_s(&filePtr, filename, "rb");
	if (error != 0)
	{
		return false;
	}

	// Read in the file header.
	count = (unsigned int)fread(&targaFileHeader, sizeof(TargaHeader), 1, filePtr);
	if (count != 1)
	{
		return false;
	}

	// Get the important information from the header.
	height = (int)targaFileHeader.height;
	width = (int)targaFileHeader.width;
	bpp = (int)targaFileHeader.bpp;

	// Check that it is 32 bit and not 24 bit.
	if (bpp != 32)
	{
		return false;
	}

	// Calculate the size of the 32 bit image data.
	imageSize = width * height * 4;
	
	// Allocate memory for the targa image data.
	targaImage = new unsigned char[imageSize];
	if (!targaImage)
	{
		return false;
	}

	// Read in the targa image data.
	count = (unsigned int)fread(targaImage, 1, imageSize, filePtr);
	if (count != imageSize)
	{
		return false;
	}

	// Close the file.
	error = fclose(filePtr);
	if (error != 0)
	{
		return false;
	}

	// Allocate memory for the targa destination data.
	textureData = new unsigned char[imageSize];
	if (!textureData)
	{
		return false;
	}

	// Initialize the index into the targa destination data array.
	index = 0;

	// Initialize the index into the targa image data.
	k = (width * height * 4) - (width * 4);

	// Now copy the targa image data into the targa destination array in the correct order since the targa format is stored upside down.
	for (j = 0; j<height; j++)
	{
		for (i = 0; i<width; i++)
		{
			textureData[index + 0] = targaImage[k + 2];  // Red.
			textureData[index + 1] = targaImage[k + 1];  // Green.
			textureData[index + 2] = targaImage[k + 0];  // Blue
			textureData[index + 3] = targaImage[k + 3];  // Alpha

			// Increment the indexes into the targa data.
			k += 4;
			index += 4;
		}

		// Set the targa image data index back to the preceding row at the beginning of the column since its reading it in upside down.
		k -= (width * 8);
	}

	// Release the targa image data now that it was copied into the destination array.
	delete[] targaImage;
	targaImage = 0;

	return true;
}