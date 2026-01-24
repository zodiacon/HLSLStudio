#include "pch.h"
#include "ShaderBar.h"
#include "ShaderDoc.h"
#include <format>

CString CShaderBar::GetEntryPoint() const {
	CString entry;
	GetDlgItemText(IDC_MAIN, entry);
	return entry;
}

CString CShaderBar::GetProfile() const {
	CString profile;
	m_ProfileCombo.GetWindowTextW(profile);
	return profile;
}

ShaderType CShaderBar::GetShaderType() const noexcept {
	return static_cast<ShaderType>(m_ShaderTypeCombo.GetItemData(m_ShaderTypeCombo.GetCurSel()));
}

void CShaderBar::SetDocument(ShaderDoc* doc) noexcept {
	m_Document = doc;
	m_Document->GetShader(GetShaderType())->Profile = GetProfile();
}

LRESULT CShaderBar::OnInitDialog(UINT, WPARAM, LPARAM, BOOL&) {
	m_ShaderTypeCombo.Attach(GetDlgItem(IDC_TYPE));
	m_ProfileCombo.Attach(GetDlgItem(IDC_PROFILE));

	const struct {
		PCWSTR text;
		ShaderType type;
	} shaders[] = {
		{ L"Vertex", ShaderType::Vertex },
		{ L"Pixel", ShaderType::Pixel },
		{ L"Geometry", ShaderType::Geometry },
		{ L"Hull", ShaderType::Hull },
		{ L"Compute", ShaderType::Compute },
		{ L"Domain", ShaderType::Domain },
	};

	for (auto& shader : shaders) {
		int n = m_ShaderTypeCombo.AddString(shader.text);
		m_ShaderTypeCombo.SetItemData(n, static_cast<DWORD_PTR>(shader.type));
	}

	m_ShaderTypeCombo.SetCurSel(0);
	UpdateProfiles();

	return 0;
}

LRESULT CShaderBar::OnTypeChanged(WORD, WORD, HWND, BOOL&) {
	UpdateProfiles();
	
	auto type = GetShaderType();
	auto item = m_Document->GetShader(type);
	CheckDlgButton(IDC_ENABLE, item->Enabled);
	SetDlgItemText(IDC_MAIN, item->Main);
	if (item->Profile.IsEmpty())
		item->Profile = GetProfile();
	else
		m_ProfileCombo.SelectString(-1, item->Profile);

	return 0;
}

LRESULT CShaderBar::OnMainChanged(WORD, WORD, HWND, BOOL&) {
	GetDlgItemText(IDC_MAIN, m_Document->GetShader(GetShaderType())->Main);
	return 0;
}

LRESULT CShaderBar::OnToggleEnable(WORD, WORD, HWND, BOOL&) {
	m_Document->GetShader(GetShaderType())->Enabled = IsDlgButtonChecked(IDC_ENABLE);
	return 0;
}

LRESULT CShaderBar::OnProfileChanged(WORD, WORD, HWND, BOOL&) {
	m_ProfileCombo.GetWindowText(m_Document->GetShader(GetShaderType())->Profile);
	return 0;
}

void CShaderBar::UpdateProfiles() {
	auto type = static_cast<ShaderType>(m_ShaderTypeCombo.GetItemData(m_ShaderTypeCombo.GetCurSel()));

	CString selected;
	m_ShaderTypeCombo.GetWindowText(selected);
	WCHAR prefix = tolower(selected[0]);

	m_ProfileCombo.ResetContent();
	for (int i = 0; i < 7; i++)
		m_ProfileCombo.AddString(std::format(L"{}s_6_{}", prefix, i).c_str());
	m_ProfileCombo.SetCurSel(0);
}
