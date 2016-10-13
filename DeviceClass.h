#pragma once
#ifndef DEVCLASS_H
#define DEVCLASS_H


#include <d3d11_2.h>
#include <d3dcompiler.h>
#include <directxmath.h>
using namespace DirectX;

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")



class CDeviceClass
{
public:
	CDeviceClass();
	~CDeviceClass();

	bool InitDeviceAndSwapChain(HWND hWnd, HINSTANCE hInst, int width, int height);
	void Begin();

	void UpdateViewPort(int w, int h);

	ID3D11Device* GetDevice() { return dev; }
	ID3D11DeviceContext* GetDevCon() { return devcon; }
	IDXGISwapChain* GetSwapChain() { return swapchain; }

	void SetBackBufferRenderTarget();

	void AlphaBlendingOn();
	void AlphaBlendingOff();

	void TurnZBufferOn();
	void TurnZBufferOff();

	void TurnCullingOn();
	void TurnCullingOff();
	void TurnCullingFront();

	void GetVideoCardInfo(char*, int&);
	void Release();
	void SetFullScreen();

	void ResolveSubRC(ID3D11Resource &srcRes, UINT srcSubRes);

	XMMATRIX GetProjectionMatrix() { return this->projectionMatrix; }
	XMMATRIX GetOrthoMatrix() { return this->orthoMatrix; }
	XMMATRIX projectionMatrix, orthoMatrix;


private:
	HWND mainWindow;

	ID3D11Resource *dstbbResource;
	HINSTANCE _hInst;
	// global declarations
	IDXGISwapChain *swapchain;             // the pointer to the swap chain interface
	ID3D11Device *dev;                     // the pointer to our Direct3D device interface
	ID3D11DeviceContext *devcon;           // the pointer to our Direct3D device context
	ID3D11RenderTargetView *backbuffer;    // global declaration
	ID3D11Texture2D *pBackBuffer;

	ID3D11RenderTargetView* m_renderTargetView;

	ID3D11Texture2D* m_depthStencilBuffer;

	ID3D11DepthStencilState* m_depthStencilState;
	ID3D11DepthStencilView* m_depthStencilView;
	ID3D11DepthStencilState* m_depthDisabledStencilState;

	ID3D11RasterizerState* m_rasterState;
	ID3D11RasterizerState* m_rasterStateNoCulling;
	ID3D11RasterizerState* m_rasterStateCullFront;


	ID3D11BlendState* m_alphaEnableBlendingState;
	ID3D11BlendState* m_alphaDisableBlendingState;

	IDXGIFactory* factory;
	IDXGIAdapter* adapter;
	IDXGIOutput* adapterOutput;
	DXGI_MODE_DESC* displayModeList;
	DXGI_ADAPTER_DESC adapterDesc;
	D3D11_VIEWPORT viewport;

	//Private functions
private:


	int sampleCount = 1;
	int sampleQuality = 0;
	int m_videoCardMemory;
	char m_videoCardDescription[128];
	bool enableAA_ = false;
	bool Initialized = false;
	
	static void OutputErrorMessageD(ID3D10Blob * errorMessage, HWND hWnd, WCHAR * shaderFilename);

public:
	bool fullscreen = false;

	std::string GetGPU();

	ID3D11Texture2D *GetBackBufferTexture() { return pBackBuffer; }
	void SetAntiAliasing(int samplecount);

	ID3D11DepthStencilView* GetDepthStencilView() { return m_depthStencilView; }
	void ResetViewPort();

	static ID3D10Blob *CompileShader(WCHAR* fileName, ShaderCompilation sc, LPCSTR entrypoint);

};

#endif