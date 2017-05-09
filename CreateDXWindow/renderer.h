#pragma once

#include "stdafx.h"
#include <string.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <dxgi1_3.h>
#include <DirectXMath.h>

class Renderer
{
public:
	Renderer();
	~Renderer();

	bool Init(HWND hWnd, int width, int height);

	bool RenderBegin();

	bool Render();
	bool RenderEnd();

private:
	IDXGISwapChain* m_SwapChain;
	ID3D11Device* m_Device;
	ID3D11DeviceContext* m_ImmediateContext;
	ID3D11Texture2D* m_BackBuffer;
	ID3D11RenderTargetView* m_RenderTargetView;
	ID3D11PixelShader* m_PixelShader;
	ID3D11VertexShader* m_VertexShader;
	ID3D11Buffer* m_VertexBuffer;
	ID3D11InputLayout* m_InputLayout;
	ID3D11Buffer* m_PerFrameCB;
	float m_Angle;
};

