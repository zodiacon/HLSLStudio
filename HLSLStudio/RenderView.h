#pragma once

#include <FrameView.h>
#include "Interfaces.h"
#include "DXEngine.h"

class CRenderView : public CFrameView<CRenderView, IMainFrame> {
public:
	using CFrameView::CFrameView;

	BEGIN_MSG_MAP(CRenderView)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		CHAIN_MSG_MAP(CFrameView)

		ALT_MSG_MAP(1)
	END_MSG_MAP()

private:
	// Handler prototypes (uncomment arguments if needed):
	//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

	DXEngine m_DX;
};

