#pragma once

#include <wil\resource.h>

#define HR(x) { HRESULT _hr = (x); if(FAILED(_hr)) return _hr; }
#ifdef _DEBUG
#define DX_ASSERT(x) ATLASSERT(SUCCEEDED(x))
#else
#define DX_ASSERT(x) __noop
#endif

#define DX_VERIFY(x) ATLVERIFY(SUCCEEDED(x))

class DXEngine {
public:
	HRESULT Init(HWND hWnd, bool useWarp = false);
	virtual ~DXEngine() noexcept;

	virtual void Update() noexcept;
	virtual void Render() noexcept;

	void SetPixelShader(IDxcBlob* blob) noexcept;
	void SetVertexShader(IDxcBlob* blob) noexcept;
	void SetClearColor(D3DCOLORVALUE color) noexcept;

	static CComPtr<IDXGIAdapter1> GetHardwareAdapter() noexcept;
	static CComPtr<IDXGIAdapter1> GetWarpAdapter() noexcept;

	void OnResize(int width, int height) noexcept;

	static constexpr int BufferCount = 2;

private:
	struct SceneConstantBuffer {
		float Time;
	private:
		char _padding[256 - sizeof(Time)];
	};
	static_assert(sizeof(SceneConstantBuffer) == 256);

	SceneConstantBuffer* m_SceneConstBufferPtr { nullptr };
	LARGE_INTEGER m_Time{ 0 };
	HRESULT CreateDevice() noexcept;
	HRESULT CreateSwapChain() noexcept;
	HRESULT CreateDefaultShaders() noexcept;
	HRESULT CreateRootSignature() noexcept;
	HRESULT CreatePipelineState() noexcept;
	HRESULT CreateVertices() noexcept;
	void WaitForPreviousFrame() noexcept;

	CComPtr<ID3D12Device> m_Device;
#ifdef _DEBUG
	CComPtr<ID3D12Debug1> m_Debug;
#endif
	CComPtr<ID3D12Resource> m_VertexBuffer;
	CComPtr<ID3D12Resource> m_RenderTarget[BufferCount];
	CComPtr<ID3D12Resource> m_ConstBuffer;
	CComPtr<ID3D12CommandAllocator> m_CommandAllocator;
	CComPtr<ID3D12CommandQueue> m_CommandQueue;
	CComPtr<ID3D12GraphicsCommandList> m_CommandList;
	CComPtr<IDXGISwapChain3> m_SwapChain;
	CComPtr<ID3D12DescriptorHeap> m_RtvHeap;
	D3D12_CPU_DESCRIPTOR_HANDLE m_RtvHandle;
	CComPtr<ID3D12DescriptorHeap> m_SrvHeap;
	CComPtr<ID3D12PipelineState> m_PipelineState;
	CComPtr<ID3D12RootSignature> m_RootSignature;
	CComPtr<IDxcBlob> m_VertexShader, m_PixelShader;
	D3D12_VERTEX_BUFFER_VIEW m_VertexBufferView;

	UINT m_RtvDescriptorSize;
	int m_Width, m_Height;
	UINT m_CurrentBufferIndex;
	D3DCOLORVALUE m_ClearColor { 1, 0, 0, 1 };
	UINT64 m_FenceValue{ 0 };
	CComPtr<ID3D12Fence> m_Fence;
	wil::unique_handle m_FenceEvent;
	HWND m_hWnd { nullptr };
	bool m_UseWarp { false };
	bool m_PipelineUpdateNeeded{ false };
};

