// View.cpp : implementation of the CView class
//
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include <ToolbarHelper.h>
#include "View.h"
#include "resource.h"

BOOL CView::PreTranslateMessage(MSG* pMsg) {
	pMsg;
	return FALSE;
}

LRESULT CView::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	m_hWndClient = m_MainSplitter.Create(m_hWnd, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN);
	m_DetailSplitter.Create(m_MainSplitter, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN);
	m_Editor.Create(m_DetailSplitter, rcDefault, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN, IDC_EDITOR, WS_EX_CLIENTEDGE);

	m_MainSplitter.SetSplitterPane(0, m_DetailSplitter);
	m_DetailSplitter.SetSplitterPane(0, m_Editor);

	m_MainSplitter.SetSplitterPosPct(50);
	m_DetailSplitter.SetSplitterPosPct(50);

	ToolBarButtonInfo const buttons[] = {
		{ ID_EDIT_COPY, IDI_COPY },
		{ ID_EDIT_PASTE, IDI_PASTE },
	};
	CreateSimpleReBar(ATL_SIMPLE_REBAR_NOBORDER_STYLE);
	CToolBarCtrl tb = ToolbarHelper::CreateAndInitToolBar(m_hWnd, buttons, _countof(buttons), 16);
	m_ShaderBar.Create(*this);
	AddSimpleReBarBand(m_ShaderBar);
	AddSimpleReBarBand(tb);
	UpdateLayout();

	//CComboBox cb = ToolbarHelper::CreateToolbarComboBox(tb, IDC_SHADER_TYPE, 14, 15, CBS_DROPDOWNLIST);
	//cb.AddString(L"Vertex");
	//cb.AddString(L"Pixel");

	//UIAddToolBar(tb);
	CReBarCtrl(m_hWndToolBar).LockBands(true);

	m_Editor.StyleSetFont(STYLE_DEFAULT, L"Consolas");
	m_Editor.StyleSetSize(STYLE_DEFAULT, 11);
	m_Editor.StyleSetFore(STYLE_DEFAULT, RGB(0, 0, 128));
	m_Editor.StyleClearAll();

	return S_OK == m_Compiler.Init() ? 0 : -1;
}

LRESULT CView::OnCompile(WORD, WORD, HWND, BOOL&) {
	std::string text;
	text.resize(m_Editor.GetTextLength());
	m_Editor.GetText(text.length(), text.data());

	HLSLCompilerOptions options;
	options.MainEntryPoint = L"VSMain";
	options.Target = L"vs_6_0";
	m_Result = m_Compiler.CompileSource(text, options);
	auto errors = m_Result.GetErrors();
	auto reflect = m_Compiler.GetReflectionFromResult();
	D3D12_SHADER_DESC desc;
	reflect->GetDesc(&desc);
	return 0;
}
