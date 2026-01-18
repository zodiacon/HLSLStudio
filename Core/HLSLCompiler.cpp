#include "pch.h"
#include "HLSLCompiler.h"

HRESULT HLSLCompiler::Init() noexcept {
	auto hr = ::DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&m_Utils));
	if (FAILED(hr))
		return hr;

	hr = m_Utils->CreateDefaultIncludeHandler(&m_IncludeHandler);
	if (FAILED(hr))
		return hr;

	return ::DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&m_Compiler));
}

CompileResult HLSLCompiler::CompileFile(PCWSTR path, HLSLCompilerOptions const& options) noexcept {
	CComPtr<IDxcBlobEncoding> source;
	auto hr = m_Utils->LoadFile(path, nullptr, &source);
	if (FAILED(hr))
		return CompileResult(hr);

	return CompileSource(std::string((const char*)source->GetBufferPointer(), source->GetBufferSize()), options);
}

CompileResult HLSLCompiler::CompileSource(std::string const& text, HLSLCompilerOptions const& options) noexcept {
	DxcBuffer src;
	src.Ptr = text.c_str();
	src.Size = text.length();
	src.Encoding = DXC_CP_ACP;

	auto name = options.Name.c_str();
	std::wstring pdbname(name);
	pdbname += L".pdb";

	std::vector args{
		name,
		L"-E", options.MainEntryPoint.c_str(),
		L"-T", options.Target.c_str(),
		L"-Fd", pdbname.c_str(),     // The file name of the pdb. This must either be supplied
	};
	if (options.EnableSlimDebugInfo)
		args.push_back(L"-Zs");

	CComPtr<IDxcResult> result;
	auto hr = m_Compiler->Compile(&src, args.data(), (UINT32)args.size(), m_IncludeHandler, IID_PPV_ARGS(&result));
	if (FAILED(hr))
		return CompileResult(hr);

	m_LastResult = CompileResult(result);
	return m_LastResult;
}

CompileResult HLSLCompiler::GetLastResult() const noexcept {
	return m_LastResult;
}

CComPtr<ID3D12ShaderReflection> HLSLCompiler::GetReflectionFromResult() const noexcept {
	return GetReflectionFromResult(m_LastResult);
}

CComPtr<ID3D12ShaderReflection> HLSLCompiler::GetReflectionFromResult(CompileResult const& result) const noexcept {
	CComPtr<IDxcBlob> blob;
	result.Get()->GetOutput(DXC_OUT_REFLECTION, IID_PPV_ARGS(&blob), nullptr);
	if (!blob)
		return nullptr;

	DxcBuffer buffer;
	buffer.Ptr = blob->GetBufferPointer();
	buffer.Size = blob->GetBufferSize();
	buffer.Encoding = DXC_CP_ACP;
	CComPtr<ID3D12ShaderReflection> reflect;
	m_Utils->CreateReflection(&buffer, IID_PPV_ARGS(&reflect));
	return reflect;
}

