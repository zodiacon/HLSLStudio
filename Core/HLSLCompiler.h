#pragma once

#include <dxcapi.h>
#include <string>
#include <vector>
#include <atlcomcli.h>
#include "CompileResult.h"

struct HLSLCompilerOptions {
	std::wstring Name{ L"shader" };
	std::wstring MainEntryPoint { L"main" };
	std::wstring Target { L"ps_6_0" };
	std::wstring OutputFile;
	std::vector<std::wstring> Defines;
	bool EnableSlimDebugInfo{ true };
};

class HLSLCompiler {
public:
	HRESULT Init() noexcept;
	CompileResult CompileFile(PCWSTR path, HLSLCompilerOptions const& options) noexcept;
	CompileResult CompileSource(std::string const& text, HLSLCompilerOptions const& options) noexcept;
	CompileResult GetLastResult() const noexcept;
	CComPtr<ID3D12ShaderReflection> GetReflectionFromResult() const noexcept;
	CComPtr<ID3D12ShaderReflection> GetReflectionFromResult(CompileResult const& result) const noexcept;

private:
	CComPtr<IDxcCompiler3> m_Compiler;
	CComPtr<IDxcUtils> m_Utils;
	CComPtr<IDxcIncludeHandler> m_IncludeHandler;
	CompileResult m_LastResult;
};
