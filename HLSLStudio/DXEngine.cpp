#include "pch.h"
#include "DXEngine.h"

#pragma comment(lib, "d3d12")
#pragma comment(lib, "dxgi")

HRESULT DXEngine::CreateDevice() {
	auto adapter = m_UseWarp ? GetWarpAdapter() : GetHardwareAdapter();
	if (adapter == nullptr)
		return E_FAIL;

#ifdef _DEBUG
	CComPtr<ID3D12Debug> debug;
	D3D12GetDebugInterface(IID_PPV_ARGS(&debug));
	if (debug)
		debug->EnableDebugLayer();
#endif

	HR(D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_1, IID_PPV_ARGS(&m_Device)));
	HR(m_Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_CommandAllocator)));
	{
		D3D12_COMMAND_QUEUE_DESC desc{};
		desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		HR(m_Device->CreateCommandQueue(&desc, IID_PPV_ARGS(&m_CommandQueue)));
	}

	return S_OK;
}

HRESULT DXEngine::CreateSwapChain() {
	CComPtr<IDXGIFactory2> factory;
	HR(CreateDXGIFactory1(IID_PPV_ARGS(&factory)));

	RECT rc;
	::GetClientRect(m_hWnd, &rc);
	m_Width = rc.right;
	m_Height = rc.bottom;
	DXGI_SWAP_CHAIN_DESC1 desc{};
	desc.BufferCount = 2;
	desc.Width = m_Width;
	desc.Height = m_Height;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	desc.SampleDesc.Count = 1;

	CComPtr<IDXGISwapChain1> sc;
	HR(factory->CreateSwapChainForHwnd(m_CommandQueue, m_hWnd, &desc, nullptr, nullptr, &sc));
	HR(sc.QueryInterface(&m_SwapChain));
	HR(factory->MakeWindowAssociation(m_hWnd, DXGI_MWA_NO_ALT_ENTER));

	m_CurrentBufferIndex = m_SwapChain->GetCurrentBackBufferIndex();

	// Create descriptor heaps
	{
		// Describe and create a render target view (RTV) descriptor heap.
		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc {};
		rtvHeapDesc.NumDescriptors = 2;
		rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		HR(m_Device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_RtvHeap)));

		m_RtvDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	}

	// Create frame resources
	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_RtvHeap->GetCPUDescriptorHandleForHeapStart());

		// Create a RTV for each frame.
		for (UINT n = 0; n < 2; n++) {
			HR(m_SwapChain->GetBuffer(n, IID_PPV_ARGS(&m_RenderTarget[n])));
			m_Device->CreateRenderTargetView(m_RenderTarget[n], nullptr, rtvHandle);
			rtvHandle.Offset(1, m_RtvDescriptorSize);
		}
	}

	return S_OK;
}

HRESULT DXEngine::Init(HWND hWnd, bool useWarp) {
	if (m_Device)
		return E_UNEXPECTED;

	m_hWnd = hWnd;
	m_UseWarp = useWarp;
	HR(CreateDevice());
	HR(CreateSwapChain());

	return S_OK;
}

CComPtr<IDXGIAdapter1> DXEngine::GetHardwareAdapter() {
	CComPtr<IDXGIFactory2> factory;
	if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&factory))))
		return nullptr;

	for (UINT i = 0;; ++i) {
		CComPtr<IDXGIAdapter1> adapter;
		if (FAILED(factory->EnumAdapters1(i, &adapter)))
			break;

		DXGI_ADAPTER_DESC1 desc;
		adapter->GetDesc1(&desc);
		if ((desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == 0)
			return adapter;
	}
	return nullptr;
}

CComPtr<IDXGIAdapter1> DXEngine::GetWarpAdapter() {
	CComPtr<IDXGIFactory5> factory;
	auto hr = CreateDXGIFactory1(IID_PPV_ARGS(&factory));
	if (FAILED(hr))
		return nullptr;

	CComPtr<IDXGIAdapter1> adapter;
	factory->EnumWarpAdapter(IID_PPV_ARGS(&adapter));
	return adapter;
}
