#include "pch.h"
#include "RenderView.h"
#include "ShaderDoc.h"

int CRenderView::UpdateShaders(ShaderDoc* doc) {
	if (!doc)
		return 0;
	m_Document = doc;
	int count = 0;
	auto ps = doc->GetShader(ShaderType::Pixel);
	if (ps && ps->Result.GetByteCode()) {
		count++;
		m_DX->SetPixelShader(ps->Result.GetByteCode());
	}
	return count;
}

LRESULT CRenderView::OnCreate(UINT, WPARAM, LPARAM, BOOL&) {
	SetTimer(1, 16);
	return 0;
}

LRESULT CRenderView::OnSize(UINT, WPARAM wp, LPARAM lp, BOOL& handled) {
	handled = FALSE;
	int cx = GET_X_LPARAM(lp), cy = GET_Y_LPARAM(lp);
	if (wp != SIZE_MINIMIZED && cx != 0 && cy != 0) {
		m_DX.reset();
		m_DX = std::make_unique<DXEngine>();
		m_DX->Init(m_hWnd);
		UpdateShaders(m_Document);
	}
	return 0;
}

LRESULT CRenderView::OnPaint(UINT, WPARAM, LPARAM, BOOL&) {
	CPaintDC dc(*this);
	if (m_DX) {
		m_DX->Render();
	}

	return 0;
}

LRESULT CRenderView::OnEraseBkgnd(UINT, WPARAM, LPARAM, BOOL&) {
	return 1;
}

LRESULT CRenderView::OnTimer(UINT, WPARAM id, LPARAM, BOOL&) {
	if (id == 1 && m_DX) {
		m_DX->Update();
		m_DX->Render();
	}
	return 0;
}
