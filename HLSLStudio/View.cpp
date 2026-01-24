// View.cpp : implementation of the CView class
//
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include <ToolbarHelper.h>
#include "View.h"
#include "resource.h"
#include <fstream>
#include <wil/resource.h>
#include "ShaderDoc.h"
#include <scintilla/ILexer.h>
#include "../lexilla/Lexlib/LexerModule.h"
#include "../lexilla/Include/SciLexer.h"

const char* KeyWords_CPP[] = {
	// Primary keywords
	"alignas alignof asm audit auto bool break case catch char class "
	"const const_cast consteval constexpr continue "
	"decltype default delete do double else enum explicit export extern false final float for "
	"goto if import inline int long module mutable naked namespace new noexcept not not_eq noreturn nullptr "
	"operator or override private protected public "
	"register reinterpret_cast restrict return "
	"short signed sizeof static static_assert static_cast struct switch "
	"template this true typedef typeid typename "
	"union unsigned using virtual void volatile while xor",
	// Secondary keywords
		"float4 float3 float2 mat3x3 mat4x4",
		"",
	// special keywords
		"SV_POSITION POSITION TEXCOORD SV_TARGET",
	// Global classes and typedefs
		"char8_t char16_t char32_t int16_t int32_t "
		"int64_t int8_t intmax_t intptr_t ptrdiff_t size_t uint16_t uint32_t uint64_t uint8_t uintmax_t uintptr_t wchar_t",
		"",
		nullptr,
};

CView::CView(IMainFrame* frame) : CFrameView(frame), m_RenderView(frame) {
	m_RenderView.SetStatic(true);
}

BOOL CView::PreTranslateMessage(MSG* pMsg) {
	pMsg;
	return FALSE;
}

bool CView::LoadFile(PCWSTR path) {
	wil::unique_hfile hFile(::CreateFileW(path, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr));
	if (!hFile)
		return false;

	wil::unique_handle hMap(::CreateFileMappingW(hFile.get(), nullptr, PAGE_READONLY, 0, 0, nullptr));
	if (!hMap)
		return false;

	wil::unique_mapview_ptr<const char> p((const char*)::MapViewOfFile(hMap.get(), FILE_MAP_READ, 0, 0, 0));
	if (!p)
		return false;

	m_Editor.AddText(::GetFileSize(hFile.get(), nullptr), p.get());
	return true;
}

void CView::SetDocument(ShaderDoc* doc) noexcept {
	m_Document = doc;
	m_ShaderBar.SetDocument(doc);
}

CompileResult CView::CompileShader(ShaderItem& shader, const char* text, HLSLCompilerOptions& options) {
	options.MainEntryPoint = shader.Main;
	options.Target = shader.Profile;
	auto result = m_Compiler.CompileSource(text, options);
	auto errors = result.GetErrors();

	//m_BuildLog.StartStyling(m_BuildLog.GetTextLength(), 0);
	int lines = m_BuildLog.GetLineCount();
	m_BuildLog.AppendText(errors.length(), errors.c_str());

	static const struct {
		PCWSTR keyword;
		int style;
	} styles[] = {
		{ L"warning", WarningStyle },
		{ L"error", ErrorStyle },
	};

	for (int i = lines - 1; i < m_BuildLog.GetLineCount(); i++) {
		auto line = m_BuildLog.GetLine(i);
		for (auto& style : styles)
			if (line.Find(style.keyword) >= 0) {
				m_BuildLog.StartStyling(m_BuildLog.PositionFromLine(i), 0);
				m_BuildLog.SetStyling(line.GetLength(), style.style);
				break;
			}
	}
	return result;
}

LRESULT CView::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	m_hWndClient = m_MainSplitter.Create(m_hWnd, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN);
	m_DetailSplitter.Create(m_MainSplitter, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN);

	m_Editor.Create(m_DetailSplitter, rcDefault, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN, IDC_EDITOR, WS_EX_CLIENTEDGE);
	m_BuildLog.Create(m_DetailSplitter, rcDefault, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN, IDC_LOGGER, WS_EX_CLIENTEDGE);
	m_BuildLog.SetReadOnly(TRUE);

	m_RenderView.Create(m_MainSplitter, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN, WS_EX_CLIENTEDGE);

	m_Editor.SetScrollWidth(100);
	m_Editor.SetScrollWidthTracking(TRUE);
	m_BuildLog.SetScrollWidth(100);
	m_BuildLog.SetScrollWidthTracking(TRUE);

	m_MainSplitter.SetSplitterPanes(m_DetailSplitter, m_RenderView);
	m_DetailSplitter.SetSplitterPanes(m_Editor, m_BuildLog);

	m_MainSplitter.SetSplitterPosPct(50);
	m_DetailSplitter.SetSplitterPosPct(85);

	extern const Lexilla::LexerModule lmCPP;
	auto lexer = lmCPP.Create();
	int count = _countof(KeyWords_CPP);
	for (int i = 0; i < count; i++)
		lexer->WordListSet(i, KeyWords_CPP[i]);

	m_Editor.SetILexer(lexer);

	ToolBarButtonInfo const buttons[] = {
		{ ID_HLSL_COMPILE, IDI_COMPILE, BTNS_BUTTON, L"Compile" },
		{ ID_HLSL_RUN, IDI_RUN, BTNS_BUTTON, L"Run" },
	};
	CreateSimpleReBar(ATL_SIMPLE_REBAR_NOBORDER_STYLE);
	auto tb = ToolbarHelper::CreateAndInitToolBar(m_hWnd, buttons, _countof(buttons), 16);
	m_ShaderBar.Create(*this);
	AddSimpleReBarBand(m_ShaderBar);
	AddSimpleReBarBand(tb, L"");

	Frame()->UIAddToolBar(tb);
	CReBarCtrl(m_hWndToolBar).LockBands(true);

	m_Editor.StyleSetFont(STYLE_DEFAULT, L"Consolas");
	m_Editor.StyleSetSize(STYLE_DEFAULT, 11);
	m_Editor.StyleClearAll();

	m_Editor.StyleSetFore(SCE_C_COMMENT, RGB(0, 128, 0));
	m_Editor.StyleSetFore(SCE_C_COMMENTLINE, RGB(0, 128, 0));
	m_Editor.StyleSetFore(SCE_C_GLOBALCLASS, RGB(0, 128, 128));
	m_Editor.StyleSetFore(SCE_C_PREPROCESSOR, RGB(255, 128, 0));
	m_Editor.StyleSetFore(SCE_C_WORD, RGB(0, 0, 255));
	m_Editor.StyleSetFore(SCE_C_WORD2, RGB(0, 128, 255));
	m_Editor.SetCaretLineBack(RGB(255, 255, 0));

	m_BuildLog.StyleSetSize(STYLE_DEFAULT, 10);
	m_BuildLog.StyleClearAll();

	m_BuildLog.StyleSetFore(ErrorStyle, RGB(240, 0, 0));
	m_BuildLog.StyleSetFore(WarningStyle, RGB(128, 128, 0));

	return S_OK == m_Compiler.Init() ? 0 : -1;
}

LRESULT CView::OnDestroy(UINT, WPARAM, LPARAM, BOOL& handled) {
	handled = FALSE;
	return 0;
}

LRESULT CView::OnCompile(WORD, WORD, HWND, BOOL&) {
	std::string text;
	text.resize(m_Editor.GetTextLength());
	if (text.empty()) {
		AtlMessageBox(m_hWnd, L"No code to compile", IDR_MAINFRAME, MB_ICONWARNING);
		return 0;
	}
	m_Editor.GetText(text.length(), text.data());

	m_BuildLog.SetReadOnly(FALSE);
	m_BuildLog.ClearAll();

	HLSLCompilerOptions options;
	options.Name = m_Document->GetName();

	int active = 0;
	for (auto& shader : m_Document->GetActiveShaders()) {
		active++;
		shader.Result = CompileShader(shader, text.c_str(), options);
	}
	if (active == 0) {
		// compile active shader
		auto shader = m_Document->GetShader(m_ShaderBar.GetShaderType());
		ATLASSERT(shader);
		shader->Result = CompileShader(*shader, text.c_str(), options);
	}
	m_BuildLog.SetReadOnly(TRUE);

	return 0;
}

LRESULT CView::OnStyleNeeded(int, LPNMHDR hdr, BOOL&) {
	return 0;
}

LRESULT CView::OnBuildLogDoubleClick(int, LPNMHDR hdr, BOOL&) {
	auto n = (SCNotification*)hdr;
	auto text = m_BuildLog.GetLine(n->line);
	if (int n = text.Find(m_Document->GetName()); n >= 0) {
		PWSTR end;
		int line = wcstol((PCWSTR)text + n + 1 + m_Document->GetName().GetLength(), &end, 10);
		int col = wcstol(end + 1, nullptr, 10);
		m_BuildLog.ClearSelections();
		m_Editor.GotoPos(m_Editor.PositionFromLine(line - 1) + col - 1);
		m_Editor.SetFocus();
	}
	return 0;
}
