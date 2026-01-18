#include "pch.h"
#include "ShaderBar.h"
#include "Interfaces.h"

LRESULT CShaderBar::OnInitDialog(UINT, WPARAM, LPARAM, BOOL&) {
	m_ShaderTypeCombo.Attach(GetDlgItem(IDC_TYPE));

	const struct {
		PCWSTR text;
		ShaderType type;
	} shaders[] = {
		{ L"Vertex", ShaderType::Vertex },
		{ L"Pixel", ShaderType::Pixel },
		{ L"Geometry", ShaderType::Geometry },
		{ L"Hull", ShaderType::Hull },
	};

	for (auto& shader : shaders) {
		int n = m_ShaderTypeCombo.AddString(shader.text);
		m_ShaderTypeCombo.SetItemData(n, static_cast<DWORD_PTR>(shader.type));
	}

	return 0;
}
