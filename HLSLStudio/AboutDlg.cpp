// aboutdlg.cpp : implementation of the CAboutDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"

#include <VersionResourceHelper.h>
#include "AboutDlg.h"

LRESULT CAboutDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	CenterWindow(GetParent());
	SetDialogIcon(IDR_MAINFRAME);

	GetDlgItem(IDC_LOGO).MoveWindow(12, 12, 48, 48);

	VersionResourceHelper vh;
	auto version = vh.GetValue(L"ProductVersion");
	SetDlgItemText(IDC_VERSION, vh.GetValue(L"ProductName") + L" v" + version);
	SetDlgItemText(IDC_COPYRIGHT, vh.GetValue(L"LegalCopyright"));

	return TRUE;
}

LRESULT CAboutDlg::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	EndDialog(wID);
	return 0;
}
