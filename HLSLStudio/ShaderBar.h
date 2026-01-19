#pragma once

#include "resource.h"
#include "Interfaces.h"

class CShaderBar : public CDialogImpl<CShaderBar> {
public:
	enum { IDD = IDD_SHADERBAR };

	BEGIN_MSG_MAP(CShaderBar)
		COMMAND_HANDLER(IDC_TYPE, CBN_SELCHANGE, OnTypeChanged)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	END_MSG_MAP()

	CString GetEntryPoint() const;
	CString GetProfile() const;
	ShaderType GetShaderType() const noexcept;

private:
	// Handler prototypes (uncomment arguments if needed):
	//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnTypeChanged(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	void UpdateProfiles();

	CComboBox m_ShaderTypeCombo, m_ProfileCombo;
};

