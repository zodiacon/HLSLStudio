#pragma once

#define HR(x) { HRESULT _hr = (x); if(FAILED(_hr)) return _hr; }

class DXEngine {
public:
	HRESULT Init(HWND hWnd, bool useWarp = false);

	static CComPtr<IDXGIAdapter1> GetHardwareAdapter();
	static CComPtr<IDXGIAdapter1> GetWarpAdapter();

private:
	HRESULT CreateDevice();
	HRESULT CreateSwapChain();

	CComPtr<ID3D12Device> m_Device;
#ifdef _DEBUG
	CComPtr<ID3D12Debug1> m_Debug;
#endif
	CComPtr<ID3D12Resource> m_RenderTarget[2];
	CComPtr<ID3D12CommandAllocator> m_CommandAllocator;
	CComPtr<ID3D12CommandQueue> m_CommandQueue;
	CComPtr<ID3D12GraphicsCommandList> m_CommandList;
	CComPtr<IDXGISwapChain3> m_SwapChain;
	CComPtr<ID3D12DescriptorHeap> m_RtvHeap;
	CComPtr<ID3D12DescriptorHeap> m_SrvHeap;
	CComPtr<ID3D12PipelineState> m_PipelineState;
	UINT m_RtvDescriptorSize;
	int m_Width, m_Height;
	UINT m_CurrentBufferIndex;
	HWND m_hWnd { nullptr};
	bool m_UseWarp { false};
};

