#pragma once
#include "Interfaces.h"
#include <generator>
#include "CompileResult.h"

struct ShaderItem {
	CString Profile;
	CString Main;
	CompileResult Result;
	bool Enabled { false };
};


class ShaderDoc {
public:
	void SetName(PCWSTR name);
	void SetPath(PCWSTR path);
	void SetModified(bool modified) noexcept;
	bool Save(PCSTR text, PCWSTR path = nullptr);

	CString const& GetPath() const noexcept {
		return m_Path;
	}
	CString const& GetName() const noexcept {
		return m_Name;
	}

	ShaderItem* GetShader(ShaderType type) noexcept;
	std::generator<ShaderItem&> GetActiveShaders();

protected:
	virtual bool DoSave(PCSTR text) const;

private:
	ShaderItem m_Shaders[(int)ShaderType::_Count];
	CString m_Path, m_Name;
	bool m_Modified{ false };
};
