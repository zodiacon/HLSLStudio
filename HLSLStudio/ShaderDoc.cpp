#include "pch.h"
#include "ShaderDoc.h"

void ShaderDoc::SetName(PCWSTR name) {
	m_Name = name;
}

void ShaderDoc::SetPath(PCWSTR path) {
	m_Path = path;
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
