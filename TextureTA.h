#pragma once

#ifndef TATEX_H
#define TATEX_H

#include <d3d11.h>
#include <stdio.h>



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

public:
	CTextureTA();
	CTextureTA(const CTextureTA&);
	~CTextureTA();

	//Loads tga image
	bool Initialize(ID3D11Device*, ID3D11DeviceContext*, char*);
	//Loads texture using free image library
	bool LoadFreeImage(ID3D11Device*, ID3D11DeviceContext*, const char*);
	void Shutdown();

	ID3D11ShaderResourceView* GetTexture();
	ID3D11ShaderResourceView* m_textureView;
private:
	bool LoadTarga(char*, int&, int&);

private:
	unsigned char* textureData;
	ID3D11Texture2D* m_texture;
};

#endif
