// View.cpp : implementation of the CView class
//
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include <ToolbarHelper.h>
#include "View.h"
#include "resource.h"
#include <fstream>
#include <wil/resource.h>
#include <scintilla/ILexer.h>
#include "../lexilla/Lexlib/LexerModule.h"
#include "../lexilla/Include/SciLexer.h"

const char* KeyWords_CPP[] = {
	// Primary keywords
	"alignas alignof asm audit auto axiom bitand bitor bool break case catch char class compl concept "
	"const const_cast consteval constexpr continue co_await co_return co_yield "
	"decltype default defined delete do double dynamic_cast else enum explicit export extern false final float for "
	"friend goto if import inline int long module mutable naked namespace new noexcept not not_eq noreturn nullptr "
	"operator or or_eq override private protected public "
	"register reinterpret_cast requires restrict return "
	"short signed sizeof static static_assert static_cast struct switch "
	"template this thread_local throw true try typedef typeid typename "
	"union unsigned using virtual void volatile while xor xor_eq",
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

BOOL CView::PreTranslateMessage(MSG* pMsg) {
	pMsg;
	return FALSE;
}

void CView::SetName(PCWSTR name) {
	m_Name = name;
}

CString const& CView::GetName() const noexcept {
	return m_Name;
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

LRESULT CView::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	m_hWndClient = m_MainSplitter.Create(m_hWnd, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN);
	m_DetailSplitter.Create(m_MainSplitter, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN);
	m_Editor.Create(m_DetailSplitter, rcDefault, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN, IDC_EDITOR, WS_EX_CLIENTEDGE);
	m_BuildLog.Create(m_DetailSplitter, rcDefault, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN, IDC_LOGGER, WS_EX_CLIENTEDGE);
	m_BuildLog.SetReadOnly(TRUE);

	m_Editor.SetScrollWidth(100);
	m_Editor.SetScrollWidthTracking(TRUE);
	m_BuildLog.SetScrollWidth(100);
	m_BuildLog.SetScrollWidthTracking(TRUE);

	m_MainSplitter.SetSplitterPane(0, m_DetailSplitter);
	m_DetailSplitter.SetSplitterPanes(m_Editor, m_BuildLog);

	m_MainSplitter.SetSplitterPosPct(50);
	m_DetailSplitter.SetSplitterPosPct(85);

	extern const Lexilla::LexerModule lmCPP;
	static auto lexer = [&]() {
		auto lexer = lmCPP.Create();
		auto count = _countof(KeyWords_CPP);
		for (int i = 0; i < count; i++)
			lexer->WordListSet(i, KeyWords_CPP[i]);
		return lexer;
	}();

	m_Editor.SetILexer(lexer);

	ToolBarButtonInfo const buttons[] = {
		{ ID_HLSL_COMPILE, IDI_COMPILE, BTNS_BUTTON, L"Compile" },
		{ 0 },
		{ ID_EDIT_COPY, IDI_COPY },
		{ ID_EDIT_PASTE, IDI_PASTE },
	};
	CreateSimpleReBar(ATL_SIMPLE_REBAR_NOBORDER_STYLE);
	CToolBarCtrl tb = ToolbarHelper::CreateAndInitToolBar(m_hWnd, buttons, _countof(buttons), 16);
	m_ShaderBar.Create(*this);
	AddSimpleReBarBand(m_ShaderBar);
	AddSimpleReBarBand(tb);

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

	m_BuildLog.StyleSetSize(STYLE_DEFAULT, 10);
	m_BuildLog.StyleClearAll();

	return S_OK == m_Compiler.Init() ? 0 : -1;
}

LRESULT CView::OnCompile(WORD, WORD, HWND, BOOL&) {
	std::string text;
	text.resize(m_Editor.GetTextLength());
	if (text.empty()) {
		AtlMessageBox(m_hWnd, L"No code to compile", IDR_MAINFRAME, MB_ICONWARNING);
		return 0;
	}
	m_Editor.GetText(text.length(), text.data());

	HLSLCompilerOptions options;
	options.MainEntryPoint = m_ShaderBar.GetEntryPoint();
	options.Target = m_ShaderBar.GetProfile();
	options.Name = m_Name;

	m_Result = m_Compiler.CompileSource(text, options);
	auto errors = m_Result.GetErrors();

	m_BuildLog.SetReadOnly(FALSE);
	m_BuildLog.ClearAll();

	m_BuildLog.StyleSetFore(50, RGB(128, 0, 0));
	m_BuildLog.AppendText(errors.length(), errors.c_str());
	m_BuildLog.StartStyling(0, 0);
	m_BuildLog.SetStyling(m_BuildLog.GetTextLength(), 50);
	m_BuildLog.SetReadOnly(TRUE);

	auto reflect = m_Compiler.GetReflectionFromResult();
	if (reflect) {
		D3D12_SHADER_DESC desc;
		reflect->GetDesc(&desc);
	}

	return 0;
}

LRESULT CView::OnStyleNeeded(int, LPNMHDR hdr, BOOL&) {
	auto notify = (SCNotification*)hdr;
	auto start = m_BuildLog.GetEndStyled();
	auto line = m_BuildLog.LineFromPosition(start);
	start = m_BuildLog.PositionFromLine(line);

	std::string text;
	text.resize(notify->position - start);
	Scintilla::TextRange r{ { (long)start, (long)notify->position}, text.data() };
	m_BuildLog.GetTextRange(&r);
	if (text.find("warning") != std::string::npos) {

	}
	return 0;
}
