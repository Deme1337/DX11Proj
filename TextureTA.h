#pragma once

#ifndef TATEX_H
#define TATEX_H
#include <stdio.h>

#include "DeviceClass.h"

class CTextureTA
{
private:
	struct TargaHeader
	{
		unsigned char data1[12];
		unsigned short width;
		unsigned short height;
		unsigned char bpp;
		unsigned char data2;
	};

private:
	unsigned char* textureData;
	ID3D11Texture2D* m_texture;

	ID3D11ShaderResourceView* srvCubeMap = 0;
	ID3D11Texture2D* srcTex[6];

public:
	CTextureTA();
	CTextureTA(const CTextureTA&);
	~CTextureTA();

	//Loads tga image
	bool Initialize(ID3D11Device*, ID3D11DeviceContext*, char*);
	//Loads texture using free image library
	bool LoadFreeImage(ID3D11Device*, ID3D11DeviceContext*, const char*);
	bool LoadCubeMap(ID3D11Device* dev, ID3D11DeviceContext* devcon, std::vector<std::string> images);
	void Shutdown();

	ID3D11ShaderResourceView* GetTexture();
	ID3D11ShaderResourceView* m_textureView;
	ID3D11ShaderResourceView* cubeGetTexture() { return this->srvCubeMap; }


private:
	bool LoadTarga(char*, int&, int&);


};


#endif
