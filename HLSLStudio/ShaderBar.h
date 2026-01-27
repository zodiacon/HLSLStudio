#pragma once

#include "resource.h"
#include "Interfaces.h"

class ShaderDoc;

class CShaderBar : public CDialogImpl<CShaderBar> {
public:
	enum { IDD = IDD_SHADERBAR };

	BEGIN_MSG_MAP(CShaderBar)
		COMMAND_HANDLER(IDC_TYPE, CBN_SELCHANGE, OnTypeChanged)
		COMMAND_HANDLER(IDC_MAIN, EN_CHANGE, OnMainChanged)
		COMMAND_HANDLER(IDC_ENABLE, BN_CLICKED, OnToggleEnable)
		COMMAND_HANDLER(IDC_PROFILE, CBN_SELCHANGE, OnProfileChanged)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	END_MSG_MAP()

	CString GetEntryPoint() const;
	CString GetProfile() const;
	ShaderType GetShaderType() const noexcept;
	void SetDocument(ShaderDoc* doc) noexcept;

private:
	// Handler prototypes (uncomment arguments if needed):
	//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnTypeChanged(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnMainChanged(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) const;
	LRESULT OnToggleEnable(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) const;
	LRESULT OnProfileChanged(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) const;

	void UpdateProfiles();
	ShaderDoc* m_Document { nullptr };
	CComboBox m_ShaderTypeCombo, m_ProfileCombo;
};
