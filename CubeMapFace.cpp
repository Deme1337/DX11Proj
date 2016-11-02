#include "stdafx.h"
#include "CubeMapFace.h"



CubeMapFace::CubeMapFace(ID3D11Device * device, ID3D11Texture2D * texture, DXGI_FORMAT format, int size, int arrayIndex, int mipCount)
{

	for (int mipIndex = 0; mipIndex < mipCount; mipIndex++)
	{
		HRESULT result;
		D3D11_RENDER_TARGET_VIEW_DESC texRenderTargetViewDesc;

		ZeroMemory(&texRenderTargetViewDesc, sizeof(D3D11_RENDER_TARGET_VIEW_DESC));

		texRenderTargetViewDesc.Format = format;
		texRenderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
		texRenderTargetViewDesc.Texture2DArray.MipSlice = mipIndex;
		texRenderTargetViewDesc.Texture2DArray.ArraySize = 1;
		texRenderTargetViewDesc.Texture2DArray.FirstArraySlice = arrayIndex;

		result = device->CreateRenderTargetView(texture,
			&texRenderTargetViewDesc, &this->renderTargets[mipIndex]);

		if (!result)
		{
			MessageBox(NULL, L"Error cubemapface", L"Error cubemap", MB_OK);
		}
	}

}

CubeMapFace::~CubeMapFace()
{
	delete this->renderTargets;
}
