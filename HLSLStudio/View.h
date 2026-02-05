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
#include "RenderView.h"

struct ShaderItem;

class CView :
	public CFrameView<CView, IMainFrame> {
public:
	explicit CView(IMainFrame* frame);

	BOOL PreTranslateMessage(MSG* pMsg);

protected:
	BEGIN_MSG_MAP(CView)
		COMMAND_ID_HANDLER(ID_HLSL_COMPILE, OnCompile)
		COMMAND_ID_HANDLER(ID_HLSL_RUN, OnRun)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		NOTIFY_CODE_HANDLER(SCN_UPDATEUI, OnUpdateUIScintilla)
		NOTIFY_HANDLER(IDC_EDITOR, SCN_SAVEPOINTREACHED, OnDocNotModified)
		NOTIFY_HANDLER(IDC_EDITOR, SCN_SAVEPOINTLEFT, OnDocModified)
		NOTIFY_HANDLER(IDC_LOGGER, SCN_STYLENEEDED, OnStyleNeeded)
		NOTIFY_HANDLER(IDC_LOGGER, SCN_DOUBLECLICK, OnBuildLogDoubleClick)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		CHAIN_MSG_MAP(CFrameView)

		ALT_MSG_MAP(1)
		COMMAND_ID_HANDLER(ID_HLSL_COMPILE, OnCompile)
		COMMAND_ID_HANDLER(ID_HLSL_RUN, OnRun)
		COMMAND_ID_HANDLER(ID_FILE_SAVE, OnFileSave)
		COMMAND_ID_HANDLER(ID_FILE_SAVE_AS, OnFileSaveAs)
		CHAIN_MSG_MAP_ALT_MEMBER(m_Editor, 1)
	END_MSG_MAP()

	bool LoadFile(PCWSTR path);
	void SetDocument(ShaderDoc* doc) noexcept;

private:
	void UpdateUIScintilla(HWND h) const;
	bool DoFileSaveAs();
	CStringA GetText();

	static const int WarningStyle = 51, ErrorStyle = 50;
	static const UINT IDC_EDITOR = 0x123, IDC_LOGGER = 0x124;
	
	CompileResult CompileShader(ShaderItem& shader, const char* text, HLSLCompilerOptions& options);

	// Handler prototypes (uncomment arguments if needed):
	//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnCompile(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnRun(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnStyleNeeded(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/);
	LRESULT OnBuildLogDoubleClick(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/);
	LRESULT OnUpdateUIScintilla(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/);
	LRESULT OnDocModified(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/);
	LRESULT OnDocNotModified(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/);
	LRESULT OnFileSave(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnFileSaveAs(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

private:
	CEditor m_Editor;
	CEditor m_BuildLog;
	CRenderView m_RenderView;
	CCustomSplitterWindow m_MainSplitter;
	CCustomHorSplitterWindow m_DetailSplitter;
	HLSLCompiler m_Compiler;
	CompileResult m_Result;
	CShaderBar m_ShaderBar;
	ShaderDoc* m_Document{ nullptr };
};
