#include "pch.h"
#include "CompileResult.h"

CompileResult::CompileResult(IDxcResult* result) noexcept : m_Result(result) {
}

CompileResult::CompileResult(HRESULT hr) noexcept : m_HResult(hr) {
}

std::string CompileResult::GetErrors() const noexcept {
	CComPtr<IDxcBlobEncoding> buffer;
	m_Result->GetErrorBuffer(&buffer);
	if (!buffer || buffer->GetBufferSize() < 1)
		return "";

	return std::string((const char*)buffer->GetBufferPointer(), buffer->GetBufferSize() - 1);
}

IDxcResult* CompileResult::Get() const noexcept {
	return m_Result;
}

bool CompileResult::HasErrors() const noexcept {
	HRESULT hr;
	return S_OK == m_Result->GetStatus(&hr) && FAILED(hr);
}
