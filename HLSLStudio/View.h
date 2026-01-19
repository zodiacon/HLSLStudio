// View.h : interface of the CView class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <FrameView.h>
#include <CustomSplitterWindow.h>
#include <HLSLCompiler.h>
#include <CompileResult.h>
#include "Interfaces.h"
#include "ScintillaCtrl.h"
#include "ScintillaCommands.h"
#include "ShaderBar.h"

class CView :
	public CFrameView<CView, IMainFrame> {
public:
	using CFrameView::CFrameView;

	BOOL PreTranslateMessage(MSG* pMsg);
	void SetName(PCWSTR name);
	CString const& GetName() const noexcept;

protected:
	BEGIN_MSG_MAP(CView)
		COMMAND_ID_HANDLER(ID_HLSL_COMPILE, OnCompile)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		NOTIFY_HANDLER(IDC_LOGGER, SCN_STYLENEEDED, OnStyleNeeded)
		CHAIN_MSG_MAP(CFrameView)

		ALT_MSG_MAP(1)
		COMMAND_ID_HANDLER(ID_HLSL_COMPILE, OnCompile)
		NOTIFY_HANDLER(IDC_LOGGER, SCN_STYLENEEDED, OnStyleNeeded)
		CHAIN_MSG_MAP_ALT_MEMBER(m_Editor, 1)
	END_MSG_MAP()

	bool LoadFile(PCWSTR path);

private:
	static const UINT IDC_EDITOR = 0x123, IDC_LOGGER = 0x124;

	// Handler prototypes (uncomment arguments if needed):
	//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnCompile(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnStyleNeeded(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/);

private:
	CEditor m_Editor;
	CEditor m_BuildLog;
	CCustomSplitterWindow m_MainSplitter;
	CCustomHorSplitterWindow m_DetailSplitter;
	HLSLCompiler m_Compiler;
	CompileResult m_Result;
	CShaderBar m_ShaderBar;
	CString m_Name;
};
