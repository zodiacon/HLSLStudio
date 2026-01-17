// View.cpp : implementation of the CView class
//
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "resource.h"

#include "View.h"

BOOL CView::PreTranslateMessage(MSG* pMsg) {
	pMsg;
	return FALSE;
}

void CView::OnFinalMessage(HWND /*hWnd*/) {
	delete this;
}

LRESULT CView::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	m_MainSplitter.Create(m_hWnd, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN);
	m_DetailSplitter.Create(m_MainSplitter, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN);
	m_SourceCtrl.Create(m_DetailSplitter, rcDefault, L"", WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOVSCROLL | ES_WANTRETURN);

	m_MainSplitter.SetSplitterPane(0, m_DetailSplitter);
	m_DetailSplitter.SetSplitterPane(0, m_SourceCtrl);

	m_MainSplitter.SetSplitterPosPct(50);
	m_DetailSplitter.SetSplitterPosPct(50);

	return 0;
}
