#include "pch.h"
#include "AboutDlg.h"
#include "View.h"
#include "MainFrm.h"

#include "ToolbarHelper.h"

BOOL CMainFrame::PreTranslateMessage(MSG* pMsg) {
	if (CFrameWindowImpl::PreTranslateMessage(pMsg))
		return TRUE;

	return m_Tabs.PreTranslateMessage(pMsg);
}

BOOL CMainFrame::OnIdle() {
	UIUpdateToolBar();

	return FALSE;
}

BOOL CMainFrame::UIAddToolBar(HWND tb) {
	return CAutoUpdateUI::UIAddToolBar(tb);
}

BOOL CMainFrame::UIEnable(UINT id, bool enable) {
	return CAutoUpdateUI::UIEnable(id, enable);
}

void CMainFrame::SetTabModified(HWND hTab, bool modified) {
	int page = hTab ? GetTabByHwnd(hTab) : m_Tabs.GetActivePage();
	if (page < 0)
		return;
	auto title = m_Tabs.GetPageTitle(page);
	CString name(title);
	if (modified)
		name += "*";
	else
		name = name.Left(name.GetLength() - 1);
	m_Tabs.SetPageTitle(page, name);
}

void CMainFrame::SetTabTitle(HWND hTab, PCWSTR title) {
	int page = hTab ? GetTabByHwnd(hTab) : m_Tabs.GetActivePage();
	if (page < 0)
		return;
	m_Tabs.SetPageTitle(page, title);
}

int CMainFrame::GetTabByHwnd(HWND h) const noexcept {
	int count = m_Tabs.GetPageCount();

	return 0;
}

LRESULT CMainFrame::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	Scintilla_RegisterClasses(nullptr);

	CreateSimpleStatusBar();

	CImageList images;
	images.Create(16, 16, ILC_COLOR32, 2, 2);
	images.AddIcon(AtlLoadIconImage(IDI_HLSL));

	m_Tabs.m_bTabCloseButton = false;
	m_hWndClient = m_Tabs.Create(m_hWnd, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, WS_EX_CLIENTEDGE);
	UISetCheck(ID_VIEW_STATUS_BAR, 1);
	m_Tabs.SetImageList(images);

	ToolBarButtonInfo const buttons[] = {
		{ ID_FILE_NEW, IDI_NEW },
		{ ID_FILE_OPEN, IDI_OPEN },
		{ 0 },
		{ ID_HLSL_COMPILE, IDI_COMPILE },
		{ 0 },
		{ ID_EDIT_COPY, IDI_COPY },
		{ ID_EDIT_CUT, IDI_CUT },
		{ ID_EDIT_PASTE, IDI_PASTE },
		{ ID_EDIT_UNDO, IDI_UNDO },
	};
	CreateSimpleReBar(ATL_SIMPLE_REBAR_NOBORDER_STYLE);
	auto tb = ToolbarHelper::CreateAndInitToolBar(m_hWnd, buttons, _countof(buttons));

	AddSimpleReBarBand(tb);
	UIAddToolBar(tb);

	InitMenu(GetMenu());

	// register object for message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop);
	pLoop->AddMessageFilter(this);
	pLoop->AddIdleHandler(this);

	const int WINDOW_MENU_POSITION = 4;

	CMenuHandle menuMain = GetMenu();
	m_Tabs.SetWindowMenu(menuMain.GetSubMenu(WINDOW_MENU_POSITION));

	UIEnable(ID_EDIT_COPY, false);
	UIEnable(ID_EDIT_PASTE, false);
	UIEnable(ID_EDIT_CUT, false);
	UIEnable(ID_EDIT_UNDO, false);
	UIEnable(ID_EDIT_REDO, false);

	return 0;
}

void CMainFrame::InitMenu(HMENU hMenu) {
	struct {
		int id;
		UINT icon;
		HICON hIcon{ nullptr };
	} const commands[] = {
		{ ID_EDIT_COPY, IDI_COPY },
		{ ID_FILE_NEW, IDI_NEW },
		{ ID_EDIT_PASTE, IDI_PASTE },
		{ ID_FILE_OPEN, IDI_OPEN },
		{ ID_FILE_SAVE, IDI_SAVE },
		{ ID_HLSL_COMPILE, IDI_COMPILE },
		{ ID_EDIT_CUT, IDI_CUT },
		{ ID_EDIT_PASTE, IDI_PASTE },
		{ ID_EDIT_UNDO, IDI_UNDO },
		{ ID_EDIT_REDO, IDI_REDO },
		{ ID_HLSL_COMPILEROPTIONS, IDI_SETTINGS },
		{ ID_HLSL_RUN, IDI_RUN },
	};
	for (auto& cmd : commands) {
		if (cmd.icon)
			AddCommand(cmd.id, cmd.icon);
		else
			AddCommand(cmd.id, cmd.hIcon);
	}
	AddMenu(hMenu);
	UIAddMenu(hMenu);
}

LRESULT CMainFrame::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {
	// unregister message filtering and idle updates
	auto pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop);
	pLoop->RemoveMessageFilter(this);
	pLoop->RemoveIdleHandler(this);

	bHandled = FALSE;
	return 1;
}

LRESULT CMainFrame::OnFileExit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	PostMessage(WM_CLOSE);
	return 0;
}

LRESULT CMainFrame::OnFileNew(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	auto doc = std::make_unique<ShaderDoc>();
	auto pView = new CView(this);
	pView->Create(m_Tabs, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 0);
	doc->SetName(L"Untitled");
	m_Tabs.AddPage(pView->m_hWnd, doc->GetName(), 0, pView);
	pView->SetDocument(doc.get());
	m_Documents.push_back(std::move(doc));

	return 0;
}

LRESULT CMainFrame::OnFileOpen(WORD, WORD, HWND, BOOL&) {
	CSimpleFileDialog dlg(TRUE, L"hlsl", nullptr, OFN_EXPLORER | OFN_ENABLESIZING,
		L"HLSL Files (*.hlsl)\0*.hlsl\0All Files\0*.*\0", m_hWnd);
	ThemeHelper::Suspend();
	auto ok = IDOK == dlg.DoModal();
	ThemeHelper::Resume();

	auto pView = new CView(this);
	pView->Create(m_Tabs, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 0);
	if (!pView->LoadFile(dlg.m_szFileName)) {
		AtlMessageBox(m_hWnd, L"Failed to read file", IDR_MAINFRAME, MB_ICONERROR);
		pView->DestroyWindow();
		return 0;
	}
	auto doc = std::make_unique<ShaderDoc>();
	doc->SetPath(dlg.m_szFileName);
	doc->SetName(dlg.m_szFileTitle);
	pView->SetDocument(doc.get());
	m_Tabs.AddPage(pView->m_hWnd, dlg.m_szFileTitle, 0, pView);
	m_Documents.push_back(std::move(doc));

	return 0;
}

LRESULT CMainFrame::OnViewStatusBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	auto bVisible = !::IsWindowVisible(m_hWndStatusBar);
	::ShowWindow(m_hWndStatusBar, bVisible ? SW_SHOWNOACTIVATE : SW_HIDE);
	UISetCheck(ID_VIEW_STATUS_BAR, bVisible);
	UpdateLayout();
	return 0;
}

LRESULT CMainFrame::OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	CAboutDlg dlg;
	dlg.DoModal();
	return 0;
}

LRESULT CMainFrame::OnWindowClose(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	int nActivePage = m_Tabs.GetActivePage();
	if (nActivePage != -1) {
		m_Tabs.RemovePage(nActivePage);
		m_Documents.erase(m_Documents.begin() + nActivePage);
	}
	else
		::MessageBeep((UINT)-1);

	return 0;
}

LRESULT CMainFrame::OnWindowCloseAll(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	m_Tabs.RemoveAllPages();

	return 0;
}

LRESULT CMainFrame::OnWindowActivate(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	int nPage = wID - ID_WINDOW_TABFIRST;
	m_Tabs.SetActivePage(nPage);

	return 0;
}
