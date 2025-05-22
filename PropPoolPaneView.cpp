// PorpPoolPaneView.cpp : implementation file
//

#include "stdafx.h"
#include "EtWorldPainter.h"
#include "PropPoolPaneView.h"

#include "PropPoolView.h"
#include "PropPoolPropertiesView.h"

#include "UserMessage.h"

// CPropPoolPaneView

IMPLEMENT_DYNCREATE(CPropPoolPaneView, CFormView)

CPropPoolPaneView::CPropPoolPaneView()
	: CFormView(CPropPoolPaneView::IDD)
{
	m_pPoolView = NULL;
	m_pPoolPropertiesView = NULL;
}

CPropPoolPaneView::~CPropPoolPaneView()
{
}

void CPropPoolPaneView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CPropPoolPaneView, CFormView)
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_MESSAGE( UM_SKINPANE_DBLCLICKITEM, OnDblClickItem )
END_MESSAGE_MAP()


// CPropPoolPaneView diagnostics

#ifdef _DEBUG
void CPropPoolPaneView::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CPropPoolPaneView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG


// CPropPoolPaneView message handlers

int CPropPoolPaneView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFormView::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here
	m_TabCtrl.Create(WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN|WS_CLIPSIBLINGS, CRect(0, 0, 0, 0), this, 1001);
	m_TabCtrl.GetPaintManager()->m_bDisableLunaColors = TRUE;
	m_TabCtrl.SetAppearance(xtpTabAppearanceStateButtons);
	m_TabCtrl.GetPaintManager()->m_bHotTracking = TRUE;
	m_TabCtrl.GetPaintManager()->m_bRotateImageOnVerticalDraw = TRUE;
	m_TabCtrl.SetPosition( xtpTabPositionLeft );

	CCreateContext cc;
	ZeroMemory(&cc, sizeof(cc));
	cc.m_pCurrentDoc = GetDocument();

	m_pPoolView = (CPropPoolView*)RUNTIME_CLASS(CPropPoolView)->CreateObject();	
	((CWnd *)m_pPoolView)->Create( NULL, NULL, WS_CHILD, CRect( 0, 0, 0, 0 ), &m_TabCtrl, 2005, &cc );

	m_pPoolPropertiesView = (CPropPoolPropertiesView*)RUNTIME_CLASS(CPropPoolPropertiesView)->CreateObject();	
	((CWnd *)m_pPoolPropertiesView)->Create( NULL, NULL, WS_CHILD, CRect( 0, 0, 0, 0 ), &m_TabCtrl, 2006, &cc );


	m_TabCtrl.InsertItem( 0, _T("Pool"), m_pPoolView->m_hWnd );
	m_TabCtrl.InsertItem( 1, _T("Properties"), m_pPoolPropertiesView->m_hWnd );

	return 0;
}

BOOL CPropPoolPaneView::OnEraseBkgnd(CDC* pDC)
{
	// TODO: Add your message handler code here and/or call default

	return TRUE;
//	return CFormView::OnEraseBkgnd(pDC);
}

void CPropPoolPaneView::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();

	// TODO: Add your specialized code here and/or call the base class
}

void CPropPoolPaneView::OnSize(UINT nType, int cx, int cy)
{
	CFormView::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here
	if( ::IsWindow(m_TabCtrl.m_hWnd) ) {
		m_TabCtrl.SetWindowPos( NULL, 0, 0, cx, cy, SWP_FRAMECHANGED );
	}
}

BOOL CPropPoolPaneView::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	// TODO: Add your specialized code here and/or call the base class
	if( (UINT)wParam == 1001 ) {
		NMHDR* pNMHDR = (NMHDR*)lParam;

		switch(pNMHDR->code)
		{
		case TCN_SELCHANGING:
			break;

		case TCN_SELCHANGE:
			break;
		}
	}

	return CFormView::OnNotify(wParam, lParam, pResult);
}

LRESULT CPropPoolPaneView::OnDblClickItem( WPARAM wParam, LPARAM lParam )
{
	return m_pPoolView->SendMessage( UM_SKINPANE_DBLCLICKITEM, wParam, lParam );
}
