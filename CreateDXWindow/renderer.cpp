#include "stdafx.h"
#include "renderer.h"

#define SAFE_RELEASE(ptr)\
	if(ptr)\
	{\
		ptr->Release();\
		ptr = nullptr;\
	}

struct Vertex
{
	float position[3];
	float color[4];
};

Vertex vertices[] =
{	// X	Y	  Z		R	  G		B	  A
	{ 600.f, 100.f, .0f, 1.0f, 0.0f, 0.0f, 1.0f },
	{ 100.f, 100.f, .0f, 0.0f, 1.0f, 0.0f, 1.0f },
	{ 600.f, 600.f, .0f, 0.0f, 0.0f, 1.0f, 1.0f },
	{ 100.f, 600.f, .0f, 1.0f, 1.0f, 0.0f, 1.0f }
};

//TODO: define global D3D11_INPUT_ELEMENT_DESC array
D3D11_INPUT_ELEMENT_DESC g_Layout[] =
{
	{
		"POSITION",	//semantic name
		0, //semantic index
		DXGI_FORMAT_R32G32B32_FLOAT, //format
		0, //input slot
		D3D11_APPEND_ALIGNED_ELEMENT, //
		D3D11_INPUT_PER_VERTEX_DATA,
		0
	},
	{
		"COLOR",	//semantic name
		0, //semantic index
		DXGI_FORMAT_R32G32B32A32_FLOAT, //format
		0, //input slot
		D3D11_APPEND_ALIGNED_ELEMENT, //
		D3D11_INPUT_PER_VERTEX_DATA,
		0
	},
};

struct PerFrameCBData
{
	float screensize[4];
};

bool Failed(HRESULT aResult);

Renderer::Renderer():
	m_SwapChain(nullptr),
	m_Device(nullptr),
	m_ImmediateContext(nullptr),
	m_BackBuffer(nullptr),
	m_RenderTargetView(nullptr),
	m_PixelShader(nullptr),
	m_VertexShader(nullptr),
	m_VertexBuffer(nullptr),
	m_InputLayout(nullptr),
	m_PerFrameCB(nullptr)
{	
}


Renderer::~Renderer()
{
	SAFE_RELEASE(m_SwapChain);
	SAFE_RELEASE(m_Device);
	SAFE_RELEASE(m_ImmediateContext);
	SAFE_RELEASE(m_BackBuffer);
	SAFE_RELEASE(m_RenderTargetView);
	SAFE_RELEASE(m_PixelShader);
	SAFE_RELEASE(m_InputLayout);
	SAFE_RELEASE(m_VertexShader);
	SAFE_RELEASE(m_VertexBuffer);
	SAFE_RELEASE(m_PerFrameCB);
}

bool Renderer::Init(HWND hWnd, int width, int height) {
	DXGI_SWAP_CHAIN_DESC scd{};

	scd.BufferCount = 2; //one back buffer
	scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; //use 32-bit color
	scd.BufferDesc.Height = height;
	scd.BufferDesc.Width = width;
	scd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	scd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; //how swap chain is to be used
	scd.OutputWindow = hWnd; //the window to be used
	scd.SampleDesc.Count = 1; //how many multisamples
	scd.SampleDesc.Quality = 0;
	scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	scd.Windowed = TRUE; //windowed/full-screen mode

	if(Failed(D3D11CreateDeviceAndSwapChain(0,D3D_DRIVER_TYPE_HARDWARE,0,D3D11_CREATE_DEVICE_DEBUG,0,0,D3D11_SDK_VERSION,&scd,&m_SwapChain,&m_Device,0,&m_ImmediateContext)))
		return false;

	if(Failed(m_SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**> (&m_BackBuffer))))
		return false;
	
	if (Failed(m_Device->CreateRenderTargetView(m_BackBuffer, nullptr, &m_RenderTargetView)))
		return false;

	ID3DBlob* vs_blob = nullptr;
	//TODO: D3DReadFileToBlob + CreateInputLayout + CreateVertexShader
	if (Failed(D3DReadFileToBlob(L"vs.cso", &vs_blob)))
		return false;

	if (Failed(m_Device->CreateVertexShader(vs_blob->GetBufferPointer(), vs_blob->GetBufferSize(), nullptr, &m_VertexShader))){
		SAFE_RELEASE(vs_blob);
		return false;
	}
	
	if (Failed(m_Device->CreateInputLayout(g_Layout, sizeof(g_Layout) / sizeof(g_Layout[0]), vs_blob->GetBufferPointer(), vs_blob->GetBufferSize(), &m_InputLayout))){
		SAFE_RELEASE(vs_blob);
		return false;
	}

	SAFE_RELEASE(vs_blob);

	//TODO: D3DReadFileToBlob + CreatePixelShader
	ID3DBlob* ps_blob = nullptr;
	if (Failed(D3DReadFileToBlob(L"ps.cso", &ps_blob)))
		return false;
	
	if (Failed(m_Device->CreatePixelShader(ps_blob->GetBufferPointer(), ps_blob->GetBufferSize(), nullptr, &m_PixelShader))){
		SAFE_RELEASE(ps_blob);
		return false;
	}

	SAFE_RELEASE(ps_blob);
	//TODO: m_Device->CreateBuffer (needs D3D11_BUFFER_DESC, D3D11_SUBRESOURCE_DATA)
	D3D11_BUFFER_DESC vertexBufferDesc = {};
	D3D11_SUBRESOURCE_DATA vertexData = {};

	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(vertices);
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	vertexData.pSysMem = vertices;

	if (Failed(m_Device->CreateBuffer(&vertexBufferDesc, &vertexData, &m_VertexBuffer)))
		return false;

	//TODO: m_ImmediateContext->RSSetViewPorts (needs D3D11_VIEWPORT)
	D3D11_VIEWPORT viewport = {};

	viewport.Width = static_cast<float>(width);
	viewport.Height = static_cast<float>(height);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	m_ImmediateContext->RSSetViewports(1, &viewport);

	//TODO: perFrameCB = Constant Buffer for Screensize
	D3D11_BUFFER_DESC perFrameCBDesc = {};
	perFrameCBDesc.Usage = D3D11_USAGE_DYNAMIC;
	perFrameCBDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	perFrameCBDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	return true;
}

bool Renderer::RenderBegin() {

	const float clearColor[4] = { 1.0f, 0.0f, 1.0f, 1.0f };
	m_ImmediateContext->ClearRenderTargetView(m_RenderTargetView, clearColor);
	m_ImmediateContext->OMSetRenderTargets(1, &m_RenderTargetView, nullptr);

	return true;
}

bool Renderer::Render() {
	unsigned int stride;
	unsigned int offset;

	stride = sizeof(Vertex);
	offset = 0;

	//TODO: m_ImmediateContext->IASetPrimitiveTopology trianglestrip
	m_ImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	//TODO: m_ImmediateContext->VSSetShader
	m_ImmediateContext->VSSetShader(m_VertexShader, nullptr, 0);

	//TODO: m_ImmediateContext->PSSetShader
	m_ImmediateContext->PSSetShader(m_PixelShader, nullptr, 0);

	//TODO: m_ImmediateConntext->IASetInputLayout
	m_ImmediateContext->IASetInputLayout(m_InputLayout);

	//TODO: m_ImmediateContext->IASetVertexBuffers
	m_ImmediateContext->IASetVertexBuffers(0, 1, &m_VertexBuffer, &stride, &offset);

	//TODO: m_ImmediateContext->Draw
	m_ImmediateContext->Draw(4,  0);

	return true;
}

bool Renderer::RenderEnd() {
	
	return !Failed(m_SwapChain->Present(2, 0));
}

bool Failed(HRESULT aResult)
{
	if (FAILED(aResult))
	{
		LPTSTR buffer;
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
			nullptr, (DWORD)aResult, LANG_USER_DEFAULT, (LPTSTR)&buffer, 0, nullptr);

		MessageBox(0, buffer, L"Fatal error", MB_OK | MB_ICONERROR);
		LocalFree(buffer);

		return true;
	}
	return false;
}