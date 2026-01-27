#pragma once

#include <string>

class CompileResult {
public:
	explicit CompileResult(IDxcResult* result) noexcept;
	explicit CompileResult(HRESULT hr = E_FAIL) noexcept;

	std::string GetErrors() const noexcept;
	IDxcResult* Get() const noexcept;
	bool HasErrors() const noexcept;
	CComPtr<IDxcBlob> GetByteCode() const;

private:
	CComPtr<IDxcResult> m_Result;
	HRESULT m_HResult{ S_OK };
};


