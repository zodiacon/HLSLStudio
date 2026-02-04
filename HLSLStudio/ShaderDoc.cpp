#include "pch.h"
#include "ShaderDoc.h"
#include <fstream>

void ShaderDoc::SetName(PCWSTR name) {
	m_Name = name;
}

void ShaderDoc::SetPath(PCWSTR path) {
	m_Path = path;
}

void ShaderDoc::SetModified(bool modified) noexcept {
	m_Modified = modified;
}

bool ShaderDoc::Save(PCSTR text, PCWSTR path) {
	if (path && *path) {
		m_Path = path;
	}

	return DoSave(text);
}

ShaderItem* ShaderDoc::GetShader(ShaderType type) noexcept {
	ATLASSERT((int)type >= 0 && (int)type < (int)ShaderType::_Count);
	return m_Shaders + (int)type;
}

std::generator<ShaderItem&> ShaderDoc::GetActiveShaders() {
	for (auto& shader : m_Shaders)
		if (shader.Enabled)
			co_yield shader;
}

bool ShaderDoc::DoSave(PCSTR text) const {
	std::ofstream stm;
	stm.open(m_Path, std::ios::binary | std::ios::out);
	if (!stm.good())
		return false;

	stm.write(text, strlen(text));
	stm.close();
	return true;
}
