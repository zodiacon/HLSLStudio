#include "pch.h"
#include "RenderView.h"

LRESULT CRenderView::OnCreate(UINT, WPARAM, LPARAM, BOOL&) {
	if (FAILED(m_DX.Init(m_hWnd)))
		return -1;

	return 0;
}

LRESULT CRenderView::OnSize(UINT, WPARAM wp, LPARAM lp, BOOL& handled) {
	handled = FALSE;
	int cx = GET_X_LPARAM(lp), cy = GET_Y_LPARAM(lp);
	if (wp != SIZE_MINIMIZED && cx != 0 && cy != 0)
		m_DX.OnResize(cx, cy);
	return 0;
}

LRESULT CRenderView::OnPaint(UINT, WPARAM, LPARAM, BOOL&) {
	m_DX.Update();
	m_DX.Render();

	ATLTRACE(L"WM_PAINT\n");
	CPaintDC dc(*this);

	return 0;
}
