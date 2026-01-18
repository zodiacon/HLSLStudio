#pragma once


template<typename T>
class CScintillaCommands {
	BEGIN_MSG_MAP(CScintillaCommands<T>)
		ALT_MSG_MAP(1)
		COMMAND_ID_HANDLER(ID_EDIT_CLEAR, OnEditClear)
		COMMAND_ID_HANDLER(ID_EDIT_CLEAR_ALL, OnEditClearAll)
		COMMAND_ID_HANDLER(ID_EDIT_COPY, OnEditCopy)
		COMMAND_ID_HANDLER(ID_EDIT_CUT, OnEditCut)
		COMMAND_ID_HANDLER(ID_EDIT_PASTE, OnEditPaste)
		COMMAND_ID_HANDLER(ID_EDIT_SELECT_ALL, OnEditSelectAll)
		COMMAND_ID_HANDLER(ID_EDIT_UNDO, OnEditUndo)
	END_MSG_MAP()

	LRESULT OnEditClear(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		T* pT = static_cast<T*>(this);
		pT->Clear();
		return 0;
	}

	LRESULT OnEditClearAll(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		T* pT = static_cast<T*>(this);
		pT->SetSel(0, -1);
		pT->Clear();
		return 0;
	}

	LRESULT OnEditCopy(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		T* pT = static_cast<T*>(this);
		pT->Copy();
		return 0;
	}

	LRESULT OnEditCut(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		T* pT = static_cast<T*>(this);
		pT->Cut();
		return 0;
	}

	LRESULT OnEditPaste(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		T* pT = static_cast<T*>(this);
		pT->Paste();
		return 0;
	}

	LRESULT OnEditSelectAll(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		T* pT = static_cast<T*>(this);
		pT->SetSel(0, -1);
		return 0;
	}

	LRESULT OnEditUndo(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		T* pT = static_cast<T*>(this);
		pT->Undo();
		return 0;
	}

	LRESULT OnEditRedo(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		T* pT = static_cast<T*>(this);
		pT->Redo();
		return 0;
	}

};

class CEditor :
	public CWindowImpl<CEditor, Scintilla::CScintillaCtrl>,
	public CScintillaCommands<CEditor> {
	BEGIN_MSG_MAP(CEditor)
		ALT_MSG_MAP(1)
		CHAIN_MSG_MAP_ALT(CScintillaCommands<CEditor>, 1)
	END_MSG_MAP()

	BOOL Create(_In_ HWND hWndParent, _In_ ATL::_U_RECT rect, _In_ DWORD dwStyle, _In_ UINT nID, _In_ DWORD dwExStyle = 0, _In_opt_ LPVOID lpParam = nullptr) {
		return Scintilla::CScintillaCtrl::Create(hWndParent, rect, dwStyle, nID, dwExStyle, lpParam);
	}
};
