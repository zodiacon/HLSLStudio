#include "pch.h"
#include "RenderView.h"

LRESULT CRenderView::OnCreate(UINT, WPARAM, LPARAM, BOOL&) {
	if (FAILED(m_DX.Init(m_hWnd)))
		return -1;

	return 0;
}
