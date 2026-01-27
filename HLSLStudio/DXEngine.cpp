#include "pch.h"
#include "DXEngine.h"

#include "HLSLCompiler.h"

#pragma comment(lib, "d3d12")
#pragma comment(lib, "dxgi")

HRESULT DXEngine::CreateDevice() noexcept {
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

	HR(m_Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_Fence)));
	m_FenceEvent.reset(::CreateEvent(nullptr, FALSE, FALSE, nullptr));

	return S_OK;
}

HRESULT DXEngine::CreateSwapChain() noexcept {
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
		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc{};
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
		for (UINT n = 0; n < BufferCount; n++) {
			HR(m_SwapChain->GetBuffer(n, IID_PPV_ARGS(&m_RenderTarget[n])));
			m_Device->CreateRenderTargetView(m_RenderTarget[n], nullptr, rtvHandle);
			rtvHandle.Offset(1, m_RtvDescriptorSize);
		}
	}

	return S_OK;
}

HRESULT DXEngine::CreateDefaultShaders() noexcept {
	m_PipelineState.Release();

	auto code = R"(struct PSInput
	{
		float4 position : SV_POSITION;
		float4 color : COLOR;
	};

	PSInput VSMain(float4 position : POSITION, float4 color : COLOR) {
		PSInput result;

		result.position = position;
		result.color = color;

		return result;
	}

	float4 PSMain(PSInput input) : SV_TARGET {
		return input.color;
	})";

	HLSLCompiler compiler;
	HR(compiler.Init());
	HLSLCompilerOptions options;
	options.Target = L"vs_6_0";
	options.MainEntryPoint = L"VSMain";
	auto vs = compiler.CompileSource(code, options);
	ATLASSERT(!vs.HasErrors());
	auto vsBlob = vs.GetByteCode();
	if (!vsBlob)
		return E_FAIL;

	options.Target = L"ps_6_0";
	options.MainEntryPoint = L"PSMain";
	auto ps = compiler.CompileSource(code, options);
	ATLASSERT(!ps.HasErrors());
	auto psBlob = ps.GetByteCode();

	D3D12_INPUT_ELEMENT_DESC inputElementDescs[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
	psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
	psoDesc.pRootSignature = m_RootSignature;
	psoDesc.VS = CD3DX12_SHADER_BYTECODE(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize());
	psoDesc.PS = CD3DX12_SHADER_BYTECODE(psBlob->GetBufferPointer(), psBlob->GetBufferSize());
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState.DepthEnable = FALSE;
	psoDesc.DepthStencilState.StencilEnable = FALSE;
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.SampleDesc.Count = 1;
	HR(m_Device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_PipelineState)));

	return S_OK;
}

HRESULT DXEngine::CreateRootSignature() noexcept {
	CD3DX12_ROOT_SIGNATURE_DESC desc;
	desc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	CComPtr<ID3DBlob> blob;
	HR(D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1, &blob, nullptr));

	return m_Device->CreateRootSignature(0, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(&m_RootSignature));
}

void DXEngine::WaitForPreviousFrame() noexcept {
	// Signal and increment the fence value.
	const UINT64 fence = m_FenceValue;
	DX_VERIFY(m_CommandQueue->Signal(m_Fence, fence));
	m_FenceValue++;

	// Wait until the previous frame is finished.
	if (m_Fence->GetCompletedValue() < fence) {
		DX_VERIFY(m_Fence->SetEventOnCompletion(fence, m_FenceEvent.get()));
		WaitForSingleObject(m_FenceEvent.get(), INFINITE);
	}

	m_CurrentBufferIndex = m_SwapChain->GetCurrentBackBufferIndex();
}

HRESULT DXEngine::Init(HWND hWnd, bool useWarp) {
	if (m_Device)
		return E_UNEXPECTED;

	m_hWnd = hWnd;
	m_UseWarp = useWarp;
	HR(CreateDevice());
	HR(CreateSwapChain());
	HR(CreateRootSignature());
	HR(CreateDefaultShaders());

	HR(m_Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_CommandAllocator, m_PipelineState, IID_PPV_ARGS(&m_CommandList)));
	HR(m_CommandList->Close());

	return S_OK;
}

void DXEngine::Update() noexcept {
}

void DXEngine::Render() noexcept {
	DX_VERIFY(m_CommandAllocator->Reset());
	DX_VERIFY(m_CommandList->Reset(m_CommandAllocator, m_PipelineState));

	m_CommandList->SetGraphicsRootSignature(m_RootSignature);
	CD3DX12_VIEWPORT vp(0.f, 0.f, (float)m_Width, (float)m_Height);
	m_CommandList->RSSetViewports(1, &vp);
	CD3DX12_RECT rc(0, 0, m_Width, m_Height);
	m_CommandList->RSSetScissorRects(1, &rc);

	auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_RenderTarget[m_CurrentBufferIndex], D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	m_CommandList->ResourceBarrier(1, &barrier);

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_RtvHeap->GetCPUDescriptorHandleForHeapStart(), m_CurrentBufferIndex, m_RtvDescriptorSize);
	m_CommandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

	m_CommandList->ClearRenderTargetView(rtvHandle, (const FLOAT*)&m_ClearColor, 0, nullptr);
	barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_RenderTarget[m_CurrentBufferIndex], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	m_CommandList->ResourceBarrier(1, &barrier);

	DX_VERIFY(m_CommandList->Close());

	ID3D12CommandList* cl = m_CommandList;
	m_CommandQueue->ExecuteCommandLists(1, &cl);

	DX_VERIFY(m_SwapChain->Present(1, 0));
	WaitForPreviousFrame();
}

void DXEngine::SetClearColor(D3DCOLORVALUE color) noexcept {
	m_ClearColor = color;
}

CComPtr<IDXGIAdapter1> DXEngine::GetHardwareAdapter() noexcept {
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

CComPtr<IDXGIAdapter1> DXEngine::GetWarpAdapter() noexcept {
	CComPtr<IDXGIFactory5> factory;
	auto hr = CreateDXGIFactory1(IID_PPV_ARGS(&factory));
	if (FAILED(hr))
		return nullptr;

	CComPtr<IDXGIAdapter1> adapter;
	factory->EnumWarpAdapter(IID_PPV_ARGS(&adapter));
	return adapter;
}

void DXEngine::OnResize(int width, int height) noexcept {
	//if (m_Width == width && m_Height == height)
	//	return;

	//m_Width = width;
	//m_Height = height;

	//for (int i = 0; i < BufferCount; i++)
	//	m_RenderTarget[i].Release();

	//DX_VERIFY(m_SwapChain->ResizeBuffers(BufferCount, width, height, DXGI_FORMAT_UNKNOWN, 0));

	//CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_RtvHeap->GetCPUDescriptorHandleForHeapStart());
	//for (UINT n = 0; n < 2; n++) {
	//	m_SwapChain->GetBuffer(n, IID_PPV_ARGS(&m_RenderTarget[n]));
	//	m_Device->CreateRenderTargetView(m_RenderTarget[n], nullptr, rtvHandle);
	//	rtvHandle.Offset(1, m_RtvDescriptorSize);
	//}

}
